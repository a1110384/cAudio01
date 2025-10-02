#include "headers.h"

HWAVEOUT wave_out;
void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);

const int frameMs = 1000 / 60;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {

	//Window init
	bmi.bmiHeader.biWidth = winW;
	bmi.bmiHeader.biHeight = -winH; //Negative sets origin to top left
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	hdc = CreateCompatibleDC(0);

	setupWindow(hInstance);

	if (frame_bitmap) DeleteObject(frame_bitmap);
	frame_bitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pixels, 0, 0);
	SelectObject(hdc, frame_bitmap);


	//Waveout init
	WAVEFORMATEX format = setFormat();
	waveOutOpen(&wave_out, WAVE_MAPPER, &format, (DWORD_PTR)waveOutProc, (DWORD_PTR)NULL, CALLBACK_FUNCTION);
	startRenderer(wave_out);

	while (!quit) {
		memset(keyPressed, 0, 256 * sizeof(keyPressed[0])); //Resets keyPressed
		static MSG message = { 0 }; while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&message); } //Process all incoming messages

		if (keyPressed[VK_LEFT]) {
			mVol = clampf(mVol - 0.05f, 0.0f, 1.0f);
		}
		if (keyPressed[VK_RIGHT]) {
			mVol = clampf(mVol + 0.05f, 0.0f, 1.0f);
		}
		if (keyPressed['W']) { }
		if (keyPressed['S']) {  }
		if (keyPressed[VK_ESCAPE]) { quit = true; }

		

		Sleep(frameMs);
		InvalidateRect(hwnd, NULL, FALSE); UpdateWindow(hwnd);

		//Do framerate sleeping here (WIP)
		
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
		PostQuitMessage(0);
	} break;

	case WM_KILLFOCUS: winFocus = false; memset(keyDown, 0, 256 * sizeof(keyDown[0])); break;
	case WM_SETFOCUS: winFocus = true; break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (!winFocus) { break; }
		static bool keyTran, keyHold;
		keyTran = ((lParam & (1 << 31)) == 0);
		keyHold = ((lParam & (1 << 30)) != 0);

		if (keyTran != keyHold) {
			keyPressed[(uint8_t)wParam] = keyTran;
			keyDown[(uint8_t)wParam] = keyTran;
		}
		break;


	case WM_PAINT: {
		static PAINTSTRUCT paint;
		static HDC device_context;
		device_context = BeginPaint(hwnd, &paint);
		BitBlt(device_context,
			0, 0,
			winW, winH,
			hdc,
			paint.rcPaint.left, paint.rcPaint.top,
			SRCCOPY);
		EndPaint(hwnd, &paint);
	} break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}