#pragma once

////////////////////////////////////////////////////////////////
class CEventDrop : public CEvent
{
    COpSquareWave sqr;
public:
    CEventDrop()
    {
        sqr.setup(30, 15, .1f);
    }

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        sqr.period += 2;
        sqr.duty_cycle += 1;
        auto* wrt = output;
        for (int a = 0; a < BUFLEN; a += 2)
        {
            float f = sqr.getNextSample();
            output[a][0] = f;
            output[a][1] = f;
        }

        CEvent::fillFloatBuffer(output);
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

