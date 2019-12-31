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
	R32 x;
	R32 y;
};

struct IV2
{
	I32 row;
	I32 col;
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
			R32 red;
			R32 green;
			R32 blue;
			R32 alpha;
		};
	};
};

static void
func IntSwap(I32 *i, I32 *j)
{
	I32 tmp = *i;
	*i = *j;
	*j = tmp;
}

static I32
func IntAbs(I32 i)
{
	I32 result = 0;
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

static B32
func IsIntBetween(I32 test, I32 min, I32 max)
{
	B32 is_between = (min <= test && test <= max);
	return is_between;
}

static I32
func IntRandom(I32 min, I32 max)
{
	I32 result = min + (rand() % (max - min + 1));
	Assert(IsIntBetween(result, min, max));
	return result;
}

static I32
func IntMin2(I32 i1, I32 i2)
{
	I32 min = 0;
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

static I32
func IntMax2(I32 i1, I32 i2)
{
	I32 max = 0;
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

static R32
func Sqrt(R32 x)
{
	return sqrtf(x);
}

static R32
func Abs(R32 x)
{
	R32 result = 0;
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

static I32
func Floor(R32 x)
{
	I32 result = (I32)floorf(x);
	return result;
}

static R32
func Fraction(R32 x)
{
    R32 fraction = x - (I32)x;
	return fraction;
}

static I32
func RandMod(I32 mod)
{
	I32 result = (rand() % mod);
	return result;
}

static void
func SeedRandom(I32 seed)
{
	srand((U32)seed);
}

static V2
func operator+(V2 point1, V2 point2)
{
	V2 result = {};
	result.x = (point1.x + point2.x);
	result.y = (point1.y + point2.y);
	return result;
}

static void
func operator+=(V2& point1, V2 point2)
{
	point1 = point1 + point2;
}

static V2
func operator-(V2 point1, V2 point2)
{
	V2 result = {};
	result.x = (point1.x - point2.x);
	result.y = (point1.y - point2.y);
	return result;
}

static V2
func operator*(R32 times, V2 point)
{
	V2 result = {};
	result.x = (times * point.x);
	result.y = (times * point.y);
	return result;
}

static IV2
func operator+(IV2 point1, IV2 point2)
{
	IV2 result = {};
	result.row = point1.row + point2.row;
	result.col = point1.col + point2.col;
	return result;
}

static B32
func operator==(V2 point1, V2 point2)
{
	B32 result = ((point1.x == point2.x) && (point1.y == point2.y));
	return result;
}

static B32
func operator!=(V2 point1, V2 point2)
{
	B32 result = ((point1.x != point2.x) || (point1.y != point2.y));
	return result;
}

static B32
func operator==(IV2 point1, IV2 point2)
{
	B32 equal = ((point1.row == point2.row) && (point1.col == point2.col));
	return equal;
}

static B32
func operator!=(IV2 point1, IV2 point2)
{
	B32 different = ((point1.row != point2.row) || (point1.col != point2.col));
	return different;
}

static R32
func Min2(R32 x, R32 y)
{
	R32 min = 0.0f;
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

static R32
func Min4(R32 x, R32 y, R32 z, R32 w)
{
	R32 min = x;
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

static R32
func Max2(R32 x, R32 y)
{
	R32 max = 0.0f;
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

static R32
func Max3(R32 x, R32 y, R32 z) 
{
	R32 max = x;
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

static R32
func Max4(R32 x, R32 y, R32 z, R32 w)
{
	R32 max = x;
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

static R32
func Clip(R32 value, R32 min, R32 max)
{
	Assert(min < max);
	R32 result = value;
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

static I32
func ClipInt(I32 value, I32 min, I32 max)
{
	Assert(min < max);
	I32 result = value;
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

static R32
func Invert(R32 value)
{
	Assert(value != 0.0f);
	R32 inverse_value = (1.0f / value);
	return inverse_value;
}

static R32
func Square(R32 x)
{
	R32 result = (x * x);
	return result;
}

static I32
func IntSquare(I32 x)
{
	I32 result = (x * x);
	return result;
}

static void
func InitRandom()
{
	srand ((U32)time(0));
}

static R32
func RandomBetween(R32 min, R32 max)
{
	R32 result = (min) + (max - min) * ((R32)rand() / (R32)RAND_MAX);
	return result;
}

static B32
func IsBetween(R32 test, R32 value1, R32 value2)
{
	R32 min = Min2(value1, value2);
	R32 max = Max2(value1, value2);
	B32 result = (min <= test && test <= max);
	return result;
}

static R32
func Lerp(R32 value1, R32 ratio, R32 value2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	return ((1.0f - ratio) * value1) + ((ratio) * value2);
}

static V2
func PointLerp(V2 point1, R32 ratio, V2 point2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	V2 result = {};
	result.x = Lerp(point1.x, ratio, point2.x);
	result.y = Lerp(point1.y, ratio, point2.y);
	return result;
}

static B32
func IsPointInRectLRTB(V2 point, R32 left, R32 right, R32 top, R32 bottom)
{
	B32 result = true;
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

static B32
func IntSign(I32 i)
{
	I32 sign = 0;
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