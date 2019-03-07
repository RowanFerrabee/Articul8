#include "lra_rotation.h"
#include <cstdlib>
#include <cstring>

#define MIN(x, y) ( (x) < (y) ? (x) : (y) )
#define MAX(x, y) ( (x) < (y) ? (y) : (x) )
#define LIMIT(x, l, u) MAX(MIN(x,u),l)
#define PI (3.1415926)

// privates
int m_intensity = LRA_MIN_INTENSITY;
float m_frequency = 1;
float m_angle = 0;
float bounds[LRA_NUM_OUTPUTS];
float cachedIntensities[LRA_NUM_OUTPUTS];

void lra_setup(float freq)
{
	m_frequency = freq;
	float step = 2*PI/LRA_NUM_OUTPUTS;
	for(int i = 0; i < LRA_NUM_OUTPUTS; ++i)
		bounds[i] = step*i;
}

void lra_count(int ms) { 
	m_angle += ms*m_frequency * (2*PI/1000);
	while(m_angle > 2*PI)
		m_angle -= 2*PI;

}

void lra_getOutputs(int* intensities, bool* changed)
{
	int lowerInd, upperInd = 1;
	while(m_angle > bounds[upperInd] && upperInd < LRA_NUM_OUTPUTS)
		upperInd++;

	lowerInd = upperInd - 1;
	if(upperInd == LRA_NUM_OUTPUTS) upperInd = 0;
	
	memset(intensities, 0, sizeof(int) * LRA_NUM_OUTPUTS);

	const float step = 2*PI/LRA_NUM_OUTPUTS;
	float lowerDist = m_angle - bounds[lowerInd];
	float upperDist = step - lowerDist;

	intensities[lowerInd] = m_intensity * upperDist / step;
	intensities[upperInd] = m_intensity * lowerDist / step;

	for(int i = 0; i < LRA_NUM_OUTPUTS; ++i)
		changed[i] = intensities[i] != cachedIntensities[i];

	memcpy(cachedIntensities, intensities, sizeof(int)*LRA_NUM_OUTPUTS);
}	

void lra_setIntensity(int intensity) { 
	m_intensity = LIMIT(intensity, LRA_MIN_INTENSITY, LRA_MAX_INTENSITY);
}

void lra_setFrequency(float angularFrequency) { 
	m_frequency = LIMIT(angularFrequency, LRA_MIN_FREQUENCY, LRA_MAX_FREQUENCY);
}