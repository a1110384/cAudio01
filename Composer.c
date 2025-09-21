#include "headers.h"

const int cKey[] = { 0, 2, 4, 5, 7, 9, 11 };
const int rhy1[] = { 1, 0, 0, 1, 0, 0, 1, 0 };

int cRhy = 0;

void generate() {

	for (int note = 0; note < 1; note++) {
		bool nTrigger = false;
		if (cStep % 16 == 0) cRhy++;
		if (rhy1[cRhy % 8] && cStep % 16 == 0) { nTrigger = true; }

		//If note isnt triggered, goto next note
		if (!nTrigger) { continue; }
		//else...

		float l = 3.5f;
		float a = l * 0.5f;
		float d = l * 0.5f;
		float s = 1.0f;
		float r = 1.0f;
		float c = 1.0f;
		int freq = k2m(rani(15, 25), cKey);
		int stepLength = (int)((l + r) * CPS);
		float gain = 0.9f;

		static float filter[OAMacro];
		float loMin = 1.0f;
		float hiMin = 0.0f;
		int center = freq * res;
		int distance = 50 * res;
		float distInv = 1.0f / distance;
		float fc = 2.0f;

		float falloffTime = 1.0f / 4.0f;
		float falloffCurve = 0.9f;
		float sharpness = 0.4f;

		static float harValues[OAMacro];

		for (int f = 0; f < oscAmount; f++) {
			//Low pass
			if (f >= center && f < center + distance) {
				filter[f] = lerp(powf(1.0f - ((f - center) * distInv), fc), 1.0f, hiMin);
			}
			//High pass
			if (f < center && f > center - distance) {
				filter[f] = lerp(powf((f - (center - distance)) * distInv, fc), 1.0f, loMin);
			}
			//Outside of ranges
			if (f >= center + distance) { filter[f] = hiMin; }
			if (f <= center - distance) { filter[f] = loMin; }

			harValues[f] = ranf();
		}

		for (int i = 0; i < stepLength; i++) {
			float time = i * cpsInv;
			float v = envADSR(time, l, a, d, s, r, c) * gain;

			float falloff = powf(time * falloffTime, falloffCurve);

			int pitchOsc = (int)(sineWave[i * 1000] / 32000.0f * 4.0f);

			//Harmonics
			for (int h = 0; h < 100; h++) {

				int index = harmonic(freq, h * 1) * res;

				float height = 1.0f - index * oscInv;

				float sharpVal = lerp(filter[index], powf(height, falloff), sharpness);
				float realFilter = lerp(sharpVal, filter[index], falloff);

				//If the harmonic is out of bounds, stop making harmonics
				if (index >= oscAmount) { break; }

				setFV(i, index + pitchOsc, v * realFilter * harValues[index], 2);
				setFV(i, index - 12 * res + pitchOsc, v * realFilter * harValues[index - 12 * res], 2);
			}
			//
			//nSetFV(i, 5, v);

			//while loops to do harmonics  
			//while cFreq < oscAmount, keep adding harmonics etc
		}
	}

}

void setFV(int offStep, int freq, float val, int channel) {
	if (freq < oscAmount && freq > 0) {
		int pos = (cStep + offStep) % bufferLength;
		char v = (char)(clampf(val, 0.0f, 1.0f) * 255);
		if (channel == 2) {
			buffer[pos][freq][0] += v; buffer[pos][freq][1] += v;
		}
		else { buffer[pos][freq][channel] += v; }
	}
}
void nSetFV(int offStep, int freq, float val) {
	int npos = (cStep + offStep) % bufferLength;
	char nv = (char)(clampf(val, 0.0f, 1.0f) * 255);
	nBuffer[npos][freq][0] += nv;
	nBuffer[npos][freq][1] += nv;
}

float* getVols() {
	static float vols[midiTotal * res * 4]; //Current and previous vols

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
		vols[osc * 2 + oscs2] = l * l * activeMult;
		vols[osc * 2 + 1 + oscs2] = r * r * activeMult;

		//Clear previous step
		buffer[prev][osc][0] = 0;
		buffer[prev][osc][1] = 0;
	}

	debugC(gStep);

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