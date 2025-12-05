#pragma once

#include <assert.h>
#include <map>

using PARAM_NAME = int;
using PARAM_VALUE = float;

////////////////////////////////////////////////////////////////
class COperator
{
public:
};

////////////////////////////////////////////////////////////////
class COpSquareWave : public COperator
{
public:
    int period = 0;
    int duty_cycle = 0;
    int counter = 0;

    void setup(int period_, int duty_cycle_)
    {
        period = period_;
        duty_cycle = duty_cycle_;
    }

    inline float getNextSample()
    {
        float out;
        if (counter < duty_cycle)
        {
            out = .1f;
        }
        else
        {
            out = -.1f;
        }
        counter++;
        if (counter == period) counter = 0;
        return out;
    }
};

////////////////////////////////////////////////////////////////
class COpSineWave : public COperator
{
public:
    float phase;
    float frequency;

    inline float getNextSample()
    {
        float out;
        float delta = 2.0f * 3.1415926535f * frequency / 48000.0f;
        phase += delta;
        if (phase > 2.0f * 3.1415926535f)
        {
            phase -= 2.0f * 3.1415926535f;
        }
        return sin(phase) * .1f;
    }
};

////////////////////////////////////////////////////////////////
int g_InstanceCounter = 0;

class CEvent
{
public:
    enum EVENT_STATE
    {
        NOT_PLAYING, // Initial state
        UPCOMING, // Created but not put in a slot yet
        PLAYING, // Producing sound
        BEING_STOLEN, // Should be removed from its slot and killed
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
            printf("Stolen and released %d\n", m_InstanceCounter);
            m_State = RELEASED;
        }
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
        sqr.setup(30, 15);
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
    }
};

////////////////////////////////////////////////////////////////
class CEventEngine : public CEvent
{
    COpSineWave w1, w2, w3;

    // Interleaved LlRrLlRr ...
    virtual void fillStereoBuffer(int16_t* output, int num_frames, int num_channels)
    {
        assert(num_channels == 2);
        assert(num_frames > 0);

        w1.frequency = 251.f + rand() % 30;
        w2.frequency = 320.f + rand() % 30;
        w3.frequency = 415.f + rand() % 30;

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

            if (events[a]->m_State == CEvent::EVENT_STATE::PLAYING || events[a]->m_State == CEvent::EVENT_STATE::BEING_STOLEN)
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

        events[candidate_slot]->m_State = CEvent::EVENT_STATE::BEING_STOLEN;
        events[candidate_slot]->m_StolenBy = new_event;
    }

private:
    CEvent* events[NUM_EVENT_SLOTS];
};
