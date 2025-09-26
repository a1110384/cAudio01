#include "headers.h"
#include "resource.h"

static HWND hwnd;
static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap = 0;
static HDC frame_device_context = 0;

HWAVEOUT wave_out;
void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);
LRESULT CALLBACK winMsgProc(HWND, UINT, WPARAM, LPARAM);


static bool quit = false;

int winW = 600;
int winH = 400;
uint32_t* pixels;

bool keyDown[256] = { 0 };
bool keyPressed[256] = { 0 };

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {

	const wchar_t winClassName[] = L"winmmAudio";
	static WNDCLASSEX wcx = { 0 };
	wcx.cbSize = sizeof(wcx);
	wcx.lpfnWndProc = winMsgProc;
	wcx.hInstance = hInstance;
	wcx.lpszClassName = winClassName;
	wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	RegisterClassEx(&wcx);

	frame_bitmap_info.bmiHeader.biWidth = winW;
	frame_bitmap_info.bmiHeader.biHeight = winH;
	frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
	frame_bitmap_info.bmiHeader.biPlanes = 1;
	frame_bitmap_info.bmiHeader.biBitCount = 32;
	frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
	frame_device_context = CreateCompatibleDC(0);

	hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, winClassName,
		winClassName,
		WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_CAPTION,
		640, 300,
		winW, winH,
		NULL, NULL, hInstance, NULL);
	if (hwnd == NULL) { return -1; }

	if (frame_bitmap) DeleteObject(frame_bitmap);
	frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info, DIB_RGB_COLORS, (void**)&pixels, 0, 0);
	SelectObject(frame_device_context, frame_bitmap);
	
	WAVEFORMATEX format = setFormat();
	waveOutOpen(&wave_out, WAVE_MAPPER, &format, (DWORD_PTR)waveOutProc, (DWORD_PTR)NULL, CALLBACK_FUNCTION);
	startRenderer(wave_out);

	
	//initUI(); printVolume(mVol);

	//TODO: MAKE AN INPUT ARRAY FOR KEYDOWN AND KEYPRESSED
	/*
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
	*/

	while (!quit) {
		static MSG message = { 0 };
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&message); }

		static unsigned int p = 0;


		if (keyPressed['W']) {
			for (int c = 0; c < winW * winH; c++) { pixels[c] = rand(); }
		}

		InvalidateRect(hwnd, NULL, FALSE);
		UpdateWindow(hwnd);
	}

	return 0;
}

void CALLBACK waveOutProc(HWAVEOUT wave_out_handle, UINT message, DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2) {
	if (message == WOM_DONE) { renderSamples(mVol); writeLoop(wave_out); } //Render and write the buffers
}


LRESULT CALLBACK winMsgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static bool winFocus = true;

	switch (message) {
	case WM_QUIT:
	case WM_DESTROY: {
		quit = true;
	} break;

	case WM_KILLFOCUS: winFocus = false; memset(keyDown, 0, 256 * sizeof(keyDown[0])); break;
	case WM_SETFOCUS: winFocus = true; break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (!winFocus) { break; }


		static bool keyIsDown, keyHold;
		keyIsDown = ((lParam & (1 << 31)) == 0);
		keyHold = ((lParam & (1 << 30)) != 0);

		if (keyIsDown && !keyDown[(uint8_t)wParam]) {
			keyPressed[(uint8_t)wParam] = true;
		}
		else {
			keyPressed[(uint8_t)wParam] = false;
		}

		if (keyIsDown != keyHold) {
			keyDown[(uint8_t)wParam] = keyIsDown;
		}

		break;


	case WM_PAINT: {
		static PAINTSTRUCT paint;
		static HDC device_context;
		device_context = BeginPaint(hwnd, &paint);
		BitBlt(device_context,
			paint.rcPaint.left, paint.rcPaint.top,
			paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top,
			frame_device_context,
			paint.rcPaint.left, paint.rcPaint.top,
			SRCCOPY);
		EndPaint(hwnd, &paint);
	} break;


	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}