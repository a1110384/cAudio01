#include "headers.h"

#define ftc(f) (int)(f * 255.0f)

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

	BITMAPINFO fbmi;
	fbmi.bmiHeader.biWidth = 64; fbmi.bmiHeader.biHeight = 38;
	fbmi.bmiHeader.biSize = sizeof(fbmi.bmiHeader);
	fbmi.bmiHeader.biPlanes = 1;
	fbmi.bmiHeader.biBitCount = 1;
	fbmi.bmiHeader.biCompression = BI_RGB;

	HBITMAP tempMap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
	tempMap = CreateDIBSection(NULL, &fbmi, DIB_RGB_COLORS, (void**)&font1, 0, 0);
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

void pixel(int x, int y, uint32_t col) {
	pixels[x + y * winW] = col;
}

void redraw(float* cVols) {
	background();

	

	float totalVolL = 0.0f; float totalVolR = 0.0f;
	for (int osc = 0; osc < oscAmount; osc++) {
		totalVolL += cVols[osc * 2];
		totalVolR += cVols[osc * 2 + 1];

		volDisp[volDispCurr][osc][0] = ftc(cVols[osc * 2] * 100);
		volDisp[volDispCurr][osc][1] = ftc(cVols[osc * 2 + 1] * 100);
	}

	int spectraHeight = 114 * 2;
	int hMult = oscAmount / spectraHeight;

	for (int i = 0; i < 64; i++) {
		for (int h = 0; h < spectraHeight; h++) {

			int index = (volDispCurr + i) % 64;

			char colL = volDisp[index][h * hMult][0];
			char colR = volDisp[index][h * hMult][1];

			hLine(300 + i * 4, 370 - h, 4, colc(colR, 0, colL));
		}
	}
	volDispCurr++; volDispCurr = volDispCurr % 64;

	//L+R volume bars
	box(10, 350, 10, clamp((int)(totalVolL * -100), -349, 0), col1(ftc(clampf(totalVolL * 0.5f, 0.0f, 1.0f))));
	box(20, 350, 10, clamp((int)(totalVolR * -100), -349, 0), col1(ftc(clampf(totalVolR * 0.5f, 0.0f, 1.0f))));
}

void character(char ch, int x, int y) {
	int bx, by;
	switch (ch) {

	}
}