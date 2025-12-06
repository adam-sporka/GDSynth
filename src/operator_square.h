#pragma once

class COpSquareWave : public COperator
{
public:
    int period = 0;
    int duty_cycle = 0;
    int counter = 0;

public:
    void setup(int period_, int duty_cycle_, float volume_)
    {
        period = period_;
        duty_cycle = duty_cycle_;
        setVolume(volume_);
    }

    void setMidiNote(int note)
    {
        float f = 440 * pow(2, (note - 69) / 12.0f);
        period = SAMPLE_RATE / f;
        duty_cycle = period / 2;
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
        if (counter >= period) counter = 0;
        return out;
    }
};

