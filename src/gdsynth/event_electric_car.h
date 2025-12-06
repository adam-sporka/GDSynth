#pragma once

////////////////////////////////////////////////////////////////
class CEventElectrictCar : public CEvent
{
    COpSineWave w1, w2, w3;
    float pan;

public:
    static constexpr TParamName VELOCITY = 0;

public:
    CEventElectrictCar()
    {
        w1.setVolume(0.1f);
        w1.frequency = 251.f + rand() % 30;

        w2.setVolume(0.1f);
        w2.frequency = 320.f + rand() % 30;

        w3.setVolume(0.1f);
        w3.frequency = 415.f + rand() % 30;

        pan = rand() % 1024 / 1024.0f;
    }

    virtual const char* getName() { return "ELECTRIC_CAR"; };

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        auto v = getRTPC(VELOCITY, 0);
        if (v < 0.f) v = 0.f;
        if (v > 100.f) v = 100.f;

        w1.frequency_bend = v / 100.f;
        w2.frequency_bend = v / 100.f;
        w3.frequency_bend = v / 100.f;

        auto* wrt = output;
        for (int a = 0; a < BUFLEN; a += 2)
        {
            float f = w1.getNextSample() + w2.getNextSample() + w3.getNextSample();
            output[a][0] = f * pan;
            output[a][1] = f * (1.0f - pan);
        }

        CEvent::fillFloatBuffer(output);

        if (m_State == EVENT_STATE::STOP_REQUESTED)
        {
            w1.setTargetVolume(0.0f, 0.5f);
            w2.setTargetVolume(0.0f, 0.5f);
            w3.setTargetVolume(0.0f, 0.5f);
            m_State = EVENT_STATE::BEING_STOPPED;
        }

        if (m_State == EVENT_STATE::BEING_STOPPED)
        {
            w1.frequency *= 0.995f;
            w2.frequency *= 0.995f;
            w3.frequency *= 0.995f;
            if (w1.volume < EPSILON)
            {
                m_State = EVENT_STATE::RELEASED;
            }
        }
    }
};

