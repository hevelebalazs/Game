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
	Int32 point_n;
};

struct Rect
{
	Real32 left;
	Real32 right;
	Real32 top;
	Real32 bottom;
};

struct IntRect
{
	Int32 left;
	Int32 right;
	Int32 top;
	Int32 bottom;
};

static IntRect
func GetIntRectIntersection(IntRect rect1, IntRect rect2)
{
	IntRect result = {};
	result.left   = IntMax2(rect1.left, rect2.left);
	result.right  = IntMin2(rect1.right, rect2.right);
	result.top    = IntMax2(rect1.top, rect2.top);
	result.bottom = IntMin2(rect1.bottom, rect2.bottom);
	return result;
}

static Bool32
func IsPointInRect(Vec2 point, Rect rect)
{
	Bool32 result = IsPointInRectLRTB(point, rect.left, rect.right, rect.top, rect.bottom);
	return result;
}

static Bool32
func IsPointInIntRect(IntVec2 point, IntRect rect)
{
	Bool32 is_row_in = IsIntBetween(point.row, rect.top, rect.bottom);
	Bool32 is_col_in = IsIntBetween(point.col, rect.left, rect.right);
	return (is_row_in && is_col_in);
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
func MakeRect(Vec2 center, Real32 x_size, Real32 y_size)
{
	Rect rect = {};
	rect.left   = center.x - x_size * 0.5f;
	rect.right  = center.x + x_size * 0.5f;
	rect.top    = center.y - y_size * 0.5f;
	rect.bottom = center.y + y_size * 0.5f;
	return rect;
}

static Rect
func MakeRectBottom(Vec2 bottom_center, Real32 x_size, Real32 y_size)
{
	Rect rect = {};
	rect.left	= bottom_center.x - x_size * 0.5f;
	rect.right  = bottom_center.x + x_size * 0.5f;
	rect.top	= bottom_center.y - y_size;
	rect.bottom = bottom_center.y;
	return rect;
}

static Rect
func MakeRectTopLeft(Vec2 top_left, Real32 x_size, Real32 y_size)
{
	Rect rect = {};
	rect.left   = top_left.x;
	rect.right  = rect.left + x_size;
	rect.top    = top_left.y;
	rect.bottom = rect.top + y_size;
	return rect;
}

static Rect
func GetExtendedRect(Rect rect, Real32 extend_side)
{
	Rect extended = {};
	extended.left   = rect.left   - extend_side;
	extended.right  = rect.right  + extend_side;
	extended.top    = rect.top    - extend_side;
	extended.bottom = rect.bottom + extend_side;
	return extended;
}

Bool32
func RectContainsPoint(Rect rect, Vec2 point)
{
	Bool32 contains_x = IsBetween(point.x, rect.left, rect.right);
	Bool32 contains_y = IsBetween(point.y, rect.top, rect.bottom);
	Bool32 contains = (contains_x && contains_y);
	return contains;
}

struct Quad 
{
	Vec2 points[4];
};

static void
func Poly16Add(Poly16 *poly, Vec2 point)
{
	Assert(poly->point_n < 16);
	poly->points[poly->point_n] = point;
	poly->point_n++;
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
	Real32 distance_square = Square(point1.x - point2.x) + Square(point1.y - point2.y);
	return distance_square;
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
func LineAngle(Vec2 start_point, Vec2 end_point)
{
	Vec2 diff = (end_point - start_point);
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
func AngleDifference(Real32 left_angle, Real32 right_angle)
{
	Assert(IsBetween(left_angle,  -PI, +PI));
	Assert(IsBetween(right_angle, -PI, +PI));

	Real32 result = 0.0f;
	if(left_angle < right_angle)
	{
		result = right_angle - left_angle;
	}
	else
	{
		result = (PI - left_angle) + (right_angle - (-PI));
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
func IsAngleBetween(Real32 min_angle, Real32 angle, Real32 max_angle)
{
	Assert(IsBetween(min_angle, -PI, +PI));
	Assert(IsBetween(angle,     -PI, +PI));
	Assert(IsBetween(max_angle, -PI, +PI));
	Bool32 result = false;
	if(min_angle > max_angle)
	{
		result = (angle <= max_angle || angle >= min_angle);
	}
	else
	{
		result = (min_angle <= angle && angle <= max_angle);
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
func PointDirection(Vec2 start_point, Vec2 end_point)
{
	Vec2 vector = MakeVector(end_point.x - start_point.x, end_point.y - start_point.y);
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

	Bool32 should_turn_right = TurnsRight(points[0], points[1], points[2]);

	if(TurnsRight(point, points[0], points[1]) != should_turn_right)
	{
		return false;
	}
	else if(TurnsRight(point, points[1], points[2]) != should_turn_right)
	{
		return false;
	}
	else if(TurnsRight(point, points[2], points[3]) != should_turn_right)
	{
		return false;
	}
	else if(TurnsRight(point, points[3], points[0]) != should_turn_right)
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
func XYToBase(Vec2 point, Vec2 base_unit)
{
	Real32 cosa = base_unit.x;
	Real32 sina = base_unit.y;

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
	Real32 det1   = Determinant(line11.x, line11.y, line12.x, line12.y);
	Real32 det_x1 = Determinant(line11.x,     1.0f, line12.x,     1.0f);
	Real32 det_y1 = Determinant(line11.y,     1.0f, line12.y,     1.0f);

	Real32 det2   = Determinant(line21.x, line21.y, line22.x, line22.y);
	Real32 det_x2 = Determinant(line21.x,     1.0f, line22.x,     1.0f);
	Real32 det_y2 = Determinant(line21.y,     1.0f, line22.y,     1.0f);

	Real32 det_x_up = Determinant(det1, det_x1, det2, det_x2);
	Real32 det_y_up = Determinant(det1, det_y1, det2, det_y2);

	det_x1 = Determinant(line11.x, 1.0f, line12.x, 1.0f);
	det_y1 = Determinant(line11.y, 1.0f, line12.y, 1.0f);
	det_x2 = Determinant(line21.x, 1.0f, line22.x, 1.0f);
	det_y2 = Determinant(line21.y, 1.0f, line22.y, 1.0f);

	Real32 det_down = Determinant(det_x1, det_y1, det_x2, det_y2);

	Assert(det_down != 0.0f);

	Vec2 result = (1.0f / det_down) * MakePoint(det_x_up, det_y_up);
	return result;
}

static Vec2
func LineIntersection(Line line1, Line line2)
{
	Vec2 intersection = LineIntersection(line1.p1, line1.p2, line2.p1, line2.p2);
	return intersection;
}

static Bool32
func IsPointInPoly(Vec2 point, Vec2 *points, Int32 point_n)
{
	Bool32 is_inside = false;
	if(point_n >= 3) 
	{
		is_inside = true;
		Int32 prev = point_n - 1;
		for(Int32 i = 0; i < point_n; i++) 
		{
			if(!TurnsRight(points[prev], points[i], point)) 
			{
				is_inside = false;
				break;
			}
			prev = i;
		}
	}
	return is_inside;
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
	Vec2 unit_base = NormalVector(base);
	Vec2 result = DotProduct(vector, unit_base) * unit_base;
	return result;
}