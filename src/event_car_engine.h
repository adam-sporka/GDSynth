#pragma once

////////////////////////////////////////////////////////////////
class CEventCarEngine : public CEvent
{
    COpSineWave w1, w2, w3;
    float pan;

public:
    CEventCarEngine()
    {
        w1.frequency = 251.f + rand() % 30;
        w2.frequency = 320.f + rand() % 30;
        w3.frequency = 415.f + rand() % 30;
        pan = rand() % 1024 / 1024.0f;
    }

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        auto* wrt = output;
        for (int a = 0; a < BUFLEN; a += 2)
        {
            float f = w1.getNextSample() + w2.getNextSample() + w3.getNextSample();
            output[a][0] = f * pan;
            output[a][1] = f * (1.0f - pan);
        }

        CEvent::fillFloatBuffer(output);

        if (m_State == EVENT_STATE::BEING_STOPPED)
        {
            w1.setTargetVolume(0.0f, 0.5f);
            w1.frequency *= 0.99f;
            w2.setTargetVolume(0.0f, 0.5f);
            w2.frequency *= 0.99f;
            w3.setTargetVolume(0.0f, 0.5f);
            w3.frequency *= 0.99f;

            if (w1.frequency < 20.0f)
            {
                m_State = EVENT_STATE::RELEASED;
            }
        }
    }
};

