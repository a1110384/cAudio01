#include "headers.h"

float clampf(float v, float lo, float hi) {
	if (v > hi) { return hi; }
	if (v < lo) { return lo; }
	return v;
}
int clamp(int v, int lo, int hi) {
	if (v > hi) { return hi; }
	if (v < lo) { return lo; }
	return v;
}

float mtf(float m) { return 440 * powf(2, (m - 69) / 12.0f); }
float ftm(float f) { return 12 * (logf(f / 220) / logf(2.0f)) + 57; }
float harmonic(float n, int har) { return ftm(mtf(n) * (har + 1)); }
int k2m(float note7, int* key) {
	float oct = note7 / 7.0f;
	int note = (int)note7 - ((int)oct * 7.0f);
	return min(((int)oct * 12 + key[note]), oscAmount - 1);
}

int rani(int min, int max) { return (rand() % (max - min)) + min; }
float ranf() { return rand() / (float)RAND_MAX; }
float ranfIn(float min, float max) { return rand() / (float)RAND_MAX * (max - min) + min; }
int raniOf(int arr[]) {
	return arr[rani(0, sizeof(arr) / 4)];
}

float lerp(float a, float b, float t) { return a + t * (b - a); }
float envADSR(float t, float l, float a, float d, float s, float r, float c) {
	if (t < a) { return powf(t / a, c); } //Attack
	if (t > a && t < a + d) { return powf(1.0f - (((t - a) / d) * (1.0f - s)), c); } //Decay
	if (t > l) { return powf(s - (t - l) / r, 1.5f); } //Release
	if (t > l + r || t < 0.0f) { return 0.0f; } //Before/After
	return s; //Sustain
}

