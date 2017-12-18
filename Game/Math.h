#include "Point.h"

extern float PI;

inline void IntSwap(int* i, int* j) {
	int tmp = *i;
	*i = *j;
	*j = tmp;
}

inline int IntAbs(int i) {
	if (i > 0) return i;
	else return -i;
}

float Min2(float x, float y);
float Max2(float x, float y);

float RandomBetween(float min, float max);
bool IsBetween(float test, float min, float max);

bool IsPointInRect(Point point, float left, float right, float top, float bottom);