#include "headers.h"

const int cKey[] = { 0, 2, 4, 5, 7, 9, 11 };
const int rhy1[] = { 1, 1, 1, 1, 0, 0, 1, 1 };
const int rhy2[] = { 1, 0, 0, 0, 0, 1, 0, 0 };
const int rhy3[] = { 0, 0, 0, 0, 1, 0, 0, 0 };

int cRhy = 0;

static void synthesize(float l,
	float ap,
	float dp,
	float s,
	float r,
	float c,
	int freq,
	float gain,

	float loMin,
	float hiMin,
	int center,
	char fKeyTrack,
	int distance,
	float fc,

	float falloffTime,
	float falloffCurve,
	float sharpness,

	//eqs
	int eq1Freq,
	float eq1Amt,
	int eq2Freq,
	float eq2Amt,
	int eq3Freq,
	float eq3Amt,
	int eqDist,
	float eqMin,
	
	float vOscRate,
	float vOscAmt,

	int harDist,
	float noiseAmt,
	float noiseCurve

	) {


	float a = l * ap;
	float d = l * dp;
	int stepLength = (int)((l + r) * CPS);

	static float filter[OAMacro];
	int rCenter = center * res;
	if (fKeyTrack == 1) { rCenter = freq * res; }
	int rDistance = distance * res;
	float distInv = 1.0f / rDistance;
	float rFalloffTime = 1.0f / falloffTime;

	static float eqs[OAMacro];

	static float harValues[OAMacro][2];
	static float noiseValues[noisesAmt];

	
	//Frequency Filters
	for (int f = 0; f < oscAmount; f++) {
		//Low pass
		if (f >= rCenter && f < rCenter + rDistance) {
			filter[f] = lerp(powf(1.0f - ((f - rCenter) * distInv), fc), 1.0f, hiMin);
		}
		//High pass
		if (f < rCenter && f > rCenter - rDistance) {
			filter[f] = lerp(powf((f - (rCenter - rDistance)) * distInv, fc), 1.0f, loMin);
		}
		//Outside of ranges
		if (f >= rCenter + rDistance) { filter[f] = hiMin; }
		if (f <= rCenter - rDistance) { filter[f] = loMin; }

		harValues[f][0] = ranf();
		harValues[f][1] = ranf();

		eqs[f] += envEq(f, eq1Freq, eqDist) * eq1Amt;
		eqs[f] += envEq(f, eq2Freq, eqDist) * eq2Amt;
		eqs[f] += envEq(f, eq3Freq, eqDist) * eq3Amt;
		eqs[f] = lerp(eqs[f], 1.0f, eqMin);
	}

	
	
	//Rendering pass
	for (int i = 0; i < stepLength; i++) {
		float time = i * cpsInv;
		float v = envADSR(time, l, a, d, s, r, c) * gain;

		float falloff = powf(time * rFalloffTime, falloffCurve);
		int pitchOsc = 0;

		//Harmonics
		for (int h = 0; h < 100; h++) {

			int index = harmonic(freq, h * harDist) * res;

			float height = 1.0f - index * oscInv;

			float sharpVal = lerp(filter[index], powf(height, falloff), sharpness);
			float realFilter = lerp(sharpVal, filter[index], falloff) * eqs[index];

			//If the harmonic is out of bounds, stop making harmonics
			if (index >= oscAmount) { break; }

			setFV(i, index, v * realFilter * osc(time * vOscRate, vOscAmt) * harValues[index][0], 0);
			setFV(i, index, v * realFilter * osc(time * vOscRate, vOscAmt) * harValues[index][1], 1);

			setFV(i, index - 12 * res, v * realFilter * osc(time * vOscRate, vOscAmt) * harValues[index][0], 0);
			setFV(i, index - 12 * res, v * realFilter * osc(time * vOscRate, vOscAmt) * harValues[index][1], 1);
		}

		//Noise
		for (int n = 0; n < noisesAmt; n++) {

			float height = powf(1.0f - n / 10.0f, noiseCurve);

			nSetFV(i, n, v * noiseAmt * height, 2);
		}
	}
}


void generate() {
	
	bool nTrigger = false;

	int cFor = 0;
	if (ranf() < 0.07f && cStep % 3 == 0) { nTrigger = true; cFor = rani(0, 5); }


	for (int note = 0; note < rani(1, 6); note++) {
		

		//If note isnt triggered, goto next note
		if (!nTrigger) { continue; }
		//else...


		synthesize(ranfIn(4.0f, 8.0f), //Length
			ranfIn(0.0f, 0.3f), //Attack percent
			0.5f, //Decay percent
			0.8f, //Sustain
			ranfIn(4.0f, 9.0f), //Release
			0.9f, //Curve
			k2m(rani(22, 35) + note * 2, cKey), //Freq
			0.7f, //Gain

			1.0f, //loMin
			0.0f, //hiMin
			40, //Filter center
			0, //Filter key tracking
			rani(47, 58), //Filter distance
			1.0f, //Filter curve

			4.0f, //Filter Falloff time
			0.9f, //Falloff curve
			ranfIn(0.0f, 0.3f), //Sharpness

			//EQs
			getFormant(cFor, 0),
			1.0f,
			getFormant(cFor, 1),
			1.0f,
			getFormant(cFor, 2),
			0.5f,
			3 * res, //eqDist
			ranfIn(0.1f, 0.99f), //eqMin

			rani(1, 5), //vOscRate
			ranfIn(0.0f, 0.4f), //vOscAmt

			rani(1, 5), //harDist
			ranfIn(0.0f, 0.05f), //Noise amount
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