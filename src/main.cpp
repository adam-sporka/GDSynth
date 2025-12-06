#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include "windows.h"
#include "engine.h"

int dummy_buffers = 0;
HWAVEOUT hWaveOut;
WAVEHDR whdr[4];
TIntBuffer wh_data[4];
int round_robin = 0;

CEngine engine;

////////////////////////////////////////////////////////////////
DWORD dw;
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    switch (uMsg) {
    case WOM_OPEN:
        break;
    case WOM_CLOSE:
        break;
    case WOM_DONE:
        engine.fillStereoBuffer(wh_data[round_robin % 4]);
        waveOutWrite(hWaveOut, &whdr[round_robin], sizeof(whdr[round_robin]));
        round_robin = (round_robin + 1) & 0x3;
        break;
    default:
        break;
    }
}

////////////////////////////////////////////////////////////////
void openAudio()
{
    // Create the output buffers
    WAVEFORMATEX waveformatex;
    waveformatex.wFormatTag = WAVE_FORMAT_PCM;
    waveformatex.nChannels = NUM_CHANNELS;
    waveformatex.nSamplesPerSec = static_cast<int>(SAMPLE_RATE);
    waveformatex.nAvgBytesPerSec = static_cast<int>(SAMPLE_RATE * NUM_CHANNELS * 2); // 2 channels, 2 bytes per sample
    waveformatex.nBlockAlign = NUM_CHANNELS * 2;
    waveformatex.wBitsPerSample = 16;
    waveformatex.cbSize = 0;
    for (int i = 0; i < 4; i++)
    {
        memset(wh_data[i], 0, sizeof(TIntBuffer));
        whdr[i].lpData = (char*)wh_data[i];
        whdr[i].dwBufferLength = BUFLEN * NUM_CHANNELS * 2;
        whdr[i].dwBytesRecorded = 0;
        whdr[i].dwUser = 0;
        whdr[i].dwFlags = 0;
        whdr[i].dwLoops = 0;
    }

    // Open the sound card
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveformatex, (DWORD_PTR)waveOutProc, (DWORD_PTR)&dw, CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT);

    // Pre-buffer
    for (int i = 0; i < 4; i++)
    {
        memset((TSample*)wh_data[i], 0, sizeof(TSample) * BUFLEN);
        waveOutPrepareHeader(hWaveOut, &whdr[i], sizeof(whdr[i]));
        waveOutWrite(hWaveOut, &whdr[i], sizeof(whdr[i]));
    }
}

////////////////////////////////////////////////////////////////
int main(int argc, char** argv[])
{
    openAudio();
    char key;
    CEventElectrictCar* engine_event = nullptr;
    CEventDutyCycle* duty_cycle_demo = nullptr;
    float velocity = 0.f;
    do
    {
        key = _getch();

        if (key == 'd')
        {
            printf("Duty cycle: 30/60\n");
            duty_cycle_demo = new CEventDutyCycle();
            engine.createEvent(duty_cycle_demo);
        }
        if (key == 'D')
        {
            int duty_cycle = rand() % 60;
            printf("Duty cycle: %d/60\n", duty_cycle);
            duty_cycle_demo->setRTPC(CEventDutyCycle::DUTY_CYCLE, duty_cycle);
        }

        if (key == 'c')
        {
            engine.createEvent(new CEventCoinPickup());
        }

        if (key == 'a')
        {
            engine.createEvent(new CEventDrop());
        }

        if (key == 'r')
        {
            engine.createEvent(new CEventRandomSponge());
        }

        if (key == 'j')
        {
            engine.createEvent(new CEventJingle());
        }

        if (key == 't')
        {
            engine.createEvent(new CEventTweet());
        }

        if (key == 'e')
        {
            engine_event = new CEventElectrictCar();
            engine.createEvent(engine_event);
        }
        if (key == 'v')
        {
            if (engine_event)
            {
                velocity += 10.f;
                engine_event->setRTPC(CEventElectrictCar::VELOCITY, velocity);
            }
        }
        if (key == 'V')
        {
            if (engine_event)
            {
                velocity -= 10.f;
                engine_event->setRTPC(CEventElectrictCar::VELOCITY, velocity);
            }
        }

        if (key >= '0' && key <= '9')
        {
            engine.stopSlot(key - '0');
        }
    }
    while (key != 27);
    printf("Terminating.");
}
