#ifndef LRA_ROTATION_H
#define LRA_ROTATION_H

#define LRA_MAX_INTENSITY 255
#define LRA_MIN_INTENSITY 0
#define LRA_MAX_FREQUENCY 10.0f
#define LRA_MIN_FREQUENCY 0.01f

#define LRA_NUM_OUTPUTS 8

void lra_setup(float freq=1);
void lra_setIntensity(int intensity);
void lra_setFrequency(float angularFrequency);
void lra_count(int ms);
void lra_getOutputs(int* intensities, bool* changed);

#endif