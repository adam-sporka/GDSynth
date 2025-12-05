#pragma once

#include <assert.h>
#include <map>

#include "operators.h"

using PARAM_NAME = int;
using PARAM_VALUE = float;
int g_InstanceCounter = 0;

////////////////////////////////////////////////////////////////
class CEvent
{
public:
    enum EVENT_STATE
    {
        NOT_PLAYING, // Just created
        PLAYING, // Producing sound
        BEING_STOLEN, // Should be removed from its slot and killed
        BEING_STOPPED, // Should stop
        RELEASED, // Reported not being processed anymore
    } m_State = NOT_PLAYING;
    std::map<PARAM_NAME, PARAM_VALUE> m_Rtpc;
    int64_t m_Served = 0;
    CEvent* m_StolenBy = nullptr;
    int m_InstanceCounter = g_InstanceCounter++;

    EVENT_STATE getState() { return m_State; }

    void handleStealing()
    {
        if (m_State == BEING_STOLEN)
        {
            m_State = RELEASED;
        }
    }

    void stop()
    {
        m_State = BEING_STOPPED;
    }

    void setRTPC(PARAM_NAME name, PARAM_VALUE value)
    {
        m_Rtpc[name] = value;
    }

    PARAM_VALUE getRTPC(PARAM_NAME name, PARAM_VALUE deflt)
    {
        if (m_Rtpc.find(name) == m_Rtpc.end())
        {
            m_Rtpc[name] = deflt;
            return deflt;
        }
        return m_Rtpc[name];
    }

    // Interleaved LlRrLlRr ...
    virtual void fillStereoBuffer(int16_t* output, int num_frames, int num_channels)
    {
        m_Served += num_frames;
        handleStealing();
    }
};

////////////////////////////////////////////////////////////////
class CEventExplosion : public CEvent
{
    COpSquareWave sqr;
public:
    CEventExplosion()
    {
        sqr.setup(30, 15, .1f);
    }

    // Interleaved LlRrLlRr ...
    virtual void fillStereoBuffer(int16_t* output, int num_frames, int num_channels)
    {
        assert(num_channels == 2);
        assert(num_frames > 0);

        sqr.period += 2;
        sqr.duty_cycle += 1;
        int num_samples = num_frames * num_channels;
        auto* wrt = output;
        for (int a = 0; a < num_samples; a += 2)
        {
            float f = sqr.getNextSample();
            *wrt = f * 32767;
            wrt++;
            *wrt = f * 32767;
            wrt++;
        }

        CEvent::fillStereoBuffer(output, num_frames, num_channels);
        if (sqr.period >= 200)
        {
            m_State = EVENT_STATE::RELEASED;
        }

        if (m_State == EVENT_STATE::BEING_STOPPED)
        {
            m_State = EVENT_STATE::RELEASED;
        }
    }
};

////////////////////////////////////////////////////////////////
class CEventEngine : public CEvent
{
    COpSineWave w1, w2, w3;

public:
    CEventEngine()
    {
        w1.frequency = 251.f + rand() % 30;
        w2.frequency = 320.f + rand() % 30;
        w3.frequency = 415.f + rand() % 30;
    }

    // Interleaved LlRrLlRr ...
    virtual void fillStereoBuffer(int16_t* output, int num_frames, int num_channels)
    {
        assert(num_channels == 2);
        assert(num_frames > 0);

        int num_samples = num_frames * num_channels;
        auto* wrt = output;
        for (int a = 0; a < num_samples; a += 2)
        {
            float f = w1.getNextSample() + w2.getNextSample() + w3.getNextSample();
            *wrt = f * 32767;
            wrt++;
            *wrt = f * 32767;
            wrt++;
        }

        CEvent::fillStereoBuffer(output, num_frames, num_channels);

        if (m_State == EVENT_STATE::BEING_STOPPED)
        {
            w1.frequency *= 0.99f;
            w2.frequency *= 0.99f;
            w3.frequency *= 0.99f;

            if (w1.frequency < 20.0f)
            {
                m_State = EVENT_STATE::RELEASED;
            }
        }
    }
};

////////////////////////////////////////////////////////////////
#define NUM_EVENT_SLOTS 8
class CEngine
{
private:
    int16_t intermediate_buffer[1024 * 1024];

public:
    // Interleaved LlRrLlRr ...
    void fillStereoBuffer(int16_t* output, int num_frames, int num_channels)
    {
        assert(num_channels == 2);
        assert(num_frames > 0);

        int num_samples = num_frames * num_channels;
        memset(output, 0, num_samples * sizeof(int16_t));

        for (int a = 0; a < NUM_EVENT_SLOTS; a++)
        {
            if (!events[a]) continue;

            if (events[a]->m_State == CEvent::EVENT_STATE::PLAYING 
                || events[a]->m_State == CEvent::EVENT_STATE::BEING_STOLEN
                || events[a]->m_State == CEvent::EVENT_STATE::BEING_STOPPED)
            {
                events[a]->fillStereoBuffer(intermediate_buffer, num_frames, num_channels);
                for (int i = 0; i < num_samples; i++)
                {
                    output[i] += intermediate_buffer[i];
                }
            }
        }

        deleteReleasedEvents();
    }

    CEngine()
    {
        memset(events, 0, sizeof(CEvent*) * NUM_EVENT_SLOTS);
    }

    void deleteReleasedEvents()
    {
        // It is safe to delete all RELEASED events;
        for (int a = 0; a < NUM_EVENT_SLOTS; a++)
        {
            // This slot doesn't have anything in yet
            if (!events[a]) continue;

            // The audio thread has released this event --> can be deleted and forgotten
            if (events[a]->getState() == CEvent::EVENT_STATE::RELEASED)
            {
                auto stolen_by = events[a]->m_StolenBy;
                delete events[a];
                events[a] = nullptr;
                if (stolen_by)
                {
                    events[a] = stolen_by;
                    events[a]->m_State = CEvent::EVENT_STATE::PLAYING;
                }
            }
        }
    }

    CEvent* createEvent(CEvent* new_event)
    {
        // Find a slot for it
        for (int a = 0; a < NUM_EVENT_SLOTS; a++)
        {
            if (!events[a])
            {
                events[a] = new_event;
                new_event->m_State = CEvent::EVENT_STATE::PLAYING;
                return new_event;
            }
        }

        // None found? Steal the oldest.
        int64_t max_served = 0;
        int candidate_slot = 0;
        for (int a = 0; a < NUM_EVENT_SLOTS; a++)
        {
            if (events[a]->m_Served >= max_served)
            {
                max_served = events[a]->m_Served;
                candidate_slot = a;
            }
        }

        // Mark the event being stolen and remember the new event that is replacing it
        events[candidate_slot]->m_State = CEvent::EVENT_STATE::BEING_STOLEN;
        events[candidate_slot]->m_StolenBy = new_event;

        return new_event;
    }

private:
    CEvent* events[NUM_EVENT_SLOTS];
};
