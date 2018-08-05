#pragma once

#include <math.h>
#include <stdlib.h>

#include "Type.hpp"

#define PI	3.14159265358979323f

struct V2 {
	F32 x;
	F32 y;
};

struct V4 {
	union {
		struct {
			V2 position;
			V2 direction;
		};
		struct {
			F32 red;
			F32 green;
			F32 blue;
			F32 alpha;
		};
	};
};

V2 operator+(V2 point1, V2 point2);
V2 operator-(V2 point1, V2 point2);
V2 operator*(F32 times, V2 point);
B32 operator==(V2 point1, V2 point2);
B32 operator!=(V2 point1, V2 point2);

inline void IntSwap(I32* i, I32* j)
{
	I32 tmp = *i;
	*i = *j;
	*j = tmp;
}

inline I32 IntAbs(I32 i)
{
	if (i > 0)
		return i;
	else
		return -i;
}

inline I32 IntRandom(I32 min, I32 max)
{
	I32 result = min + (rand() % (max - min + 1));
	return result;
}

inline I32 IntMin2(I32 i1, I32 i2)
{
	if (i1 < i2) 
		return i1;
	else 
		return i2;
}

inline I32 IntMax2(I32 i1, I32 i2)
{
	if (i1 > i2)
		return i1;
	else
		return i2;
}

// TODO: these should be inline functions
F32 Min2(F32 x, F32 y);
F32 Min4(F32 x, F32 y, F32 z, F32 w);
F32 Max2(F32 x, F32 y);
F32 Max3(F32 x, F32 y, F32 z);
F32 Max4(F32 x, F32 y, F32 z, F32 w);
F32 Clip(F32 value, F32 min, F32 max);
I32 ClipInt(I32 value, I32 min, I32 max);
F32 Invert(F32 value);

inline F32 Sqrt(F32 x)
{
	return sqrtf(x);
}

F32 Square(F32 x);

inline F32 Abs(F32 x)
{
	if (x > 0.0f)
		return x;
	else
		return -x;
}

inline I32 Floor(F32 x)
{
	return (I32)floorf(x);
}

inline F32 Lerp(F32 value1, F32 ratio, F32 value2)
{
	return ((1.0f - ratio) * value1) + ((ratio) * value2);
}

inline I32 RandMod(I32 mod)
{
	return (rand() % mod);
}

inline void SeedRandom(I32 seed)
{
	srand((U32)seed);
}

void InitRandom();
F32 RandomBetween(F32 min, F32 max);
B32 IsBetween(F32 test, F32 min, F32 max);
B32 IsIntBetween(I32 test, I32 min, I32 max);

B32 IsPointInRect(V2 point, F32 left, F32 right, F32 top, F32 bottom);