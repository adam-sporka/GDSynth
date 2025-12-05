#pragma once

////////////////////////////////////////////////////////////////
class CEventCarEngine : public CEvent
{
    COpSineWave w1, w2, w3;

public:
    CEventCarEngine()
    {
        w1.frequency = 251.f + rand() % 30;
        w2.frequency = 320.f + rand() % 30;
        w3.frequency = 415.f + rand() % 30;
    }

    // Interleaved LlRrLlRr ...
    virtual void fillStereoBuffer(int16_t* output, int num_frames, int num_channels)
    {
        assert(num_channels == 2);
        assert(num_frames > 0);

        int num_samples = num_frames * num_channels;
        auto* wrt = output;
        for (int a = 0; a < num_samples; a += 2)
        {
            float f = w1.getNextSample() + w2.getNextSample() + w3.getNextSample();
            *wrt = f * 32767;
            wrt++;
            *wrt = f * 32767;
            wrt++;
        }

        CEvent::fillStereoBuffer(output, num_frames, num_channels);

        if (m_State == EVENT_STATE::BEING_STOPPED)
        {
            w1.frequency *= 0.99f;
            w2.frequency *= 0.99f;
            w3.frequency *= 0.99f;

            if (w1.frequency < 20.0f)
            {
                m_State = EVENT_STATE::RELEASED;
            }
        }
    }
};

