#pragma once

#include "Debug.hpp"
#include "Math.hpp"
#include "Type.hpp"

union Line 
{
	struct
	{
		V2 p1;
		V2 p2;
	};

	struct 
	{
		F32 x1, y1;
		F32 x2, y2;
	};
};

Line MakeLine(V2 point1, V2 point2)
{
	Line line = {};
	line.p1 = point1;
	line.p2 = point2;
	return line;
}

Line MakeLineXYXY(F32 x1, F32 y1, F32 x2, F32 y2)
{
	Line line = {};
	line.x1 = x1;
	line.y1 = y1;
	line.x2 = x2;
	line.y2 = y2;
	return line;
}

struct Poly16 
{
	V2 points[16];
	I32 pointN;
};

struct Rect
{
	F32 left;
	F32 right;
	F32 top;
	F32 bottom;
};

Rect func MakeSquareRect(V2 center, F32 size)
{
	Rect rect = {};
	rect.left   = center.x - size * 0.5f;
	rect.right  = center.x + size * 0.5f;
	rect.top    = center.y - size * 0.5f;
	rect.bottom = center.y + size * 0.5f;
	return rect;
}

struct Quad 
{
	V2 points[4];
};

// TODO: create a type that always represents a unit vector?

static void func Poly16Add(Poly16* poly, V2 point)
{
	Assert(poly->pointN < 16);
	poly->points[poly->pointN] = point;
	poly->pointN++;
}

static V2 func MakePoint(F32 x, F32 y)
{
	V2 point = {};
	point.x = x;
	point.y = y;
	return point;
}

static V2 func MakeVector(F32 x, F32 y)
{
	V2 result = MakePoint(x, y);
	return result;
}

static Quad func MakeQuad(V2 point1, V2 point2, V2 point3, V2 point4)
{
	Quad quad = {};
	quad.points[0] = point1;
	quad.points[1] = point2;
	quad.points[2] = point3;
	quad.points[3] = point4;
	return quad;
}

static F32 func DistanceSquare(V2 point1, V2 point2)
{
	return (point1.x - point2.x) * (point1.x - point2.x) +
		(point1.y - point2.y) * (point1.y - point2.y);
}

static F32 func CityDistance(V2 point1, V2 point2)
{
	return fabsf(point1.x - point2.x) + fabsf(point1.y - point2.y);
}

static F32 func Distance(V2 point1, V2 point2)
{
	F32 dx = (point1.x - point2.x);
	F32 dy = (point1.y - point2.y);

	return sqrtf((dx * dx) + (dy * dy));
}

static F32 func VectorLength(V2 vector)
{
	return sqrtf((vector.x * vector.x) + (vector.y * vector.y));
}

static F32 func VectorAngle(V2 vector)
{
	return atan2f(vector.y, vector.x);
}

static F32 func LineAngle(V2 startPoint, V2 endPoint)
{
	V2 diff = (endPoint - startPoint);
	F32 angle = VectorAngle(diff);
	return angle;
}

static F32 func NormalizeAngle(F32 angle)
{
	while (angle > PI)
	{
		angle -= 2.0f * PI;
	}

	while (angle < -PI)
	{
		angle += 2.0f * PI;
	}

	return angle;
}

static F32 func AngleDifference(F32 leftAngle, F32 rightAngle)
{
	Assert(IsBetween(leftAngle,  -PI, +PI));
	Assert(IsBetween(rightAngle, -PI, +PI));

	F32 result = 0.0f;
	if (leftAngle < rightAngle)
	{
		result = rightAngle - leftAngle;
	}
	else
	{
		result = (PI - leftAngle) + (rightAngle - (-PI));
	}
	Assert(IsBetween(result, 0.0f, TAU));
	return result;
}

static B32 func TurnsRight(V2 point1, V2 point2, V2 point3)
{
	F32 dx1 = point2.x - point1.x;
	F32 dy1 = point2.y - point1.y;
	F32 dx2 = point3.x - point2.x;
	F32 dy2 = point3.y - point2.y;

	F32 det = (dx1 * dy2) - (dx2 * dy1);

	return (det > 0.0f);
}

static B32 func IsAngleBetween(F32 minAngle, F32 angle, F32 maxAngle)
{
	Assert(IsBetween(minAngle, -PI, +PI));
	Assert(IsBetween(angle,    -PI, +PI));
	Assert(IsBetween(maxAngle, -PI, +PI));
	B32 result = false;
	if (minAngle > maxAngle)
	{
		result = (angle <= maxAngle || angle >= minAngle);
	}
	else
	{
		result = (minAngle <= angle && angle <= maxAngle);
	}
	return result;
}

static V2 func RotationVector(F32 angle)
{
	V2 result = MakePoint(cosf(angle), sinf(angle));
	return result;
}

static V2 func NormalVector(V2 vector)
{
	F32 length = VectorLength(vector);
	if (length == 0.0f) 
	{
		return vector;
	}

	V2 result = (1.0f / length) * vector;
	return result;
}

static V2 func PointDirection(V2 startPoint, V2 endPoint)
{
	V2 vector = {endPoint.x - startPoint.x, endPoint.y - startPoint.y};
	vector = NormalVector(vector);

	return vector;
}

static B32 func IsPointInQuad(Quad quad, V2 point)
{
	V2* points = quad.points;
	if (point.x < points[0].x && point.x < points[1].x && point.x < points[2].x && point.x < points[3].x)
	{
		return false;
	}
	if (point.x > points[0].x && point.x > points[1].x && point.x > points[2].x && point.x > points[3].x)
	{
		return false;
	}

	if (point.y < points[0].y && point.y < points[1].y && point.y < points[2].y && point.y < points[3].y)
	{
		return false;
	}
	if (point.y > points[0].y && point.y > points[1].y && point.y > points[2].y && point.y > points[3].y)
	{
		return false;
	}

	B32 shouldTurnRight = TurnsRight(points[0], points[1], points[2]);

	if (TurnsRight(point, points[0], points[1]) != shouldTurnRight)
	{
		return false;
	}
	else if (TurnsRight(point, points[1], points[2]) != shouldTurnRight)
	{
		return false;
	}
	else if (TurnsRight(point, points[2], points[3]) != shouldTurnRight)
	{
		return false;
	}
	else if (TurnsRight(point, points[3], points[0]) != shouldTurnRight)
	{
		return false;
	}
	else
	{
		return true;
	}
}

static V2 func TurnVectorToRight(V2 vector)
{
	V2 result = {};
	result.x = -vector.y;
	result.y = vector.x;
	return result;
}

static V2 func XYToBase(V2 point, V2 baseUnit)
{
	F32 cosa = baseUnit.x;
	F32 sina = baseUnit.y;

	V2 result = {};
	result.x = point.x * cosa + point.y * sina;
	result.y = -point.x * sina + point.y * cosa;

	return result;
}

// TODO: can this be merged with LineIntersection?
static B32 func DoLinesCross(V2 line11, V2 line12, V2 line21, V2 line22)
{
	B32 right1 = TurnsRight(line11, line21, line22);
	B32 right2 = TurnsRight(line12, line21, line22);
	if (right1 == right2) 
	{
		return false;
	}

	B32 right3 = TurnsRight(line21, line11, line12);
	B32 right4 = TurnsRight(line22, line11, line12);
	if (right3 == right4) 
	{
		return false;
	}

	return true;
}

static F32 func Determinant(F32 a, F32 b, F32 c, F32 d)
{
	return (a * d) - (b * c);
}

static V2 func LineIntersection(V2 line11, V2 line12, V2 line21, V2 line22)
{
	F32 det1  = Determinant(line11.x, line11.y, line12.x, line12.y);
	F32 detX1 = Determinant(line11.x,     1.0f, line12.x,     1.0f);
	F32 detY1 = Determinant(line11.y,     1.0f, line12.y,     1.0f);

	F32 det2  = Determinant(line21.x, line21.y, line22.x, line22.y);
	F32 detX2 = Determinant(line21.x,     1.0f, line22.x,     1.0f);
	F32 detY2 = Determinant(line21.y,     1.0f, line22.y,     1.0f);

	F32 detXUp = Determinant(det1, detX1, det2, detX2);
	F32 detYUp = Determinant(det1, detY1, det2, detY2);

	detX1 = Determinant(line11.x, 1.0f, line12.x, 1.0f);
	detY1 = Determinant(line11.y, 1.0f, line12.y, 1.0f);
	detX2 = Determinant(line21.x, 1.0f, line22.x, 1.0f);
	detY2 = Determinant(line21.y, 1.0f, line22.y, 1.0f);

	F32 detDown = Determinant(detX1, detY1, detX2, detY2);

	Assert(detDown != 0.0f);

	V2 result = (1.0f / detDown) * MakePoint(detXUp, detYUp);
	return result;
}

static V2 func LineIntersection(Line line1, Line line2)
{
	V2 intersection = LineIntersection(line1.p1, line1.p2, line2.p1, line2.p2);
	return intersection;
}

static B32 func IsPointInPoly(V2 point, V2* points, I32 pointN)
{
	B32 isInside = false;
	if (pointN >= 3) 
	{
		isInside = true;
		I32 prev = pointN - 1;
		for (I32 i = 0; i < pointN; ++i) 
		{
			if (!TurnsRight(points[prev], points[i], point)) 
			{
				isInside = false;
				break;
			}
			prev = i;
		}
	}
	return isInside;
}

static F32 func DotProduct(V2 vector1, V2 vector2)
{
	return ((vector1.x * vector2.x) + (vector1.y * vector2.y));
}

// TODO: create a version of this where base is unit length?
static V2 func ParallelVector(V2 vector, V2 base)
{
	V2 unitBase = NormalVector(base);
	V2 result = DotProduct(vector, unitBase) * unitBase;
	return result;
}