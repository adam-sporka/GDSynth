#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include "windows.h"
#include "engine.h"

#define SAMPLE_RATE 48000
#define NUM_CHANNELS 2
#define BUFLEN 512

int dummy_buffers = 0;
HWAVEOUT hWaveOut;
WAVEHDR whdr[8];
unsigned char* wh_data[8];
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
        engine.fillStereoBuffer((int16_t*)wh_data[round_robin % 4], BUFLEN, NUM_CHANNELS);
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
        wh_data[i] = new unsigned char[BUFLEN * NUM_CHANNELS * 2];
        memset(wh_data[i], 0, BUFLEN * NUM_CHANNELS * 2);
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
        memset((int16_t*)wh_data[i], 0, sizeof(int16_t) * BUFLEN);
        waveOutPrepareHeader(hWaveOut, &whdr[i], sizeof(whdr[i]));
        waveOutWrite(hWaveOut, &whdr[i], sizeof(whdr[i]));
    }
}

////////////////////////////////////////////////////////////////
int main(int argc, char** argv[])
{
    openAudio();
    char key;
    CEvent* engine_event = nullptr;
    do
    {
        key = _getch();
        if (key == 'a')
        {
            engine.createEvent(new CEventExplosion());
        }
        if (key == 'e')
        {
            engine_event = engine.createEvent(new CEventEngine());
        }
        if (key == 'k')
        {
            if (engine_event)
            {
                engine_event->stop();
            }
        }
    }
    while (key != 27);
    printf("Terminating.");
}
