#pragma once

#include <assert.h>
#include <map>

#include "types.h"
#include "envelope.h"
#include "operators.h"
#include "event.h"
#include "event_drop.h"
#include "event_car_engine.h"

////////////////////////////////////////////////////////////////
class CEngine
{
private:
    TFloatBuffer intermediate;
    TFloatBuffer accumulator;

public:
    // Interleaved LlRrLlRr ...
    void fillStereoBuffer(TIntBuffer output)
    {
        memset(accumulator, 0, sizeof(TFloatBuffer));
        for (int a = 0; a < NUM_EVENT_SLOTS; a++)
        {
            if (!events[a]) continue;

            if (events[a]->m_State == CEvent::EVENT_STATE::PLAYING 
             || events[a]->m_State == CEvent::EVENT_STATE::BEING_STOLEN
             || events[a]->m_State == CEvent::EVENT_STATE::BEING_STOPPED)
            {
                events[a]->fillFloatBuffer(intermediate);
                for (int i = 0; i < BUFLEN; i++)
                {
                    accumulator[i][0] += intermediate[i][0];
                    accumulator[i][1] += intermediate[i][1];
                }
            }
        }

        for (int a = 0; a < BUFLEN; a++)
        {
            output[a][0] = accumulator[a][0] * 32767;
            output[a][1] = accumulator[a][1] * 32767;
        }

        deleteReleasedEvents();
    }

    CEngine()
    {
        memset(events, 0, sizeof(CEvent*) * NUM_EVENT_SLOTS);
    }

    void stopSlot(int slotIndex)
    {
        if (slotIndex >= NUM_EVENT_SLOTS)
        {
            return;
        }

        if (events[slotIndex] == nullptr)
        {
            return;
        }

        events[slotIndex]->stop();
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
