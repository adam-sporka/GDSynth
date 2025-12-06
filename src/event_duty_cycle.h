#pragma once

////////////////////////////////////////////////////////////////
class CEventDutyCycle : public CEvent
{
    COpSquareWave sqr;

public:
    static constexpr TParamName DUTY_CYCLE = 0;
    
public:
    CEventDutyCycle()
    {
        sqr.setup(60, 30, .1f);
    }

    virtual const char* getName() { return "DUTY_CYCLE_TEST"; };

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        sqr.duty_cycle = getRTPC(DUTY_CYCLE, 30);

        for (int a = 0; a < BUFLEN; a += 2)
        {
            float f = sqr.getNextSample();
            output[a][0] = f;
            output[a][1] = f;
        }
        
        if (m_State == EVENT_STATE::STOP_REQUESTED)
        {
            m_State = EVENT_STATE::RELEASED;
        }

        CEvent::fillFloatBuffer(output);
    }
};

  