#include <stdlib.h>
#include <time.h>

#include "Debug.h"
#include "Math.h"

extern float PI = 3.14159265358979323f;

float Min2(float x, float y) {
	if (x < y)
		return x;
	else
		return y;
}

float Max2(float x, float y) {
	if (x > y)
		return x;
	else
		return y;
}

float Max3(float x, float y, float z) 
{
	float max = x;
	if (y > max) 
		max = y;
	if (z > max)
		max = z;
	return max;
}

float Clip(float value, float min, float max)
{
	Assert(min < max);
	float result = value;
	if (result < min)
		result = min;
	if (result > max)
		result = max;
	return result;
}

void InitRandom() {
	srand((unsigned int)time(0));
}

float RandomBetween(float min, float max) {
	return (min) + (max - min) * ((float)rand() / (float)RAND_MAX);
}

bool IsBetween(float test, float min, float max) {
	return (min <= test && test <= max);
}

bool IsPointInRect(Point point, float left, float right, float top, float bottom) {
	if (point.x < left || point.x > right)
		return false;
	if (point.y < top || point.y > bottom)
		return false;

	return true;
}