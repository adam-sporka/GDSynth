#pragma once

////////////////////////////////////////////////////////////////
class CEventCoin : public CEvent
{
    COpSquareWave sqr;
public:
    CEventCoin()
    {
        sqr.setup(24, 12, .1f);
    }

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        sqr.setTargetVolume(0.f, 4.0f);

        for (int a = 0; a < BUFLEN; a += 2)
        {
            float f = sqr.getNextSample();
            output[a][0] = f;
            output[a][1] = f;
        }

        CEvent::fillFloatBuffer(output);

        if (sqr.volume < EPSILON)
        {
            m_State = EVENT_STATE::RELEASED;
        }
    }
};

