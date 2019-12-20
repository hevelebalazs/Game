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
		R32 x1, y1;
		R32 x2, y2;
	};
};

Line
func MakeLine(V2 point1, V2 point2)
{
	Line line = {};
	line.p1 = point1;
	line.p2 = point2;
	return line;
}

Line
func MakeLineXYXY(R32 x1, R32 y1, R32 x2, R32 y2)
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
	I32 point_n;
};

struct Rect
{
	R32 left;
	R32 right;
	R32 top;
	R32 bottom;
};

struct IntRect
{
	I32 left;
	I32 right;
	I32 top;
	I32 bottom;
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

static B32
func IsPointInRect(V2 point, Rect rect)
{
	B32 result = IsPointInRectLRTB(point, rect.left, rect.right, rect.top, rect.bottom);
	return result;
}

static B32
func IsPointInIntRect(IV2 point, IntRect rect)
{
	B32 is_row_in = IsIntBetween(point.row, rect.top, rect.bottom);
	B32 is_col_in = IsIntBetween(point.col, rect.left, rect.right);
	return (is_row_in && is_col_in);
}

static Rect
func MakeSquareRect(V2 center, R32 size)
{
	Rect rect = {};
	rect.left   = center.x - size * 0.5f;
	rect.right  = center.x + size * 0.5f;
	rect.top    = center.y - size * 0.5f;
	rect.bottom = center.y + size * 0.5f;
	return rect;
}

static Rect
func MakeRect(V2 center, R32 x_size, R32 y_size)
{
	Rect rect = {};
	rect.left   = center.x - x_size * 0.5f;
	rect.right  = center.x + x_size * 0.5f;
	rect.top    = center.y - y_size * 0.5f;
	rect.bottom = center.y + y_size * 0.5f;
	return rect;
}

static Rect
func MakeRectBottom(V2 bottom_center, R32 x_size, R32 y_size)
{
	Rect rect = {};
	rect.left	= bottom_center.x - x_size * 0.5f;
	rect.right  = bottom_center.x + x_size * 0.5f;
	rect.top	= bottom_center.y - y_size;
	rect.bottom = bottom_center.y;
	return rect;
}

static Rect
func MakeRectTopLeft(V2 top_left, R32 x_size, R32 y_size)
{
	Rect rect = {};
	rect.left   = top_left.x;
	rect.right  = rect.left + x_size;
	rect.top    = top_left.y;
	rect.bottom = rect.top + y_size;
	return rect;
}

static Rect
func GetExtendedRect(Rect rect, R32 extend_side)
{
	Rect extended = {};
	extended.left   = rect.left   - extend_side;
	extended.right  = rect.right  + extend_side;
	extended.top    = rect.top    - extend_side;
	extended.bottom = rect.bottom + extend_side;
	return extended;
}

B32
func RectContainsPoint(Rect rect, V2 point)
{
	B32 contains_x = IsBetween(point.x, rect.left, rect.right);
	B32 contains_y = IsBetween(point.y, rect.top, rect.bottom);
	B32 contains = (contains_x && contains_y);
	return contains;
}

struct Quad 
{
	V2 points[4];
};

static void
func Poly16Add(Poly16 *poly, V2 point)
{
	Assert(poly->point_n < 16);
	poly->points[poly->point_n] = point;
	poly->point_n++;
}

static V2
func MakePoint(R32 x, R32 y)
{
	V2 point = {};
	point.x = x;
	point.y = y;
	return point;
}

static IV2
func MakeIntPoint(I32 row, I32 col)
{
	IV2 point = {};
	point.row = row;
	point.col = col;
	return point;
}

static V2
func MakeVector(R32 x, R32 y)
{
	V2 result = MakePoint(x, y);
	return result;
}

static Quad
func MakeQuad(V2 point1, V2 point2, V2 point3, V2 point4)
{
	Quad quad = {};
	quad.points[0] = point1;
	quad.points[1] = point2;
	quad.points[2] = point3;
	quad.points[3] = point4;
	return quad;
}

static R32
func DistanceSquare(V2 point1, V2 point2)
{
	R32 distance_square = Square(point1.x - point2.x) + Square(point1.y - point2.y);
	return distance_square;
}

static R32
func CityDistance(V2 point1, V2 point2)
{
	R32 distance = Abs(point1.x - point2.x) + Abs(point1.y - point2.y);
	return distance;
}

static R32
func MaxDistance(V2 point1, V2 point2)
{
	R32 distance = Max2(Abs(point1.x - point2.x), Abs(point1.y - point2.y));
	return distance;
}

static R32
func Distance(V2 point1, V2 point2)
{
	R32 dx = (point1.x - point2.x);
	R32 dy = (point1.y - point2.y);

	R32 distance = sqrtf((dx * dx) + (dy * dy));
	return distance;
}

static R32
func VectorLength(V2 vector)
{
	R32 length = sqrtf((vector.x * vector.x) + (vector.y * vector.y));
	return length;
}

static R32
func VectorAngle (V2 vector)
{
	R32 angle = atan2f(vector.y, vector.x);
	return angle;
}

static R32
func LineAngle(V2 start_point, V2 end_point)
{
	V2 diff = (end_point - start_point);
	R32 angle = VectorAngle(diff);
	return angle;
}

static R32
func NormalizeAngle(R32 angle)
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

static R32
func AngleDifference(R32 left_angle, R32 right_angle)
{
	Assert(IsBetween(left_angle,  -PI, +PI));
	Assert(IsBetween(right_angle, -PI, +PI));

	R32 result = 0.0f;
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

static B32
func TurnsRight(V2 point1, V2 point2, V2 point3)
{
	R32 dx1 = point2.x - point1.x;
	R32 dy1 = point2.y - point1.y;
	R32 dx2 = point3.x - point2.x;
	R32 dy2 = point3.y - point2.y;

	R32 det = (dx1 * dy2) - (dx2 * dy1);

	B32 result = (det > 0.0f);
	return result;
}

static B32
func IsAngleBetween(R32 min_angle, R32 angle, R32 max_angle)
{
	Assert(IsBetween(min_angle, -PI, +PI));
	Assert(IsBetween(angle,     -PI, +PI));
	Assert(IsBetween(max_angle, -PI, +PI));
	B32 result = false;
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

static V2
func RotationVector(R32 angle)
{
	V2 result = MakePoint(cosf(angle), sinf(angle));
	return result;
}

static V2
func NormalVector(V2 vector)
{
	R32 length = VectorLength(vector);

	V2 result = {};
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

static V2
func PointDirection(V2 start_point, V2 end_point)
{
	V2 vector = MakeVector(end_point.x - start_point.x, end_point.y - start_point.y);
	V2 normal = NormalVector(vector);

	return normal;
}

static B32
func IsPointInQuad(Quad quad, V2 point)
{
	V2 *points = quad.points;
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

	B32 should_turn_right = TurnsRight(points[0], points[1], points[2]);

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

static V2
func TurnVectorToRight(V2 vector)
{
	V2 result = {};
	result.x = -vector.y;
	result.y = vector.x;
	return result;
}

static V2
func XYToBase(V2 point, V2 base_unit)
{
	R32 cosa = base_unit.x;
	R32 sina = base_unit.y;

	V2 result = {};
	result.x = point.x * cosa + point.y * sina;
	result.y = -point.x * sina + point.y * cosa;

	return result;
}

// TODO: can this be merged with LineIntersection?
static B32
func DoLinesCross(V2 line11, V2 line12, V2 line21, V2 line22)
{
	B32 right1 = TurnsRight(line11, line21, line22);
	B32 right2 = TurnsRight(line12, line21, line22);
	if(right1 == right2) 
	{
		return false;
	}

	B32 right3 = TurnsRight(line21, line11, line12);
	B32 right4 = TurnsRight(line22, line11, line12);
	if(right3 == right4) 
	{
		return false;
	}

	return true;
}

static R32
func Determinant(R32 a, R32 b, R32 c, R32 d)
{
	R32 result = (a * d) - (b * c);
	return result;
}

static V2
func LineIntersection(V2 line11, V2 line12, V2 line21, V2 line22)
{
	R32 det1   = Determinant(line11.x, line11.y, line12.x, line12.y);
	R32 det_x1 = Determinant(line11.x,     1.0f, line12.x,     1.0f);
	R32 det_y1 = Determinant(line11.y,     1.0f, line12.y,     1.0f);

	R32 det2   = Determinant(line21.x, line21.y, line22.x, line22.y);
	R32 det_x2 = Determinant(line21.x,     1.0f, line22.x,     1.0f);
	R32 det_y2 = Determinant(line21.y,     1.0f, line22.y,     1.0f);

	R32 det_x_up = Determinant(det1, det_x1, det2, det_x2);
	R32 det_y_up = Determinant(det1, det_y1, det2, det_y2);

	det_x1 = Determinant(line11.x, 1.0f, line12.x, 1.0f);
	det_y1 = Determinant(line11.y, 1.0f, line12.y, 1.0f);
	det_x2 = Determinant(line21.x, 1.0f, line22.x, 1.0f);
	det_y2 = Determinant(line21.y, 1.0f, line22.y, 1.0f);

	R32 det_down = Determinant(det_x1, det_y1, det_x2, det_y2);

	Assert(det_down != 0.0f);

	V2 result = (1.0f / det_down) * MakePoint(det_x_up, det_y_up);
	return result;
}

static V2
func LineIntersection(Line line1, Line line2)
{
	V2 intersection = LineIntersection(line1.p1, line1.p2, line2.p1, line2.p2);
	return intersection;
}

static B32
func IsPointInPoly(V2 point, V2 *points, I32 point_n)
{
	B32 is_inside = false;
	if(point_n >= 3) 
	{
		is_inside = true;
		I32 prev = point_n - 1;
		for(I32 i = 0; i < point_n; i++) 
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

static R32
func DotProduct(V2 vector1, V2 vector2)
{
	R32 result = ((vector1.x * vector2.x) + (vector1.y * vector2.y));
	return result;
}

static V2
func ParallelVector(V2 vector, V2 base)
{
	V2 unit_base = NormalVector(base);
	V2 result = DotProduct(vector, unit_base) * unit_base;
	return result;
}