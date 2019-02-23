#pragma once

#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "Debug.hpp"
#include "Type.hpp"

#define PI	3.14159265358979323f
#define TAU (2.0f * PI)

struct V2 
{
	F32 x;
	F32 y;
};

struct V4 
{
	union 
	{
		struct 
		{
			V2 position;
			V2 direction;
		};
		struct 
		{
			F32 red;
			F32 green;
			F32 blue;
			F32 alpha;
		};
	};
};

static void func IntSwap(I32* i, I32* j)
{
	I32 tmp = *i;
	*i = *j;
	*j = tmp;
}

static I32 func IntAbs(I32 i)
{
	if (i > 0)
	{
		return i;
	}
	else
	{
		return -i;
	}
}

static I32 func IntRandom(I32 min, I32 max)
{
	I32 result = min + (rand() % (max - min + 1));
	return result;
}

static I32 func IntMin2(I32 i1, I32 i2)
{
	if (i1 < i2) 
	{
		return i1;
	}
	else 
	{
		return i2;
	}
}

static I32 func IntMax2(I32 i1, I32 i2)
{
	if (i1 > i2)
	{
		return i1;
	}
	else
	{
		return i2;
	}
}

static F32 func Sqrt(F32 x)
{
	return sqrtf(x);
}

static F32 func Abs(F32 x)
{
	if (x > 0.0f)
	{
		return x;
	}
	else
	{
		return -x;
	}
}

static I32 func Floor(F32 x)
{
	return (I32)floorf(x);
}

static I32 func RandMod(I32 mod)
{
	return (rand() % mod);
}

static void func SeedRandom(I32 seed)
{
	srand((U32)seed);
}

static V2 func operator+(V2 point1, V2 point2)
{
	V2 result = {};
	result.x = (point1.x + point2.x);
	result.y = (point1.y + point2.y);
	return result;
}

static V2 func operator-(V2 point1, V2 point2)
{
	V2 result = {};
	result.x = (point1.x - point2.x);
	result.y = (point1.y - point2.y);
	return result;
}

static V2 func operator*(F32 times, V2 point)
{
	V2 result = {};
	result.x = (times * point.x);
	result.y = (times * point.y);
	return result;
}

static B32 func operator==(V2 point1, V2 point2)
{
	return ((point1.x == point2.x) && (point1.y == point2.y));
}

static B32 func operator!=(V2 point1, V2 point2)
{
	return ((point1.x != point2.x) || (point1.y != point2.y));
}

static F32 func Min2(F32 x, F32 y)
{
	if (x < y)
	{
		return x;
	}
	else
	{
		return y;
	}
}

static F32 func Min4(F32 x, F32 y, F32 z, F32 w)
{
	F32 min = x;
	if (y < min)
	{
		min = y;
	}
	if (z < min)
	{
		min = z;
	}
	if (w < min)
	{
		min = w;
	}
	return min;
}

static F32 func Max2(F32 x, F32 y)
{
	if (x > y)
	{
		return x;
	}
	else
	{
		return y;
	}
}

static F32 func Max3(F32 x, F32 y, F32 z) 
{
	F32 max = x;
	if (y > max) 
	{
		max = y;
	}
	if (z > max)
	{
		max = z;
	}
	return max;
}

static F32 func Max4(F32 x, F32 y, F32 z, F32 w)
{
	F32 max = x;
	if (y > max)
	{
		max = y;
	}
	if (z > max)
	{
		max = z;
	}
	if (w > max)
	{
		max = w;
	}
	return max;
}

static F32 func Clip(F32 value, F32 min, F32 max)
{
	Assert(min < max);
	F32 result = value;
	if (result < min)
	{
		result = min;
	}
	if (result > max)
	{
		result = max;
	}
	return result;
}

static I32 func ClipInt(I32 value, I32 min, I32 max)
{
	Assert(min < max);
	I32 result = value;
	if (result < min)
	{
		result = min;
	}
	if (result > max)
	{
		result = max;
	}
	return result;
}

static F32 func Invert(F32 value)
{
	Assert(value != 0.0f);
	F32 inverseValue = 1.0f / value;
	return inverseValue;
}

static F32 func Square(F32 x)
{
	F32 result = x * x;
	return result;
}

static I32 func IntSquare(I32 x)
{
	I32 result = x * x;
	return result;
}

static void func InitRandom()
{
	srand((U32)time(0));
}

static F32 func RandomBetween(F32 min, F32 max)
{
	return (min) + (max - min) * ((F32)rand() / (F32)RAND_MAX);
}

static B32 func IsBetween(F32 test, F32 value1, F32 value2)
{
	F32 min = Min2(value1, value2);
	F32 max = Max2(value1, value2);
	return (min <= test && test <= max);
}

static F32 func Lerp(F32 value1, F32 ratio, F32 value2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	return ((1.0f - ratio) * value1) + ((ratio) * value2);
}

static V2 func PointLerp(V2 point1, F32 ratio, V2 point2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	V2 result = {};
	result.x = Lerp(point1.x, ratio, point2.x);
	result.y = Lerp(point1.y, ratio, point2.y);
	return result;
}

static B32 func IsIntBetween(I32 test, I32 min, I32 max)
{
	B32 isBetween = (min <= test && test <= max);
	return isBetween;
}

static B32 func IsPointInRect(V2 point, F32 left, F32 right, F32 top, F32 bottom)
{
	if (point.x < left || point.x > right)
	{
		return false;
	}
	if (point.y < top || point.y > bottom)
	{
		return false;
	}

	return true;
}

static B32 func IntSign(I32 i)
{
	I32 sign = 0;
	if (i < 0)
	{
		sign = -1;
	}
	else if (i > 0)
	{
		sign = +1;
	}
	return sign;
}