#pragma once

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

