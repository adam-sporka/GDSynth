#pragma once

////////////////////////////////////////////////////////////////
class COperator
{
public:
};

////////////////////////////////////////////////////////////////
class COpSquareWave : public COperator
{
public:
    int period = 0;
    int duty_cycle = 0;
    int counter = 0;
    float volume = 0.1f;

    void setup(int period_, int duty_cycle_, float volume_)
    {
        period = period_;
        duty_cycle = duty_cycle_;
        volume = volume_;
    }

    inline float getNextSample()
    {
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
    float volume = 0.1f;

    inline float getNextSample()
    {
        float out;
        float delta = 2.0f * 3.1415926535f * frequency / 48000.0f;
        phase += delta;
        if (phase > 2.0f * 3.1415926535f)
        {
            phase -= 2.0f * 3.1415926535f;
        }
        return sin(phase) * volume;
    }
};

