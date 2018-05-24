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

inline int IntMin2(int i1, int i2) {
	if (i1 < i2) return i1;
	else return i2;
}

inline int IntMax2(int i1, int i2) {
	if (i1 > i2) return i1;
	else return i2;
}

// TODO: these should be inline functions
float Min2(float x, float y);
float Max2(float x, float y);
float Max3(float x, float y, float z);

inline float Sqrt(float x) {
	return sqrtf(x);
}

inline float Abs(float x) {
	if (x > 0)
		return x;
	else
		return -x;
}

inline float Floor(float x) {
	return floorf(x);
}

inline float Lerp(float value1, float ratio, float value2) {
	return ((1.0f - ratio) * value1) + ((ratio) * value2);
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