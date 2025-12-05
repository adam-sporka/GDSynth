#pragma once

////////////////////////////////////////////////////////////////
class COperator
{
public:
    float volume = 1.0f;
    float target_volume = 1.0f;
    float rate = 1.0f / (float)SAMPLE_RATE;

public:
    inline void updateVolume()
    {
        if (fabs(target_volume - volume) < EPSILON)
        {
            return;
        }
        else if (target_volume > volume)
        {
            volume += rate;
            if (volume > target_volume) volume = target_volume;
        }
        else
        {
            volume -= rate;
            if (volume < target_volume) volume = target_volume;
        }
    }

    void setVolume(float target_volume_)
    {
        target_volume = target_volume_;
        volume = target_volume_;
    }

    void setTargetVolume(float target_volume_)
    {
        target_volume = target_volume_;
    }
};

////////////////////////////////////////////////////////////////
class COpSquareWave : public COperator
{
public:
    int period = 0;
    int duty_cycle = 0;
    int counter = 0;

    void setup(int period_, int duty_cycle_, float volume_)
    {
        period = period_;
        duty_cycle = duty_cycle_;
        setTargetVolume(volume_);
    }

    inline float getNextSample()
    {
        updateVolume();
        float out;
        if (counter < duty_cycle)
        {
            out = volume;
        }
        else
        {
            out = -volume;
        }
        counter++;
        if (counter == period) counter = 0;
        return out;
    }
};

////////////////////////////////////////////////////////////////
class COpSineWave : public COperator
{
public:
    float phase;
    float frequency;

    inline float getNextSample()
    {
        updateVolume();
        float out;
        float delta = 2 * PI * frequency / 48000.0f;
        phase += delta;
        if (phase > 2 * PI)
        {
            phase -= 2 * PI;
        }
        return sin(phase) * volume * 0.1f;
    }
};

