#pragma once

////////////////////////////////////////////////////////////////
class CEventRandomSponge: public CEvent
{
    COpSquareWave sqr;
    
public:
    CEventRandomSponge()
    {
        sqr.setup(24, 12, .1f);
    }

    virtual const char* getName() { return "RANDOM_SPONGE"; };

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        sqr.period = rand() % 32;
        sqr.duty_cycle = sqr.period >> 1;
        
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

  