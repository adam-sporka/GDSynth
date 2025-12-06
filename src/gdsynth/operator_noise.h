#pragma once

class COpNoise : public COperator
{
public:
    inline float getNextSample()
    {
        updateVolume();
        return rand() % 1024 / 1024.f * volume;
    }
};

