#pragma once

////////////////////////////////////////////////////////////////
class CEventCoinPickup : public CEvent
{
    COpSquareWave sqr;
    int length_counter = 0;

public:
    CEventCoinPickup()
    {
        sqr.setup(24, 12, .1f);
    }

    virtual const char* getName() { return "COIN_PICKUP"; };

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        for (int a = 0; a < BUFLEN; a += 2)
        {
            float f = sqr.getNextSample();
            output[a][0] = f;
            output[a][1] = f;
        }

        length_counter++;
        if (length_counter > 3)
        {
            m_State = EVENT_STATE::RELEASED;
        }

        CEvent::fillFloatBuffer(output);
    }
};

