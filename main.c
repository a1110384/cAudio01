#include "headers.h"

HWAVEOUT wave_out;
void CALLBACK WaveOutProc(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);

int main() {
	
	WAVEFORMATEX format = setFormat();
	waveOutOpen(&wave_out, WAVE_MAPPER, &format, (DWORD_PTR)WaveOutProc, (DWORD_PTR)NULL, CALLBACK_FUNCTION);
	startRenderer(wave_out);

	initUI(); printVolume(mVol);

	int c = 0;
	while (c != 27) {
		switch (c = _getch()) {
		case 75: { //LEFT ARROW
			mVol = clampf(mVol - 0.05f, 0.0f, 1.0f);
			printVolume(mVol);
		} break;
		case 77: { //RIGHT ARROW
			mVol = clampf(mVol + 0.05f, 0.0f, 1.0f);
			printVolume(mVol);
		} break;
		case 32: { //SPACEBAR
			if (playing) { waveOutPause(wave_out); }
			else { waveOutRestart(wave_out); writeLoop(wave_out); } //Fix
		} break;
		}
	}
	return 0;
}

void CALLBACK WaveOutProc(HWAVEOUT wave_out_handle, UINT message, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2) {
	if (message == WOM_DONE) { renderSamples(mVol); writeLoop(wave_out); } //Render and write the buffers
}