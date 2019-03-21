#pragma once

#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "Debug.hpp"
#include "Type.hpp"

#define PI	3.14159265358979323f
#define TAU (2.0f * PI)

struct Vec2 
{
	Real32 x;
	Real32 y;
};

struct IntVec2
{
	Int32 x;
	Int32 y;
};

struct Vec4 
{
	union 
	{
		struct 
		{
			Vec2 position;
			Vec2 direction;
		};
		struct 
		{
			Real32 red;
			Real32 green;
			Real32 blue;
			Real32 alpha;
		};
	};
};

static void func IntSwap(Int32* i, Int32* j)
{
	Int32 tmp = *i;
	*i = *j;
	*j = tmp;
}

static Int32 func IntAbs(Int32 i)
{
	Int32 result = 0;
	if(i > 0)
	{
		result = i;
	}
	else
	{
		result = -i;
	}
	return result;
}

static Bool32 func IsIntBetween(Int32 test, Int32 min, Int32 max)
{
	Bool32 isBetween = (min <= test && test <= max);
	return isBetween;
}

static Int32 func IntRandom(Int32 min, Int32 max)
{
	Int32 result = min + (rand() % (max - min + 1));
	Assert(IsIntBetween(result, min, max));
	return result;
}

static Int32 func IntMin2(Int32 i1, Int32 i2)
{
	Int32 min = 0;
	if(i1 < i2) 
	{
		min = i1;
	}
	else 
	{
		min = i2;
	}
	return min;
}

static Int32 func IntMax2(Int32 i1, Int32 i2)
{
	Int32 max = 0;
	if(i1 > i2)
	{
		max = i1;
	}
	else
	{
		max = i2;
	}
	return max;
}

static Real32 func Sqrt(Real32 x)
{
	return sqrtf(x);
}

static Real32 func Abs(Real32 x)
{
	Real32 result = 0;
	if(x > 0.0f)
	{
		result = x;
	}
	else
	{
		result = -x;
	}
	return result;
}

static Int32 func Floor(Real32 x)
{
	Int32 result = (Int32)floorf(x);
	return result;
}

static Real32 func Fraction (Real32 x)
{
    Real32 fraction = x - (Int32)x;
	return fraction;
}

static Int32 func RandMod(Int32 mod)
{
	Int32 result = (rand() % mod);
	return result;
}

static void func SeedRandom(Int32 seed)
{
	srand((UInt32)seed);
}

static Vec2 func operator+(Vec2 point1, Vec2 point2)
{
	Vec2 result = {};
	result.x = (point1.x + point2.x);
	result.y = (point1.y + point2.y);
	return result;
}

static Vec2 func operator-(Vec2 point1, Vec2 point2)
{
	Vec2 result = {};
	result.x = (point1.x - point2.x);
	result.y = (point1.y - point2.y);
	return result;
}

static Vec2 func operator*(Real32 times, Vec2 point)
{
	Vec2 result = {};
	result.x = (times * point.x);
	result.y = (times * point.y);
	return result;
}

static Bool32 func operator==(Vec2 point1, Vec2 point2)
{
	Bool32 result = ((point1.x == point2.x) && (point1.y == point2.y));
	return result;
}

static Bool32 func operator!=(Vec2 point1, Vec2 point2)
{
	Bool32 result = ((point1.x != point2.x) || (point1.y != point2.y));
	return result;
}

static Real32 func Min2(Real32 x, Real32 y)
{
	Real32 min = 0.0f;
	if(x < y)
	{
		min = x;
	}
	else
	{
		min = y;
	}
	return min;
}

static Real32 func Min4(Real32 x, Real32 y, Real32 z, Real32 w)
{
	Real32 min = x;
	if(y < min)
	{
		min = y;
	}
	if(z < min)
	{
		min = z;
	}
	if(w < min)
	{
		min = w;
	}
	return min;
}

static Real32 func Max2(Real32 x, Real32 y)
{
	Real32 max = 0.0f;
	if(x > y)
	{
		max = x;
	}
	else
	{
		max = y;
	}
	return max;
}

static Real32 func Max3(Real32 x, Real32 y, Real32 z) 
{
	Real32 max = x;
	if(y > max) 
	{
		max = y;
	}
	if(z > max)
	{
		max = z;
	}
	return max;
}

static Real32 func Max4(Real32 x, Real32 y, Real32 z, Real32 w)
{
	Real32 max = x;
	if(y > max)
	{
		max = y;
	}
	if(z > max)
	{
		max = z;
	}
	if(w > max)
	{
		max = w;
	}
	return max;
}

static Real32 func Clip(Real32 value, Real32 min, Real32 max)
{
	Assert(min < max);
	Real32 result = value;
	if(result < min)
	{
		result = min;
	}
	if(result > max)
	{
		result = max;
	}
	return result;
}

static Int32 func ClipInt(Int32 value, Int32 min, Int32 max)
{
	Assert(min < max);
	Int32 result = value;
	if(result < min)
	{
		result = min;
	}
	if(result > max)
	{
		result = max;
	}
	return result;
}

static Real32 func Invert(Real32 value)
{
	Assert(value != 0.0f);
	Real32 inverseValue = (1.0f / value);
	return inverseValue;
}

static Real32 func Square(Real32 x)
{
	Real32 result = (x * x);
	return result;
}

static Int32 func IntSquare(Int32 x)
{
	Int32 result = (x * x);
	return result;
}

static void func InitRandom()
{
	srand ((UInt32)time(0));
}

static Real32 func RandomBetween(Real32 min, Real32 max)
{
	Real32 result = (min) + (max - min) * ((Real32)rand() / (Real32)RAND_MAX);
	return result;
}

static Bool32 func IsBetween(Real32 test, Real32 value1, Real32 value2)
{
	Real32 min = Min2(value1, value2);
	Real32 max = Max2(value1, value2);
	Bool32 result = (min <= test && test <= max);
	return result;
}

static Real32 func Lerp(Real32 value1, Real32 ratio, Real32 value2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	return ((1.0f - ratio) * value1) + ((ratio) * value2);
}

static Vec2 func PointLerp(Vec2 point1, Real32 ratio, Vec2 point2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	Vec2 result = {};
	result.x = Lerp(point1.x, ratio, point2.x);
	result.y = Lerp(point1.y, ratio, point2.y);
	return result;
}

static Bool32 func IsPointInRectLRTB(Vec2 point, Real32 left, Real32 right, Real32 top, Real32 bottom)
{
	Bool32 result = true;
	if(point.x < left || point.x > right)
	{
		result = false;
	}
	if(point.y < top || point.y > bottom)
	{
		result = false;
	}
	return result;
}

static Bool32 func IntSign(Int32 i)
{
	Int32 sign = 0;
	if(i < 0)
	{
		sign = -1;
	}
	else if(i > 0)
	{
		sign = +1;
	}
	return sign;
}