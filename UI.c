#include "headers.h"
#include "fontDef.h"

#define ftc(f) (unsigned char)(f * 255.0f)
#define ctf(c) (float)c * byteMult

static char volDisp[64][OAMacro][2];
static int volDispCurr = 0;


void setupWindow(HINSTANCE hInstance) {
	static WNDCLASSEX wcx = { 0 };
	wcx.cbSize = sizeof(wcx);
	wcx.hInstance = hInstance;
	wcx.lpszClassName = winClassName;
	wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcx.lpfnWndProc = winMsgProc;
	RegisterClassEx(&wcx);

	hwnd = CreateWindowEx(WS_EX_STATICEDGE,
		winClassName, winClassName,
		WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		640, 300,
		winW, winH,
		NULL, NULL, hInstance, NULL);
	if (hwnd == NULL) { return -1; }

	// Set the window corner preference for Windows 11
	DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
	DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));

	winWInv = 1.0f / (float)winW; winHInv = 1.0f / (float)winH;
}

void background() {

	for (int x = 0; x < winW; x++) {
		for (int y = 0; y < winH; y++) {
			pixel(x, y, colc(rani(10, 20), rani(10, 30), rani(10, 40)));
		}
	}
}

void box(int x, int y, int xs, int ys, uint32_t col) {
	//Stupid box drawing thing
	if (xs < 0 && ys > 0) { 
		for (int xp = x; xp > x + xs; xp--) {
			for (int yp = y; yp < y + ys; yp++) {
				pixel(xp, yp, col);
			}
		}
	}
	if (xs > 0 && ys < 0) {
		for (int xp = x; xp < x + xs; xp++) {
			for (int yp = y; yp > y + ys; yp--) {
				pixel(xp, yp, col);
			}
		}
	}

	if (xs > 0 && ys > 0) {
		for (int xp = x; xp < x + xs; xp++) {
			for (int yp = y; yp < y + ys; yp++) {
				pixel(xp, yp, col);
			}
		}
	}
	
}

void outline(int x, int y, int xs, int ys, uint32_t col) {
	for (int xp = x; xp < x + xs; xp++) {
		pixel(xp, y, col);
		pixel(xp, y + ys, col);
	}
	for (int yp = y; yp < y + ys; yp++) {
		pixel(x, yp, col);
		pixel(x + xs, yp, col);
	}
}

void hLine(int x, int y, int xs, uint32_t col) {
	for (int xp = x; xp < x + xs; xp++) {
		pixel(xp, y, col);
	}
}

uint32_t colc(unsigned char r, unsigned char g, unsigned char b) {
	return (uint32_t)((r << 16) + (g << 8) + b);
}
uint32_t col1(unsigned char v) {
	return (uint32_t)((v << 16) + (v << 8) + v);
}
uint32_t colf(float r, float g, float b) {

}

void pixel(int x, int y, uint32_t col) {
	pixels[x + y * winW] = col;
}

void redraw(float* cVols, float inVol) {
	//background();

	float totalVolL = 0.0f; float totalVolR = 0.0f;
	for (int osc = 0; osc < oscAmount; osc++) {
		totalVolL += cVols[osc * 2];
		totalVolR += cVols[osc * 2 + 1];

		volDisp[volDispCurr][osc][0] = ftc(cVols[osc * 2] * 100);
		volDisp[volDispCurr][osc][1] = ftc(cVols[osc * 2 + 1] * 100);
	}

	int spectraHeight = 114 * 2;
	float hMult = oscAmount / (float)winW;

	uint32_t colL = colc(255, 0, 0);
	uint32_t colR = colc(0, 0, 255);

	for (int i = 0; i < 64; i++) {
		for (int h = 0; h < winW; h++) {

			int index = (volDispCurr + i) % 64;

			

			float amtL = ctf(volDisp[index][(int)(h * hMult)][0]);
			float amtR = ctf(volDisp[index][(int)(h * hMult)][1]);
			//Figure out a simplified version of lerpC to test

			box(h, 360 - (int)(i * 5.7f), 1, 6, lerpC(col1(0), colR, 0.5f));
		}
	}
	volDispCurr++; volDispCurr = volDispCurr % 64;

	//L+R volume bars
	box(10, 350, 10, clamp((int)(totalVolL * -100), -349, 0), col1(ftc(clampf(totalVolL * 0.5f, 0.0f, 1.0f))));
	box(20, 350, 10, clamp((int)(totalVolR * -100), -349, 0), col1(ftc(clampf(totalVolR * 0.5f, 0.0f, 1.0f))));

	float bSize = kbSize; const char* bType = kbString;
	if (kbSize > 1000.0f) { bSize = mbSize; bType = mbString; }

	int line = 15;
	char sCurrStep[40]; sprintf_s(sCurrStep, sizeof(sCurrStep), "Current Step: %d", getStep());
	print(sCurrStep, 4, line++);

	char sBufferSize[40]; sprintf_s(sBufferSize, sizeof(sBufferSize), "Buffer size: %d, %.1f secs (%.1f%s)", bufferLength, bufferLength * cpsInv, bSize, bType);
	print(sBufferSize, 4, line++);

	char sPlayRate[40]; sprintf_s(sPlayRate, sizeof(sPlayRate), "Playback rate: %.1fms, %d steps/second", 1000.0f * cpsInv, CPS);
	print(sPlayRate, 4, line++);

	char sOscs[40]; sprintf_s(sOscs, sizeof(sOscs), "Oscillators: %d (Res: %d)", oscAmount, res);
	print(sOscs, 4, line++);

	char sVolume[40]; sprintf_s(sVolume, sizeof(sVolume), "Volume: %.2f", inVol);
	print(sVolume, 4, 20);

	outline(40, 332, 300, 17, col1(255));
	box(42, 334, (int)(inVol * 296), 14, col1(255));
}

void print(char* text, int x, int y) {

	unsigned char c = *(text++);
	int xOff = 0;

	while (c != '\0') {

		//Gets correct index of the character in the font array
		static int cChar;
		for (int ch = 0; ch < fontChars; ch++) {
			if (c == font1[ch * 4 + 3]) { cChar = ch; break; }
		}

		//Character rendering
		for (int b = 0; b < 3; b++) {
			for (int bit = 0; bit < 8; bit++) {
				//If the bit is 0, skip this bit
				if ((font1[cChar * 4 + b] & (0b10000000 >> bit)) == 0) { continue; }

				//Draw the pixel
				box(((x + xOff) * 5 + (bit % 4)) * pixelScale, (y * 8 + (b * 2 + (bit >= 4))) * pixelScale, pixelScale, pixelScale, col1(255));
			}
		}
		xOff++; c = *(text++); //Increment x offset and current char value
	}
}

uint32_t lerpC(uint32_t c1, uint32_t c2, float t) {
	unsigned char c1r = c1 & 0x00ff0000 >> 16;
	unsigned char c1g = c1 & 0x0000ff00 >> 8;
	unsigned char c1b = c1 & 0x000000ff;
	unsigned char c2r = c2 & 0x00ff0000 >> 16;
	unsigned char c2g = c2 & 0x0000ff00 >> 8;
	unsigned char c2b = c2 & 0x000000ff;

	float r = lerp(ctf(c1r), ctf(c2r), t);
	float g = lerp(ctf(c1g), ctf(c2g), t);
	float b = lerp(ctf(c1b), ctf(c2b), t);

	return colc(ftc(r), ftc(g), ftc(b));
}