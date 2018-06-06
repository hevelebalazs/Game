#include <stdlib.h>
#include <time.h>

#include "Debug.hpp"
#include "Math.hpp"

extern float PI = 3.14159265358979323f;

float Min2(float x, float y) {
	if (x < y)
		return x;
	else
		return y;
}

float Min4(float x, float y, float z, float w)
{
	float min = x;
	if (y < min)
		min = y;
	if (z < min)
		min = z;
	if (w < min)
		min = w;
	return min;
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

float Max4(float x, float y, float z, float w)
{
	float max = x;
	if (y > max)
		max = y;
	if (z > max)
		max = z;
	if (w > max)
		max = w;
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

int ClipInt(int value, int min, int max)
{
	Assert(min < max);
	int result = value;
	if (result < min)
		result = min;
	if (result > max)
		result = max;
	return result;
}

float Invert(float value)
{
	Assert(value != 0.0f);
	float inverseValue = 1.0f / value;
	return inverseValue;
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

bool IsIntBetween(int test, int min, int max)
{
	bool isBetween = (min <= test && test <= max);
	return isBetween;
}

bool IsPointInRect(Point point, float left, float right, float top, float bottom) {
	if (point.x < left || point.x > right)
		return false;
	if (point.y < top || point.y > bottom)
		return false;

	return true;
}