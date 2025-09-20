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

float* cVols;


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
	cVols = getVols();

	for (int i = 0; i < halfChunk; ++i) {
		//Clear previous data on this sample
		chunks[chunkIndex][i * 2] = 0;
		chunks[chunkIndex][i * 2 + 1] = 0;

		float time = i * timeMult;

		for (int osc = 0; osc < oscAmount; osc++) {
			if (cVols[osc * 2] != 0.0f || cVols[osc * 2 + 1] != 0.0f || cVols[osc * 2 + oscs2] != 0.0f || cVols[osc * 2 + 1 + oscs2] != 0.0f) {

				int sIndex = (int)((totalOffset + i) * mtfs[osc] + sineStarts[osc]) % sineLength;

				float realVolL = lerp(cVols[osc * 2 + oscs2], cVols[osc * 2], time);
				float realVolR = lerp(cVols[osc * 2 + 1 + oscs2], cVols[osc * 2 + 1], time);

				chunks[chunkIndex][i * 2] += sineWave[sIndex] * realVolL * inVol; //LEFT
				chunks[chunkIndex][i * 2 + 1] += sineWave[sIndex] * realVolR * inVol; //RIGHT
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