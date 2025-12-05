#pragma once

////////////////////////////////////////////////////////////////
class CEventJingle : public CEvent
{
    COpSquareWave sqr;
    
    int melody[8] = { 72, 69, 67, 65, 67, 60, 62, -1 };
    int melody_pos = 0;
    int tempo = 16;
    int tick_counter = 0;
    
public:
    CEventJingle()
    {
        sqr.setup(24, 12, .1f);
    }

    // Interleaved LlRrLlRr ...
    virtual void fillFloatBuffer(TFloatBuffer output)
    {
        // Advance the melody
        if (tick_counter == 0)
        {
            printf(".");
            if (melody[melody_pos] == -1)
            {
                printf("!");
                m_State = EVENT_STATE::RELEASED;
            }
            sqr.setMidiNote(melody[melody_pos]);
            melody_pos++;
        }

        tick_counter++;
        if (tick_counter == tempo)
        {
            tick_counter = 0;
        }
    
        for (int a = 0; a < BUFLEN; a += 2)
        {
            float f = sqr.getNextSample();
            output[a][0] = f;
            output[a][1] = f;
        }

        CEvent::fillFloatBuffer(output);
    }
};

  