#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Math.hpp"
#include "Type.hpp"

struct Bezier4 
{
	V2 points[4];
};

static Bezier4
func TurnBezier4(V4 start_point, V4 end_point)
{
	Bezier4 result = {};

	R32 turn_ratio = Distance(start_point.position, end_point.position) * 0.33f;

	result.points[0] = start_point.position;
	result.points[1] = start_point.position + (turn_ratio * start_point.direction);
	result.points[2] = end_point.position - (turn_ratio * end_point.direction);
	result.points[3] = end_point.position;

	return result;
}

static V2
func Bezier4Interpolation(V2 point1, R32 ratio1, V2 point2, R32 ratio2)
{
	V2 result = (ratio1 * point1) + (ratio2 * point2);
	return result;
}

static V2
func Bezier4Point(Bezier4 bezier4, R32 ratio)
{
	R32 ratio2 = (1.0f - ratio);

	V2 p12 = Bezier4Interpolation(bezier4.points[0], ratio2, bezier4.points[1], ratio);
	V2 p23 = Bezier4Interpolation(bezier4.points[1], ratio2, bezier4.points[2], ratio);
	V2 p34 = Bezier4Interpolation(bezier4.points[2], ratio2, bezier4.points[3], ratio);

	V2 p123 = Bezier4Interpolation(p12, ratio2, p23, ratio);
	V2 p234 = Bezier4Interpolation(p23, ratio2, p34, ratio);

	V2 p1234 = Bezier4Interpolation(p123, ratio2, p234, ratio);

	return p1234;
}

static V4
func Bezier4DirectedPoint(Bezier4 bezier4, R32 ratio)
{
	R32 ratio2 = (1.0f - ratio);

	V2 p12 = Bezier4Interpolation(bezier4.points[0], ratio2, bezier4.points[1], ratio);
	V2 p23 = Bezier4Interpolation(bezier4.points[1], ratio2, bezier4.points[2], ratio);
	V2 p34 = Bezier4Interpolation(bezier4.points[2], ratio2, bezier4.points[3], ratio);

	V2 p123 = Bezier4Interpolation(p12, ratio2, p23, ratio);
	V2 p234 = Bezier4Interpolation(p23, ratio2, p34, ratio);

	V2 p1234 = Bezier4Interpolation(p123, ratio2, p234, ratio);

	V4 result = {};
	result.position = p1234;
	result.direction = PointDirection(p123, p234);

	return result;
}

#define Bezier4SegmentN 10
#define Bezier4PointN (Bezier4SegmentN + 1)

static R32
func MoveOnBezier4(Bezier4 bezier4, R32 start_ratio, R32 move_distance)
{
	Assert(IsBetween(start_ratio, 0.0f, 1.0f));
	Assert(move_distance >= 0.0f);
	V2 position = Bezier4Point(bezier4, start_ratio);
	R32 ratio = start_ratio;
	I32 next_point_index = Floor(ratio * Bezier4SegmentN) + 1;
	while(next_point_index <= Bezier4SegmentN) 
	{
		R32 next_ratio = (R32)next_point_index / Bezier4SegmentN;
		V2 next_position = Bezier4Point(bezier4, next_ratio);
		R32 distance = Distance(position, next_position);
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

static R32
func GetBezier4DistanceFromEnd(Bezier4 bezier4, R32 ratio)
{
	R32 result = 0;
	V2 position = Bezier4Point(bezier4, ratio);
	I32 next_point_index = Floor(ratio * Bezier4PointN) + 1;
	while (next_point_index <= Bezier4PointN) 
	{
		V2 next_position = Bezier4Point(bezier4, (R32)next_point_index / Bezier4PointN);
		R32 distance = Distance(position, next_position);
		result += distance;
		position = next_position;
		next_point_index++;
	}
	return result;
}

static void
func DrawBezier4(Canvas *canvas, Bezier4 bezier4, V4 color, 
				 R32 line_width, I32 segment_count)
{
	V2 point = bezier4.points[0];

	for(I32 i = 1; i <= segment_count; ++i) 
	{
		R32 ratio = (R32)i / (R32)segment_count;
		V2 next_point = Bezier4Point(bezier4, ratio);

		DrawLine(canvas, point, next_point, color, line_width);

		point = next_point;
	}
}