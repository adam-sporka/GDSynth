#pragma once

class COperator
{
public:
    float volume = 1.0f;
    float target_volume = 1.0f;
    float volume_delta = 1.0f / (float)SAMPLE_RATE;

public:
    inline void updateVolume()
    {
        if (fabs(target_volume - volume) < EPSILON)
        {
            return;
        }
        else if (target_volume > volume)
        {
            volume += volume_delta;
            if (volume > target_volume) volume = target_volume;
        }
        else
        {
            volume -= volume_delta;
            if (volume < target_volume) volume = target_volume;
        }
    }

    virtual inline float getNextSample()
    {
        return 0.0f;
    }

    // Set immediate volume level
    void setVolume(float target_volume_)
    {
        target_volume = target_volume_;
        volume = target_volume_;
    }

    // Set target volume level
    // rate = float number of seconds it would take to go from 0 to 1
    void setTargetVolume(float target_volume_, float rate_)
    {
        target_volume = target_volume_;
        volume_delta = rate_ / (float)SAMPLE_RATE;
    }
};

