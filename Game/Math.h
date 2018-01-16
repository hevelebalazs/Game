#pragma once

#include <math.h>
#include <stdlib.h>

#include "Point.h"

extern float PI;

inline void IntSwap(int* i, int* j) {
	int tmp = *i;
	*i = *j;
	*j = tmp;
}

inline int IntAbs(int i) {
	if (i > 0)
		return i;
	else
		return -i;
}

inline int IntRandom(int min, int max) {
	int result = min + (rand() % (max - min + 1));
	return result;
}

// TODO: these should be inline functions
float Min2(float x, float y);
float Max2(float x, float y);

inline float Sqrt(float x) {
	return sqrtf(x);
}

inline float Abs(float x) {
	if (x > 0)
		return x;
	else
		return -x;
}

inline int RandMod(int mod) {
	return (rand() % mod);
}

inline void SeedRandom(int seed) {
	srand((unsigned int)seed);
}

void InitRandom();
float RandomBetween(float min, float max);
bool IsBetween(float test, float min, float max);

bool IsPointInRect(Point point, float left, float right, float top, float bottom);