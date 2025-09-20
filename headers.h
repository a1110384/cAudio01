#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#define SAMPLE_RATE 44100
#define BPM 140
#define SPM (SAMPLE_RATE * 60)
#define CHUNK_SIZE (SPM / BPM / 8)
#define CHUNK_AMT 2
const static int halfChunk = CHUNK_SIZE / 2;
const static int CPS = (SAMPLE_RATE / CHUNK_SIZE);
float cpsInv;

#define res 16
#define resF (float)res
#define midiTotal 114
const static short oscAmount = res * midiTotal;
#define OAMacro res * midiTotal
double oscMult;
short oscs2;
float oscInv;

#define activeOscs 32
float activeMult;

static int cStep = 0;
static int gStep = 0;

#define bufferLength 512
#define noisesAmt 10
const static float byteMult = 1.0f / 256.0f;

unsigned char buffer[bufferLength][OAMacro][2];
unsigned char nBuffer[bufferLength][noisesAmt][2];

const static float kbSize = bufferLength * OAMacro * 2 / 1000.0f;
const static float mbSize = bufferLength * OAMacro * 2 / 1000000.0f;
const static char* kbString = "KB"; const static char* mbString = "MB";

static float mVol = 0.2f;
static short playing = 1;

//Renderer.c
WAVEFORMATEX setFormat();
void startRenderer(HWAVEOUT waveOut);
void renderSamples(float inVol);
void writeLoop(HWAVEOUT waveOut);

//Composer.c
void generate();
void setFV(int offStep, int freq, float val);
float* getVols();

//UT.c
float clampf(float v, float lo, float hi);
int clamp(int v, int lo, int hi);

float mtf(float m);
float ftm(float f);
float harmonic(float n, int har);
int k2m(float note7, int* key);

int rani(int max);
int rani(int min, int max);
float ranf();
float ranfIn(float min, float max);

float lerp(float a, float b, float t);
float envADSR(float t, float l, float a, float d, float s, float r, float c);

//UI.c
void initUI();
void setCursor(int x, int y);
void writef(const char* text, float f, int x, int y);
void write(const char* text, int x, int y);
void writes(const char* text, const char* add, int x, int y);
void printVolume(float v);
void setBox(int x, int y, int width, int height);
void debugC(int gst);