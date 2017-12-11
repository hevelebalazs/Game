#include <stdlib.h>

#include "Math.h"

float PI = 3.14159265358979323f;

float Min2(float x, float y) {
	if (x < y) return x;
	else return y;
}

float Max2(float x, float y) {
	if (x > y) return x;
	else return y;
}

float RandomBetween(float min, float max) {
	return (min) + (max - min) * ((float)rand() / (float)RAND_MAX);
}

bool IsBetween(float test, float min, float max) {
	return (min <= test && test <= max);
}

bool IsPointInRect(Point point, float left, float right, float top, float bottom) {
	if (point.x < left || point.x > right) return false;
	if (point.y < top || point.y > bottom) return false;

	return true;
}