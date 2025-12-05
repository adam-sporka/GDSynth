#pragma once

#include <stdint.h>

#define NUM_EVENT_SLOTS 8
#define SAMPLE_RATE 48000
#define NUM_CHANNELS 2
#define BUFLEN 512
#define PI 3.1415926535f
#define EPSILON 0.001

using TParamName = int;
using TParamValue = float;
using TSample = float;

using TFloatBuffer = TSample[BUFLEN][NUM_CHANNELS];
using TIntBuffer = int16_t[BUFLEN][NUM_CHANNELS];