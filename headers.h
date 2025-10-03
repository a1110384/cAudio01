#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <dwmapi.h>

#include "resource.h"

//#define UNICODE
//#define _UNICODE

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

//Windows vars
static const wchar_t winClassName[] = L"winmmAudio - a111";
static HWND hwnd;
static BITMAPINFO bmi;
static HBITMAP frame_bitmap = 0;
static HDC hdc = 0;

#define fontW 64
#define fontH 38
#define fontChars 96
#define pixelScale 2
uint32_t colText;

LRESULT CALLBACK winMsgProc(HWND, UINT, WPARAM, LPARAM);

static bool quit = false;

#define winW 600
#define winH 400
uint32_t* pixels;
static float winWInv, winHInv;

static bool keyDown[256] = { 0 };
static bool keyPressed[256] = { 0 };


#define SHORT_MIN -32768
#define SHORT_MAX 32767

#define SAMPLE_RATE 44100
#define BPM 140
#define SPM (SAMPLE_RATE * 60)
#define CHUNK_SIZE 2048
#define CHUNK_AMT 4
const static int halfChunk = CHUNK_SIZE / 2;
const static int CPS = (SAMPLE_RATE / CHUNK_SIZE);
float cpsInv;

#define res 8
#define resF (float)res
#define midiTotal 114
const static short oscAmount = res * midiTotal;
#define OAMacro res * midiTotal
static double oscMult;
static short oscs2;
static float oscInv;

#define activeOscs 32

static int cStep = 0;
static int gStep = 0;

#define sineLength 1024
short sineWave[sineLength];

#define bufferLength 512
#define noisesAmt 10
#define noiseLength 8192
const static float nSpeeds[] = { 4.0f, 3.0f, 2.1f, 1.5f, 1.0f, 0.50f, 0.30f, 0.15f, 0.08f, 0.02f };
const static float byteMult = 1.0f / 256.0f;
const static float noiseMult = 1.0f / 7.0f;
const static float activeMult = 1.0f / 6.0f;

static unsigned char buffer[bufferLength][OAMacro][2];
static unsigned char nBuffer[bufferLength][noisesAmt][2];

const static float kbSize = bufferLength * OAMacro * 2 / 1000.0f;
const static float mbSize = bufferLength * OAMacro * 2 / 1000000.0f;
const static char* kbString = "KB"; const static char* mbString = "MB";

static float mVol = 0.2f;
static short playing = 1;

static int wavSize;






//Renderer.c
WAVEFORMATEX setFormat();
void startRenderer(HWAVEOUT waveOut);
void renderSamples(float inVol);
void writeLoop(HWAVEOUT waveOut);

//Composer.c
void generate();
void setFV(int offStep, int freq, float val, int channel);
void nSetFV(int offStep, int freq, float val, int channel);
float* getVols();
float* getNoises();
int getStep();

//Synthesizer.c
int getFormant(int vowel, int formant);

//UT.c
float clampf(float v, float lo, float hi);
int clamp(int v, int lo, int hi);

float mtf(float m);
float ftm(float f);
float harmonic(float n, int har);
int k2m(float note7, int key[]);

int rani(int min, int max);
float ranf();
float ranfIn(float min, float max);

float lerp(float a, float b, float t);
unsigned char lerpByte(unsigned char a, unsigned char b, float t);
float envADSR(float t, float l, float a, float d, float s, float r, float c);
float envEq(float t, float center, float distance);
float osc(float t, float amt);


//UI.c
void setupWindow(HINSTANCE hInstance);
void pixel(int x, int y, uint32_t col);
void box(int x, int y, int xs, int ys, uint32_t col);
void redraw(float* cVols, float inVol);
void print(char* text, int x, int y);
uint32_t lerpC(uint32_t c1, uint32_t c2, float t);
uint32_t lerpC2D(uint32_t c1, uint32_t c2, uint32_t none, uint32_t both, float t1, float t2);
uint32_t colc(unsigned char r, unsigned char g, unsigned char b);