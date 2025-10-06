#include "headers.h"

const int cKey[] = { 0, 2, 4, 5, 7, 9, 11 };
const int rhy1[] = { 1, 1, 1, 1, 0, 0, 1, 1 };
const int rhy2[] = { 1, 0, 0, 0, 0, 1, 0, 0 };
const int rhy3[] = { 0, 0, 0, 0, 1, 0, 0, 0 };

int cRhy = 0;

static void synthesize(
	struct adsr vEnv,
	int freq,
	struct adsr pEnv,
	float pEnvAmt,
	float gain,

	//Filter
	float loMin,
	float hiMin,
	int center,
	char fKeyTrack,
	int distance,
	int minDistance,
	float fc,
	float falloffTime,

	//Eqs
	int eq1Freq,
	float eq1Amt,
	int eq2Freq,
	float eq2Amt,
	int eq3Freq,
	float eq3Amt,
	int eqDist,
	float eqMin,
	
	//Modulation
	float vOscRate,
	float vOscAmt,
	float pOscRate,
	float pOscAmt,

	//Timbre
	int harDist,
	float noiseAmt,
	float noiseCurve

	) {


	float a = vEnv.l * vEnv.ap;
	float d = vEnv.l * vEnv.dp;
	int stepLength = (int)((vEnv.l + vEnv.r) * CPS);

	float pa = pEnv.l * pEnv.ap;
	float pd = pEnv.l * pEnv.dp;

	int rCenter = center * res;
	if (fKeyTrack == 1) { rCenter = freq * res; }
	int rDistance = distance * res;
	int rMinDistance = minDistance * res;
	float distInv = 1.0f / rDistance;
	float rFalloffTime = 1.0f / falloffTime;

	static float eqs[OAMacro];

	static float harValues[OAMacro][2];
	static float noiseValues[noisesAmt];

	//Frequency Filters
	for (int f = 0; f < oscAmount; f++) {

		harValues[f][0] = ranf();
		harValues[f][1] = ranf();

		eqs[f] += envEq(f, eq1Freq, eqDist) * eq1Amt;
		eqs[f] += envEq(f, eq2Freq, eqDist) * eq2Amt;
		eqs[f] += envEq(f, eq3Freq, eqDist) * eq3Amt;
		eqs[f] = lerp(eqs[f], 1.0f, eqMin);
	}

	
	
	//Rendering pass
	int i;
	#pragma omp parallel for num_threads(activeThreads)
	for (i = 0; i < stepLength; i++) {
		float time = i * cpsInv;
		float v = envADSR(time, vEnv.l, a, d, vEnv.s, vEnv.r, vEnv.c) * gain; //Volume envelope
		float p = envADSR(time, pEnv.l, pa, pd, pEnv.s, pEnv.r, pEnv.c) * pEnvAmt * res; //Pitch envelope
		if (time >= pEnv.l) { p = 0.0f; }

		//Harmonics
		for (int h = 0; h < 100; h++) {

			float index = harmonic(freq, h * harDist) * res;
			if (index >= oscAmount) { break; } //If the harmonic is out of bounds, stop

			//Made the cutoff distance relative to the volume of the note?
			int cDistance = lerp(rMinDistance, rDistance, v);
			
			//FILTERING
			float lowHighPass = 1.0f;
			//Low pass
			if (index >= rCenter && index < rCenter + cDistance) {
				lowHighPass = lerp(powf(1.0f - ((index - rCenter) * distInv), fc), 1.0f, hiMin);
			}
			//High pass
			if (index < rCenter && index > rCenter - cDistance) {
				lowHighPass = lerp(powf((index - (rCenter - cDistance)) * distInv, fc), 1.0f, loMin);
			}
			//Outside of ranges
			if (index >= rCenter + cDistance) { lowHighPass = hiMin; }
			if (index <= rCenter - cDistance) { lowHighPass = loMin; }

			lowHighPass *= eqs[(int)index];


			
			

			setFFV(i, index + p + osc(time * pOscRate, pOscAmt), v * lowHighPass * osc(time * vOscRate, vOscAmt) * harValues[(int)index][0], 0);
			setFFV(i, index + p + osc(time * pOscRate, pOscAmt), v * lowHighPass * osc(time * vOscRate, vOscAmt) * harValues[(int)index][1], 1);

			//Octave down render for no reason?
			setFFV(i, index + p + osc(time * pOscRate, pOscAmt) - 12.0f * res, v * lowHighPass * osc(time * vOscRate, vOscAmt) * harValues[(int)index][0], 0);
			setFFV(i, index + p + osc(time * pOscRate, pOscAmt) - 12.0f * res, v * lowHighPass * osc(time * vOscRate, vOscAmt) * harValues[(int)index][1], 1);
		}

		//Noise
		for (int n = 0; n < noisesAmt; n++) {

			float height = powf(1.0f - n / 10.0f, noiseCurve);

			//nSetFV(i, n, v * noiseAmt * height, 2);
		}
	}
}


void generate() {

	//Init Seeding
	for (int t = 0; t < time(NULL) % 256; t++) { ranf(); }
	
	
	bool nTrigger = false;

	int cFor = 0;
	if (ranf() < 0.07f && cStep % 3 == 0) { nTrigger = true; cFor = rani(0, 1); }

	

	for (int note = 0; note < rani(1, 6); note++) {

		//If note isnt triggered, goto next note
		if (!nTrigger) { continue; }
		//else...

		//MAKE ATTACK/DECAY 1 VALUE (DECAY = 1f - AP)
		struct adsr s1v = { 8.0f, ranfIn(0.0f, 0.5f), 0.5f, 0.5f, 5.0f, 1.9f};
		struct adsr s1p = { 1.9f, 0.0f, 0.9f, 0.0f, 0.0f, 1.9f };
		synthesize(
			s1v, //vEnv
			k2m(rani(24, 38) + note * 2, cKey), //Freq
			s1p, //pEnv
			-0.5f, //pEnvAmt
			ranfIn(0.8f, 0.99f), //Gain

			1.0f, //loMin
			0.0f, //hiMin
			40, //Filter center
			0, //Filter key tracking
			100, //Filter distance
			10, //Filter min distance
			3.0f, //Filter curve
			2.0f, //Filter Falloff time

			//EQs
			getFormant(cFor, 0),
			1.0f,
			getFormant(cFor, 1),
			1.0f,
			getFormant(cFor, 2),
			0.5f,
			9 * res, //eqDist
			0.9f, //eqMin

			rani(2, 5), //vOscRate
			ranfIn(0.0f, 0.1f), //vOscAmt

			ranfIn(18.0f, 19.0f), //pOscRate
			ranfIn(0.0f, 0.05f) * res, //pOscAmt

			rani(1, 5), //harDist
			ranfIn(0.0f, 0.02f), //Noise amount
			0.5f //Noise curve
		);
		
	}
	
}


//Converts float values into bytes(chars) for storage in the buffer
void setFV(int offStep, int freq, float val, int channel) {
	if (freq >= oscAmount || freq < 0) { return; }
	
	int pos = (cStep + offStep) % bufferLength;
	uint16_t v = clampf(val, 0.0f, 1.0f) * 255;

	if (channel == 2) { 
		buffer[pos][freq][0] = clamp((uint16_t)buffer[pos][freq][0] + v, 0, 255); 
		buffer[pos][freq][1] = clamp((uint16_t)buffer[pos][freq][1] + v, 0, 255);
	}
	else { buffer[pos][freq][channel] = clamp((uint16_t)buffer[pos][freq][channel] + v, 0, 255);
	}
}
void nSetFV(int offStep, int freq, float val, int channel) {
	if (freq >= noisesAmt || freq < 0) { return; }

	int pos = (cStep + offStep) % bufferLength;
	uint16_t v = clampf(val, 0.0f, 1.0f) * 255;

	if (channel == 2) {
		nBuffer[pos][freq][0] = clamp((uint16_t)nBuffer[pos][freq][0] + v, 0, 255);
		nBuffer[pos][freq][1] = clamp((uint16_t)nBuffer[pos][freq][1] + v, 0, 255);
	}
	else {
		nBuffer[pos][freq][channel] = clamp((uint16_t)nBuffer[pos][freq][channel] + v, 0, 255);
	}
}

//Same as above but using float frequency
void setFFV(int offStep, float freq, float val, int channel) {
	if (freq >= oscAmount - 1 || freq < 0) { return; }

	int f1 = (int)freq;
	int f2 = f1 + 1;
	float diff = freq - f1;

	int pos = (cStep + offStep) % bufferLength;
	uint16_t v1 = clampf(val * sqrtf(1.0f - diff), 0.0f, 1.0f) * 255;
	uint16_t v2 = clampf(val * sqrtf(diff), 0.0f, 1.0f) * 255;

	if (channel == 2) {
		buffer[pos][f1][0] = clamp((uint16_t)buffer[pos][f1][0] + v1, 0, 255);
		buffer[pos][f1][1] = clamp((uint16_t)buffer[pos][f1][1] + v1, 0, 255);

		buffer[pos][f2][0] = clamp((uint16_t)buffer[pos][f2][0] + v2, 0, 255);
		buffer[pos][f2][1] = clamp((uint16_t)buffer[pos][f2][1] + v2, 0, 255);
	}
	else {
		buffer[pos][f1][channel] = clamp((uint16_t)buffer[pos][f1][channel] + v1, 0, 255);
		buffer[pos][f2][channel] = clamp((uint16_t)buffer[pos][f2][channel] + v2, 0, 255);
	}

}

//Sends the current + previous volumes from the buffer to the renderer
float* getVols() {
	static float vols[OAMacro * 4]; //Current and previous vols

	int prev = cStep - 1;
	if (prev < 0) { prev = bufferLength - 1; }

	for (int osc = 0; osc < oscAmount; osc++) {

		//Current step
		float l = buffer[cStep][osc][0] * byteMult;
		float r = buffer[cStep][osc][1] * byteMult;
		vols[osc * 2] = l * l * activeMult;
		vols[osc * 2 + 1] = r * r * activeMult;

		//Previous step
		l = buffer[prev][osc][0] * byteMult;
		r = buffer[prev][osc][1] * byteMult;
		vols[osc * 2 + OAMacro * 2] = l * l * activeMult;
		vols[osc * 2 + 1 + OAMacro * 2] = r * r * activeMult;

		//Clear previous step
		buffer[prev][osc][0] = 0;
		buffer[prev][osc][1] = 0;
	}
	

	//Advance step only after the vols have all been transmitted
	cStep++; if (cStep >= bufferLength) { cStep -= bufferLength; }
	gStep++;
	return vols;
}

float* getNoises() {
	static float noises[noisesAmt * 4];

	int prev = cStep - 1;
	if (prev < 0) { prev = bufferLength - 1; }

	for (int i = 0; i < noisesAmt; i++) {

		//Current step
		float l = nBuffer[cStep][i][0] * byteMult;
		float r = nBuffer[cStep][i][1] * byteMult;
		noises[i * 2] = l * l * noiseMult;
		noises[i * 2 + 1] = r * r * noiseMult;

		//Previous step
		l = nBuffer[prev][i][0] * byteMult;
		r = nBuffer[prev][i][1] * byteMult;
		noises[i * 2 + 20] = l * l * noiseMult;
		noises[i * 2 + 1 + 20] = r * r * noiseMult;

		//Clear previous step
		nBuffer[prev][i][0] = 0;
		nBuffer[prev][i][1] = 0;
	}
	return noises;
}

int getStep() { return gStep; }