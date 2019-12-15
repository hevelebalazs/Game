#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Math.hpp"
#include "Type.hpp"

struct Bezier4 
{
	Vec2 points[4];
};

static Bezier4
func TurnBezier4(Vec4 start_point, Vec4 end_point)
{
	Bezier4 result = {};

	Real32 turn_ratio = Distance(start_point.position, end_point.position) * 0.33f;

	result.points[0] = start_point.position;
	result.points[1] = start_point.position + (turn_ratio * start_point.direction);
	result.points[2] = end_point.position - (turn_ratio * end_point.direction);
	result.points[3] = end_point.position;

	return result;
}

static Vec2
func Bezier4Interpolation(Vec2 point1, Real32 ratio1, Vec2 point2, Real32 ratio2)
{
	Vec2 result = (ratio1 * point1) + (ratio2 * point2);
	return result;
}

static Vec2
func Bezier4Point(Bezier4 bezier4, Real32 ratio)
{
	Real32 ratio2 = (1.0f - ratio);

	Vec2 p12 = Bezier4Interpolation(bezier4.points[0], ratio2, bezier4.points[1], ratio);
	Vec2 p23 = Bezier4Interpolation(bezier4.points[1], ratio2, bezier4.points[2], ratio);
	Vec2 p34 = Bezier4Interpolation(bezier4.points[2], ratio2, bezier4.points[3], ratio);

	Vec2 p123 = Bezier4Interpolation(p12, ratio2, p23, ratio);
	Vec2 p234 = Bezier4Interpolation(p23, ratio2, p34, ratio);

	Vec2 p1234 = Bezier4Interpolation(p123, ratio2, p234, ratio);

	return p1234;
}

static Vec4
func Bezier4DirectedPoint(Bezier4 bezier4, Real32 ratio)
{
	Real32 ratio2 = (1.0f - ratio);

	Vec2 p12 = Bezier4Interpolation(bezier4.points[0], ratio2, bezier4.points[1], ratio);
	Vec2 p23 = Bezier4Interpolation(bezier4.points[1], ratio2, bezier4.points[2], ratio);
	Vec2 p34 = Bezier4Interpolation(bezier4.points[2], ratio2, bezier4.points[3], ratio);

	Vec2 p123 = Bezier4Interpolation(p12, ratio2, p23, ratio);
	Vec2 p234 = Bezier4Interpolation(p23, ratio2, p34, ratio);

	Vec2 p1234 = Bezier4Interpolation(p123, ratio2, p234, ratio);

	Vec4 result = {};
	result.position = p1234;
	result.direction = PointDirection(p123, p234);

	return result;
}

#define Bezier4SegmentN 10
#define Bezier4PointN (Bezier4SegmentN + 1)

static Real32
func MoveOnBezier4(Bezier4 bezier4, Real32 start_ratio, Real32 move_distance)
{
	Assert(IsBetween(start_ratio, 0.0f, 1.0f));
	Assert(move_distance >= 0.0f);
	Vec2 position = Bezier4Point(bezier4, start_ratio);
	Real32 ratio = start_ratio;
	Int32 next_point_index = Floor(ratio * Bezier4SegmentN) + 1;
	while(next_point_index <= Bezier4SegmentN) 
	{
		Real32 next_ratio = (Real32)next_point_index / Bezier4SegmentN;
		Vec2 next_position = Bezier4Point(bezier4, next_ratio);
		Real32 distance = Distance(position, next_position);
		if (move_distance <= distance) 
		{
			ratio = Lerp(ratio, move_distance / distance, next_ratio);
			break;
		} 
		else 
		{
			move_distance -= distance;
			position = next_position;
			ratio = next_ratio;
			next_point_index++;
		}
	}

	Assert (IsBetween (ratio, 0.0f, 1.0f));
	return ratio;
}

static Real32
func GetBezier4DistanceFromEnd(Bezier4 bezier4, Real32 ratio)
{
	Real32 result = 0;
	Vec2 position = Bezier4Point(bezier4, ratio);
	Int32 next_point_index = Floor(ratio * Bezier4PointN) + 1;
	while (next_point_index <= Bezier4PointN) 
	{
		Vec2 next_position = Bezier4Point(bezier4, (Real32)next_point_index / Bezier4PointN);
		Real32 distance = Distance(position, next_position);
		result += distance;
		position = next_position;
		next_point_index++;
	}
	return result;
}

static void
func DrawBezier4(Canvas *canvas, Bezier4 bezier4, Vec4 color, 
				 Real32 line_width, Int32 segment_count)
{
	Vec2 point = bezier4.points[0];

	for(Int32 i = 1; i <= segment_count; ++i) 
	{
		Real32 ratio = (Real32)i / (Real32)segment_count;
		Vec2 next_point = Bezier4Point(bezier4, ratio);

		DrawLine(canvas, point, next_point, color, line_width);

		point = next_point;
	}
}