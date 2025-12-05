#pragma once

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
    };

public:
    EVENT_STATE m_State;
    std::map<TParamName, TParamValue> m_Rtpc;
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

    void setRTPC(TParamName name, TParamValue value)
    {
        m_Rtpc[name] = value;
    }

    TParamValue getRTPC(TParamName name, TParamValue deflt)
    {
        if (m_Rtpc.find(name) == m_Rtpc.end())
        {
            m_Rtpc[name] = deflt;
            return deflt;
        }
        return m_Rtpc[name];
    }

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        m_Served += BUFLEN;
        handleStealing();
    }
};

