#include <stdlib.h>
#include <time.h>

#include "Debug.hpp"
#include "Math.hpp"
#include "Type.hpp"

V2 operator+(V2 point1, V2 point2)
{
	V2 result = {};
	result.x = (point1.x + point2.x);
	result.y = (point1.y + point2.y);
	return result;
}

V2 operator-(V2 point1, V2 point2)
{
	V2 result = {};
	result.x = (point1.x - point2.x);
	result.y = (point1.y - point2.y);
	return result;
}

V2 operator*(F32 times, V2 point)
{
	V2 result = {};
	result.x = (times * point.x);
	result.y = (times * point.y);
	return result;
}

B32 operator==(V2 point1, V2 point2)
{
	return ((point1.x == point2.x) && (point1.y == point2.y));
}

B32 operator!=(V2 point1, V2 point2)
{
	return ((point1.x != point2.x) || (point1.y != point2.y));
}

F32 Min2(F32 x, F32 y)
{
	if (x < y)
		return x;
	else
		return y;
}

F32 Min4(F32 x, F32 y, F32 z, F32 w)
{
	F32 min = x;
	if (y < min)
		min = y;
	if (z < min)
		min = z;
	if (w < min)
		min = w;
	return min;
}

F32 Max2(F32 x, F32 y)
{
	if (x > y)
		return x;
	else
		return y;
}

F32 Max3(F32 x, F32 y, F32 z) 
{
	F32 max = x;
	if (y > max) 
		max = y;
	if (z > max)
		max = z;
	return max;
}

F32 Max4(F32 x, F32 y, F32 z, F32 w)
{
	F32 max = x;
	if (y > max)
		max = y;
	if (z > max)
		max = z;
	if (w > max)
		max = w;
	return max;
}

F32 Clip(F32 value, F32 min, F32 max)
{
	Assert(min < max);
	F32 result = value;
	if (result < min)
		result = min;
	if (result > max)
		result = max;
	return result;
}

I32 ClipInt(I32 value, I32 min, I32 max)
{
	Assert(min < max);
	I32 result = value;
	if (result < min)
		result = min;
	if (result > max)
		result = max;
	return result;
}

F32 Invert(F32 value)
{
	Assert(value != 0.0f);
	F32 inverseValue = 1.0f / value;
	return inverseValue;
}

void InitRandom()
{
	srand((U32)time(0));
}

F32 RandomBetween(F32 min, F32 max)
{
	return (min) + (max - min) * ((F32)rand() / (F32)RAND_MAX);
}

B32 IsBetween(F32 test, F32 min, F32 max)
{
	return (min <= test && test <= max);
}

B32 IsIntBetween(I32 test, I32 min, I32 max)
{
	B32 isBetween = (min <= test && test <= max);
	return isBetween;
}

B32 IsPointInRect(V2 point, F32 left, F32 right, F32 top, F32 bottom)
{
	if (point.x < left || point.x > right)
		return false;
	if (point.y < top || point.y > bottom)
		return false;

	return true;
}