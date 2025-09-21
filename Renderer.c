#include "headers.h"

#define TWOPI 6.283185
#define SHORT_MIN -32768
#define SHORT_MAX 32767

WAVEHDR header[CHUNK_AMT] = { 0 };
int16_t chunks[CHUNK_AMT][CHUNK_SIZE];
short chunkIndex = 0;

#define sineLength 1024
short sineWave[sineLength];
double lengthMult;
long totalOffset = 0;

#define blendAmount 0.9f
float timeMult;

double mtfs[midiTotal * res];
short sineStarts[midiTotal * res];
short noiseWave[noiseLength];

float* cVols;
float* cNoises;


WAVEFORMATEX setFormat() {
	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = SAMPLE_RATE;
	format.wBitsPerSample = 16;
	format.cbSize = 0;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	return format;
}

void startRenderer(HWAVEOUT waveOut) {

	cpsInv = 1.0f / CPS;
	oscMult = sqrt(1.0 / oscAmount);
	oscs2 = oscAmount * 2;
	oscInv = 1.0f / oscAmount;

	activeMult = sqrtf(1.0f / activeOscs);

	lengthMult = sineLength / (double)SAMPLE_RATE;
	timeMult = (1.0f / (halfChunk * blendAmount));

	for (int i = 0; i < sineLength; i++) {
		sineWave[i] = sinf(i * TWOPI / sineLength) * SHORT_MAX;
	}

	for (int osc = 0; osc < oscAmount; osc++) {
		mtfs[osc] = mtf((osc / resF) + 16) * lengthMult;
		sineStarts[osc] = rand() % 44100;
	}
	
	for (int i = 0; i < noiseLength; i++) {
		//noiseWave[i] = (short)(((uint8_t)noiseChars[i * 2] << 8) | (uint8_t)noiseChars[i * 2 + 1]);
		noiseWave[i] = rani(SHORT_MIN, SHORT_MAX);
	}
	



	waveOutSetVolume(waveOut, 0xFFFFFFFF);
	for (int i = 0; i < CHUNK_AMT; ++i) {
		chunkIndex = i; renderSamples(0.2f);
		header[i].lpData = (CHAR*)chunks[i];
		header[i].dwBufferLength = CHUNK_SIZE * 2;

		waveOutPrepareHeader(waveOut, &header[i], sizeof(header[i]));
		waveOutWrite(waveOut, &header[i], sizeof(header[i]));
	}
	chunkIndex = 0;
}

void renderSamples(float inVol) {

	generate();
	cNoises = getNoises(); //Get noises first (cStep advances in getVols)
	cVols = getVols();

	for (int i = 0; i < halfChunk; ++i) {
		//Clear previous data on this sample
		chunks[chunkIndex][i * 2] = 0;
		chunks[chunkIndex][i * 2 + 1] = 0;

		float time = i * timeMult;

		//Sine rendering
		for (int osc = 0; osc < oscAmount; osc++) {
			if (cVols[osc * 2] > 0.0f || cVols[osc * 2 + 1] > 0.0f || cVols[osc * 2 + oscs2] > 0.0f || cVols[osc * 2 + 1 + oscs2] > 0.0f) {

				int index = (int)((totalOffset + i) * mtfs[osc] + sineStarts[osc]) % sineLength;

				float realVolL = lerp(cVols[osc * 2 + oscs2], cVols[osc * 2], time);
				float realVolR = lerp(cVols[osc * 2 + 1 + oscs2], cVols[osc * 2 + 1], time);

				chunks[chunkIndex][i * 2] += sineWave[index] * realVolL * inVol; //LEFT
				chunks[chunkIndex][i * 2 + 1] += sineWave[index] * realVolR * inVol; //RIGHT
			}
		}

		//Noise rendering
		for (int n = 0; n < noisesAmt; n++) {
			if (cNoises[n * 2] > 0.0f || cNoises[n * 2 + 1] > 0.0f || cNoises[n * 2 + 20] > 0.0f || cNoises[n * 2 + 1 + 20] > 0.0f) {

				//ADD LERPING for proper resampling
				int index = (int)((totalOffset + i) * nSpeeds[n]) % noiseLength;

				float volL = lerp(cNoises[n * 2], cNoises[n * 2], time);
				float volR = lerp(cNoises[n * 2 + 1], cNoises[n * 2 + 1], time);

				chunks[chunkIndex][i * 2] += noiseWave[index] * volL * inVol; //LEFT
				chunks[chunkIndex][i * 2 + 1] += noiseWave[index] * volR * inVol; //RIGHT
			}
		}

		//Clamp values
		chunks[chunkIndex][i * 2] = clamp(chunks[chunkIndex][i * 2], SHORT_MIN, SHORT_MAX);
		chunks[chunkIndex][i * 2 + 1] = clamp(chunks[chunkIndex][i * 2 + 1], SHORT_MIN, SHORT_MAX);
	}
	totalOffset += halfChunk;
}

void writeLoop(HWAVEOUT waveOut) {
	waveOutWrite(waveOut, &header[chunkIndex], sizeof(header[chunkIndex]));
	chunkIndex = (chunkIndex + 1) % CHUNK_AMT;
}