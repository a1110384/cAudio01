#include "headers.h"

static int fAh[] = { 76, 85, 100 };
static int fEe[] = { 62, 94, 101 };
static int fEh[] = { 67, 92, 99 };
static int fOh[] = { 67, 79, 99 };
static int fOo[] = { 65, 74, 100 };

int getFormant(int vowel, int formant) {
	switch (vowel) {
	case 0: return fAh[formant] * res;
	case 1: return fEe[formant] * res;
	case 2: return fEh[formant] * res;
	case 3: return fOh[formant] * res;
	case 4: return fOo[formant] * res;
	}
	return 0;
}