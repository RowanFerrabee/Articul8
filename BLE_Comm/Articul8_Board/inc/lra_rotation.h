#ifndef LRA_ROTATION_H
#define LRA_ROTATION_H

#include "lra_man.h"

#define LRA_MAX_FREQUENCY 10.0f
#define LRA_MIN_FREQUENCY 0.01f

void lra_rotate_setup(float freq=1);
void lra_rotate_setIntensity(int intensity);
void lra_rotate_setFrequency(float angularFrequency);
void lra_rotate_count(int ms);
void lra_rotate_getOutputs(int* intensities, bool* changed);

#endif