#include "headers.h"

void initUI() {
	SetConsoleOutputCP(65001);
	SetConsoleTitle(TEXT("winmmAudio - a111"));
	//Set text color to cyan
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY | 0);

	//Remove cursor visibility
	CONSOLE_CURSOR_INFO lpCursor;
	lpCursor.bVisible = FALSE;
	lpCursor.dwSize = 100;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &lpCursor);

	//Set window size
	RECT r;
	GetWindowRect(GetConsoleWindow(), &r);
	MoveWindow(GetConsoleWindow(), r.left, r.top, 500, 400, TRUE);

	//Set font
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof cfi;
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = 15;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, sizeof cfi.FaceName, L"Terminal");
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

	WAVEOUTCAPS caps;
	waveOutGetDevCaps((UINT)0, &caps, sizeof(caps));
	setCursor(0, 0);
	printf("OPEN DEVICE: %S", caps.szPname);
	setBox(0, 1, 44, 13);

}
void setCursor(int x, int y) {
	COORD coord;
	coord.X = x; coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void writef(const char* text, float f, int x, int y) {
	setCursor(x, y);
	printf(text, f);
}
void write(const char* text, int x, int y) {
	setCursor(x, y);
	printf(text);
}
void writes(const char* text, const char* add, int x, int y) {
	setCursor(x, y);
	printf(text, add);
}

void printVolume(float v) {
	int x = 1;
	int y = 2;
	int dist = 40;

	for (int i = 0; i < dist; i++)
	{
		if (i < v * dist) { write(u8"█", x + i + 4, y); }
		else { write(" ", x + i + 4, y); }
	}

	//box(x - 1, y - 1, dist + 4, 1);
	for (int i = 0; i < dist + 4; i++)
	{
		write(u8"═", x + i, y + 1);
	}
	write(u8"╠", x - 1, y + 1);
	write(u8"╣", x + dist + 4, y + 1);
	write("Volume", x + 1, y - 1);
	writef("%.2f:", v, x, y);
}

void setBox(int x, int y, int width, int height) {

	write(u8"╔", x, y);
	write(u8"╚", x, y + height + 1);
	write(u8"╗", x + width + 1, y);
	write(u8"╝", x + width + 1, y + height + 1);

	for (int i = 0; i < width; i++)
	{
		write(u8"═", x + i + 1, y);
		write(u8"═", x + i + 1, y + height + 1);
	}

	for (int i = 0; i < height; i++) {
		write(u8"║", x, y + i + 1);
		write(u8"║", x + width + 1, y + i + 1);
	}
}

void debugC(int gst) {
	
	float bSize = kbSize; const char* bType = kbString;
	if (kbSize > 1000.0f) { bSize = mbSize; bType = mbString; }

	setCursor(1, 4); printf("Current Step: %d          ", gst);
	setCursor(1, 5); printf("Buffer size: %d steps, %.1f secs (%.1f%s) ", bufferLength, bufferLength * cpsInv, bSize, bType);
	setCursor(1, 6); printf("Playback rate: %.1fms, %d steps/second    ", 1000.0f * cpsInv, CPS);
	setCursor(1, 7); printf("Oscillators: %d (Res: %d)", oscAmount, res);
	setCursor(1, 9); printf("time: %d", time(NULL));
	
}

void debugR(int wavSize) {
	setCursor(1, 11); printf("time: %d", wavSize);
}