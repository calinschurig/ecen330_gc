#include "tone.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define TONE_BIAS_LEVEL 0x80
#define TONE_MAX_LEVEL 0xFF

typedef uint8_t tone_buffer_sample_t;
tone_buffer_sample_t* tone_buffer = NULL;
uint32_t tone_buffer_length = 0;

// Tone waveforms

// Initialize the tone driver. Must be called before using.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback tone.
// Return zero if successful, or non-zero otherwise.
int32_t tone_init(uint32_t sample_hz) 
{
    if (sample_hz < 2*LOWEST_FREQ) {
        return TONE_FREQ_ERROR; // frequency is too low, allocation failed. 
    }
    if (tone_buffer != NULL) {
        tone_deinit();
        return -1*tone_init(sample_hz)-1; // already initialized, reinitializing
    }
    
    sound_init(sample_hz);

    tone_buffer_length = sample_hz/LOWEST_FREQ;

    tone_buffer = malloc(tone_buffer_length * sizeof(tone_buffer_sample_t));

    return 0; // success!
}

// Free resources used for tone generation (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t tone_deinit(void)
{
    if (tone_buffer == NULL) {
        return 1; // already deinitialized
    } else {
        free(tone_buffer);
        tone_buffer = NULL;
        tone_buffer_length = 0;
        sound_deinit();
        return 0; // success
    }
    
}

// Start playing the specified tone.
// tone: one of the enumerated tone types.
// freq: frequency of the tone in Hz.
void tone_start(tone_t tone, uint32_t freq)
{
    if (tone < 0 || tone > TONE_ENUM_MAX_VALUE) {
        return; // invalid tone enum
    }
    if (freq < LOWEST_FREQ) {
        return;
    }
    uint32_t sample_freq = tone_buffer_length * LOWEST_FREQ;
    printf("sample_freq: %ld\n", sample_freq);
    printf("freq: %ld\n", freq);
    if (freq > sample_freq/2) {
        return; // frequency is too high.
    }


    uint32_t samples_per_period = sample_freq / freq;
    if (samples_per_period > tone_buffer_length) {
        return ;
    }
    printf("samples_per_period: %ld\n", samples_per_period);

    float step = 2.0*M_PI / (float) samples_per_period; 
    printf("step: %f\n", step);

    // generates different waveforms depending on the input. 
    uint32_t i = 0;
    switch (tone) {
        case SINE_T:
            // fills tone buffer with a sine wave, with a period of
            // samples_per_period when measured in index. 
            for (i = 0; i < samples_per_period; i++) {
                float fval = sinf(step*i);
                int16_t ival = ((int16_t) (fval * TONE_BIAS_LEVEL)) + TONE_BIAS_LEVEL;
                if (ival < 0) {
                    ival = 0;
                }
                if (ival > TONE_MAX_LEVEL) {
                    ival = TONE_MAX_LEVEL;
                }
                tone_buffer[i] = (uint8_t) ival;
            }
            break;
        case SQUARE_T:
            
            // Sets the first half of i in a period to TONE_MAX_LEVEL,
            // the second half of to 0. 
            for (i = 0; i < samples_per_period; i++) {
                if ( ((2*i)/(samples_per_period)) % 2 == 0 ) {
                    tone_buffer[i] = TONE_MAX_LEVEL;
                } else {
                    tone_buffer[i] = 0;
                }
            }
            break;
        case TRIANGLE_T:
            // constructs a triangular wave, starting at TONE_BIAS_LEVEL, going up
            // to TONE_MAX_LEVEL, going down to 0, and coming back up to TONE_BIAS_LEVEL.
            for (i = 0; i < samples_per_period; i++) {
                float fourth = samples_per_period/4;
                uint32_t ifourth = fourth;

                // below is a piecewise triangle wave function. The middle branch
                // contains samples_per_period%4 extra points, so special consideration
                // is taken for it.
                if (i < ifourth) {
                    tone_buffer[i] = ( ((TONE_MAX_LEVEL-TONE_BIAS_LEVEL)/((float)ifourth))*i ) + TONE_BIAS_LEVEL;
                } else if (i < 3*ifourth + samples_per_period%4) {
                    tone_buffer[i] = ( ((0-TONE_MAX_LEVEL)/(2.0*fourth+samples_per_period%4))*i ) 
                        + TONE_MAX_LEVEL + ifourth*(TONE_MAX_LEVEL/(2.0*ifourth+samples_per_period%4));
                } else {
                    tone_buffer[i] = ( ((TONE_BIAS_LEVEL-0)/(float)ifourth)*(i-samples_per_period%4) ) - TONE_MAX_LEVEL - TONE_BIAS_LEVEL;
                }
                
            }
            break;
        case SAW_T:
            for (i = 0; i < samples_per_period; i++) {
                // relies on integar overflow for the sawtooth form.
                tone_buffer[i] = (TONE_MAX_LEVEL/(float)samples_per_period)*i + TONE_BIAS_LEVEL;
            }
            break;
        case LAST_T:
            return; //another invalid tone enum
            break;
        default: 
            return; //invalid tone enum
            break;
    }

    sound_cyclic(tone_buffer, samples_per_period);

}