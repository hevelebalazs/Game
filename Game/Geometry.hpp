#pragma once

#include "Debug.hpp"
#include "Math.hpp"
#include "Type.hpp"

union Line 
{
	struct
	{
		Vec2 p1;
		Vec2 p2;
	};

	struct 
	{
		Real32 x1, y1;
		Real32 x2, y2;
	};
};

Line
func MakeLine(Vec2 point1, Vec2 point2)
{
	Line line = {};
	line.p1 = point1;
	line.p2 = point2;
	return line;
}

Line
func MakeLineXYXY(Real32 x1, Real32 y1, Real32 x2, Real32 y2)
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
	Vec2 points[16];
	Int32 pointN;
};

struct Rect
{
	Real32 left;
	Real32 right;
	Real32 top;
	Real32 bottom;
};

static Bool32
func IsPointInRect(Vec2 point, Rect rect)
{
	Bool32 result = IsPointInRectLRTB(point, rect.left, rect.right, rect.top, rect.bottom);
	return result;
}

static Rect
func MakeSquareRect(Vec2 center, Real32 size)
{
	Rect rect = {};
	rect.left   = center.x - size * 0.5f;
	rect.right  = center.x + size * 0.5f;
	rect.top    = center.y - size * 0.5f;
	rect.bottom = center.y + size * 0.5f;
	return rect;
}

static Rect
func MakeRect(Vec2 center, Real32 xSize, Real32 ySize)
{
	Rect rect = {};
	rect.left   = center.x - xSize * 0.5f;
	rect.right  = center.x + xSize * 0.5f;
	rect.top    = center.y - ySize * 0.5f;
	rect.bottom = center.y + ySize * 0.5f;
	return rect;
}

static Rect
func MakeRectBottom(Vec2 bottomCenter, Real32 xSize, Real32 ySize)
{
	Rect rect = {};
	rect.left	= bottomCenter.x - xSize * 0.5f;
	rect.right  = bottomCenter.x + xSize * 0.5f;
	rect.top	= bottomCenter.y - ySize;
	rect.bottom = bottomCenter.y;
	return rect;
}

static Rect
func MakeRectTopLeft(Vec2 topLeft, Real32 xSize, Real32 ySize)
{
	Rect rect = {};
	rect.left   = topLeft.x;
	rect.right  = rect.left + xSize;
	rect.top    = topLeft.y;
	rect.bottom = rect.top + ySize;
	return rect;
}

Bool32
func RectContainsPoint(Rect rect, Vec2 point)
{
	Bool32 containsX = IsBetween(point.x, rect.left, rect.right);
	Bool32 containsY = IsBetween(point.y, rect.top, rect.bottom);
	Bool32 contains = (containsX && containsY);
	return contains;
}

struct Quad 
{
	Vec2 points[4];
};

static void
func Poly16Add(Poly16 *poly, Vec2 point)
{
	Assert(poly->pointN < 16);
	poly->points[poly->pointN] = point;
	poly->pointN++;
}

static Vec2
func MakePoint(Real32 x, Real32 y)
{
	Vec2 point = {};
	point.x = x;
	point.y = y;
	return point;
}

static IntVec2
func MakeIntPoint(Int32 row, Int32 col)
{
	IntVec2 point = {};
	point.row = row;
	point.col = col;
	return point;
}

static Vec2
func MakeVector(Real32 x, Real32 y)
{
	Vec2 result = MakePoint(x, y);
	return result;
}

static Quad
func MakeQuad(Vec2 point1, Vec2 point2, Vec2 point3, Vec2 point4)
{
	Quad quad = {};
	quad.points[0] = point1;
	quad.points[1] = point2;
	quad.points[2] = point3;
	quad.points[3] = point4;
	return quad;
}

static Real32
func DistanceSquare(Vec2 point1, Vec2 point2)
{
	Real32 distanceSquare = Square(point1.x - point2.x) + Square(point1.y - point2.y);
	return distanceSquare;
}

static Real32
func CityDistance(Vec2 point1, Vec2 point2)
{
	Real32 distance = Abs(point1.x - point2.x) + Abs(point1.y - point2.y);
	return distance;
}

static Real32
func MaxDistance(Vec2 point1, Vec2 point2)
{
	Real32 distance = Max2(Abs(point1.x - point2.x), Abs(point1.y - point2.y));
	return distance;
}

static Real32
func Distance(Vec2 point1, Vec2 point2)
{
	Real32 dx = (point1.x - point2.x);
	Real32 dy = (point1.y - point2.y);

	Real32 distance = sqrtf((dx * dx) + (dy * dy));
	return distance;
}

static Real32
func VectorLength(Vec2 vector)
{
	Real32 length = sqrtf((vector.x * vector.x) + (vector.y * vector.y));
	return length;
}

static Real32
func VectorAngle (Vec2 vector)
{
	Real32 angle = atan2f(vector.y, vector.x);
	return angle;
}

static Real32
func LineAngle(Vec2 startPoint, Vec2 endPoint)
{
	Vec2 diff = (endPoint - startPoint);
	Real32 angle = VectorAngle(diff);
	return angle;
}

static Real32
func NormalizeAngle(Real32 angle)
{
	while(angle > PI)
	{
		angle -= 2.0f * PI;
	}

	while(angle < -PI)
	{
		angle += 2.0f * PI;
	}

	return angle;
}

static Real32
func AngleDifference(Real32 leftAngle, Real32 rightAngle)
{
	Assert(IsBetween(leftAngle,  -PI, +PI));
	Assert(IsBetween(rightAngle, -PI, +PI));

	Real32 result = 0.0f;
	if(leftAngle < rightAngle)
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

static Bool32
func TurnsRight(Vec2 point1, Vec2 point2, Vec2 point3)
{
	Real32 dx1 = point2.x - point1.x;
	Real32 dy1 = point2.y - point1.y;
	Real32 dx2 = point3.x - point2.x;
	Real32 dy2 = point3.y - point2.y;

	Real32 det = (dx1 * dy2) - (dx2 * dy1);

	Bool32 result = (det > 0.0f);
	return result;
}

static Bool32
func IsAngleBetween(Real32 minAngle, Real32 angle, Real32 maxAngle)
{
	Assert(IsBetween(minAngle, -PI, +PI));
	Assert(IsBetween(angle,    -PI, +PI));
	Assert(IsBetween(maxAngle, -PI, +PI));
	Bool32 result = false;
	if(minAngle > maxAngle)
	{
		result = (angle <= maxAngle || angle >= minAngle);
	}
	else
	{
		result = (minAngle <= angle && angle <= maxAngle);
	}
	return result;
}

static Vec2
func RotationVector(Real32 angle)
{
	Vec2 result = MakePoint(cosf(angle), sinf(angle));
	return result;
}

static Vec2
func NormalVector(Vec2 vector)
{
	Real32 length = VectorLength(vector);

	Vec2 result = {};
	if(length == 0.0f) 
	{
		result = vector;
	}
	else
	{
		result = (1.0f / length) * vector;
	}
	return result;
}

static Vec2
func PointDirection(Vec2 startPoint, Vec2 endPoint)
{
	Vec2 vector = MakeVector(endPoint.x - startPoint.x, endPoint.y - startPoint.y);
	Vec2 normal = NormalVector(vector);

	return normal;
}

static Bool32
func IsPointInQuad(Quad quad, Vec2 point)
{
	Vec2 *points = quad.points;
	if(point.x < points[0].x && point.x < points[1].x && point.x < points[2].x && point.x < points[3].x)
	{
		return false;
	}
	if(point.x > points[0].x && point.x > points[1].x && point.x > points[2].x && point.x > points[3].x)
	{
		return false;
	}

	if(point.y < points[0].y && point.y < points[1].y && point.y < points[2].y && point.y < points[3].y)
	{
		return false;
	}
	if(point.y > points[0].y && point.y > points[1].y && point.y > points[2].y && point.y > points[3].y)
	{
		return false;
	}

	Bool32 shouldTurnRight = TurnsRight(points[0], points[1], points[2]);

	if(TurnsRight(point, points[0], points[1]) != shouldTurnRight)
	{
		return false;
	}
	else if(TurnsRight(point, points[1], points[2]) != shouldTurnRight)
	{
		return false;
	}
	else if(TurnsRight(point, points[2], points[3]) != shouldTurnRight)
	{
		return false;
	}
	else if(TurnsRight(point, points[3], points[0]) != shouldTurnRight)
	{
		return false;
	}
	else
	{
		return true;
	}
}

static Vec2
func TurnVectorToRight(Vec2 vector)
{
	Vec2 result = {};
	result.x = -vector.y;
	result.y = vector.x;
	return result;
}

static Vec2
func XYToBase(Vec2 point, Vec2 baseUnit)
{
	Real32 cosa = baseUnit.x;
	Real32 sina = baseUnit.y;

	Vec2 result = {};
	result.x = point.x * cosa + point.y * sina;
	result.y = -point.x * sina + point.y * cosa;

	return result;
}

// TODO: can this be merged with LineIntersection?
static Bool32
func DoLinesCross(Vec2 line11, Vec2 line12, Vec2 line21, Vec2 line22)
{
	Bool32 right1 = TurnsRight(line11, line21, line22);
	Bool32 right2 = TurnsRight(line12, line21, line22);
	if(right1 == right2) 
	{
		return false;
	}

	Bool32 right3 = TurnsRight(line21, line11, line12);
	Bool32 right4 = TurnsRight(line22, line11, line12);
	if(right3 == right4) 
	{
		return false;
	}

	return true;
}

static Real32
func Determinant(Real32 a, Real32 b, Real32 c, Real32 d)
{
	Real32 result = (a * d) - (b * c);
	return result;
}

static Vec2
func LineIntersection(Vec2 line11, Vec2 line12, Vec2 line21, Vec2 line22)
{
	Real32 det1  = Determinant(line11.x, line11.y, line12.x, line12.y);
	Real32 detX1 = Determinant(line11.x,     1.0f, line12.x,     1.0f);
	Real32 detY1 = Determinant(line11.y,     1.0f, line12.y,     1.0f);

	Real32 det2  = Determinant(line21.x, line21.y, line22.x, line22.y);
	Real32 detX2 = Determinant(line21.x,     1.0f, line22.x,     1.0f);
	Real32 detY2 = Determinant(line21.y,     1.0f, line22.y,     1.0f);

	Real32 detXUp = Determinant(det1, detX1, det2, detX2);
	Real32 detYUp = Determinant(det1, detY1, det2, detY2);

	detX1 = Determinant(line11.x, 1.0f, line12.x, 1.0f);
	detY1 = Determinant(line11.y, 1.0f, line12.y, 1.0f);
	detX2 = Determinant(line21.x, 1.0f, line22.x, 1.0f);
	detY2 = Determinant(line21.y, 1.0f, line22.y, 1.0f);

	Real32 detDown = Determinant(detX1, detY1, detX2, detY2);

	Assert(detDown != 0.0f);

	Vec2 result = (1.0f / detDown) * MakePoint(detXUp, detYUp);
	return result;
}

static Vec2
func LineIntersection(Line line1, Line line2)
{
	Vec2 intersection = LineIntersection(line1.p1, line1.p2, line2.p1, line2.p2);
	return intersection;
}

static Bool32
func IsPointInPoly(Vec2 point, Vec2 *points, Int32 pointN)
{
	Bool32 isInside = false;
	if(pointN >= 3) 
	{
		isInside = true;
		Int32 prev = pointN - 1;
		for(Int32 i = 0; i < pointN; i++) 
		{
			if(!TurnsRight(points[prev], points[i], point)) 
			{
				isInside = false;
				break;
			}
			prev = i;
		}
	}
	return isInside;
}

static Real32
func DotProduct(Vec2 vector1, Vec2 vector2)
{
	Real32 result = ((vector1.x * vector2.x) + (vector1.y * vector2.y));
	return result;
}

static Vec2
func ParallelVector(Vec2 vector, Vec2 base)
{
	Vec2 unitBase = NormalVector(base);
	Vec2 result = DotProduct(vector, unitBase) * unitBase;
	return result;
}