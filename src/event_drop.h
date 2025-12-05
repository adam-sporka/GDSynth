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
    virtual void fillStereoBuffer(int16_t* output, int num_frames, int num_channels)
    {
        assert(num_channels == 2);
        assert(num_frames > 0);

        sqr.period += 2;
        sqr.duty_cycle += 1;
        int num_samples = num_frames * num_channels;
        auto* wrt = output;
        for (int a = 0; a < num_samples; a += 2)
        {
            float f = sqr.getNextSample();
            *wrt = f * 32767;
            wrt++;
            *wrt = f * 32767;
            wrt++;
        }

        CEvent::fillStereoBuffer(output, num_frames, num_channels);
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

