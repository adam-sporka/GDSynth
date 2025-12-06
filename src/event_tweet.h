#pragma once

////////////////////////////////////////////////////////////////
class CEventTweet : public CEvent
{
    COpSineWave w1, w2;
    float pan;

public:
    CEventTweet()
    {
        pan = (rand() % 1024) / 1024.f;

        w1.setVolume(0.1f);
        w1.frequency = 3500.f + rand() % 500;
        w1.setTargetVolume(0, .25f);

        w2.setVolume(1.f);
        w2.phase = 2 * PI * ((rand() % 1000) / 1000.f);
        w2.frequency = 6.f + rand() % 6;
    }

    virtual const char* getName() { return "TWEET"; };

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        for (int a = 0; a < BUFLEN; a += 2)
        {
            float mod = w2.getNextSample();
            w1.frequency_bend = mod * 0.25f;
            float value = w1.getNextSample();
            output[a][0] = value * pan;
            output[a][1] = value * (1.0f - pan);
        }

        if (w1.volume < EPSILON)
        {
            m_State = EVENT_STATE::RELEASED;
        }
    }
};