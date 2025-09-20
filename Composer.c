#include "headers.h"


const int cKey[] = { 0, 2, 4, 5, 7, 9, 11 };

void setFV(int offStep, int freq, float val) {
	int pos = (cStep + offStep) % bufferLength;
	char v = (char)(clampf(val, 0.0f, 1.0f) * 255);
	buffer[pos][freq][0] += v;
	buffer[pos][freq][1] += v;
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

void generate() {

	for (int n = 0; n < 4; n++) {
		bool nTrigger = false;

		if (ranf() < 0.01f) { nTrigger = true; }

		if (nTrigger) {
			float l = 1.0f;
			float a = l * 0.0f;
			float d = l * 0.8f;
			float s = 1.0f;
			float r = ranfIn(2.0f, 9.0f);
			float c = 1.0f;
			int freq = k2m(rani(15, 25), cKey);
			int stepLength = (int)((l + r) * CPS);
			float gain = 0.9f;

			static float filter[OAMacro];
			float loMin = 1.0f;
			float hiMin = 0.0f;
			int center = freq * res;
			int distance = 10 * res;
			float distInv = 1.0f / distance;
			float fc = 2.0f;

			float falloffTime = 1.0f / 4.0f;
			float falloffCurve = 0.5f;
			float sharpness = 1.0f;

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

				//Harmonics
				for (int h = 0; h < 100; h++) {

					int index = harmonic(freq, h * 1) * res;

					float height = 1.0f - index * oscInv;

					float sharpVal = lerp(filter[index], powf(height, falloff), sharpness);
					float realFilter = lerp(sharpVal, filter[index], falloff);

					//If the harmonic is out of bounds, stop making harmonics
					if (index < oscAmount) {
						setFV(i, index, v * realFilter * harValues[index]);
					}
					else { break; }
				}

				//while loops to do harmonics  
				//while cFreq < oscAmount, keep adding harmonics etc
			}
		}
	}

}