#pragma once

class COpSineWave : public COperator
{
public:
    float phase;
    float frequency;
    float frequency_bend = 0.0f;

public:
    inline float getNextSample()
    {
        updateVolume();
        float delta = 2 * PI * (frequency + (frequency_bend * frequency)) / 48000.0f;
        phase += delta;
        if (phase > 2 * PI)
        {
            phase -= 2 * PI;
        }
        return sinf(phase) * volume;
    }
};

