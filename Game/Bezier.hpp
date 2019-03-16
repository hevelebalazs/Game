#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Math.hpp"
#include "Type.hpp"

struct Bezier4 
{
	Vec2 points[4];
};

static Bezier4 func TurnBezier4(Vec4 startPoint, Vec4 endPoint)
{
	Bezier4 result = {};

	Real32 turnRatio = Distance(startPoint.position, endPoint.position) * 0.33f;

	result.points[0] = startPoint.position;
	result.points[1] = startPoint.position + (turnRatio * startPoint.direction);
	result.points[2] = endPoint.position - (turnRatio * endPoint.direction);
	result.points[3] = endPoint.position;

	return result;
}

static Vec2 func Bezier4Interpolation(Vec2 point1, Real32 ratio1, Vec2 point2, Real32 ratio2)
{
	Vec2 result = (ratio1 * point1) + (ratio2 * point2);
	return result;
}

static Vec2 func Bezier4Point(Bezier4 bezier4, Real32 ratio)
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

static Vec4 func Bezier4DirectedPoint(Bezier4 bezier4, Real32 ratio)
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

static Real32 func MoveOnBezier4(Bezier4 bezier4, Real32 startRatio, Real32 moveDistance)
{
	Assert(IsBetween(startRatio, 0.0f, 1.0f));
	Assert(moveDistance >= 0.0f);
	Vec2 position = Bezier4Point(bezier4, startRatio);
	Real32 ratio = startRatio;
	Int32 nextPointIndex = Floor(ratio * Bezier4SegmentN) + 1;
	while(nextPointIndex <= Bezier4SegmentN) 
	{
		Real32 nextRatio = (Real32)nextPointIndex / Bezier4SegmentN;
		Vec2 nextPosition = Bezier4Point(bezier4, nextRatio);
		Real32 distance = Distance(position, nextPosition);
		if (moveDistance <= distance) 
		{
			ratio = Lerp(ratio, moveDistance / distance, nextRatio);
			break;
		} 
		else 
		{
			moveDistance -= distance;
			position = nextPosition;
			ratio = nextRatio;
			nextPointIndex++;
		}
	}

	Assert (IsBetween (ratio, 0.0f, 1.0f));
	return ratio;
}

static Real32 func GetBezier4DistanceFromEnd(Bezier4 bezier4, Real32 ratio)
{
	Real32 result = 0;
	Vec2 position = Bezier4Point(bezier4, ratio);
	Int32 nextPointIndex = Floor(ratio * Bezier4PointN) + 1;
	while (nextPointIndex <= Bezier4PointN) 
	{
		Vec2 nextPosition = Bezier4Point(bezier4, (Real32)nextPointIndex / Bezier4PointN);
		Real32 distance = Distance(position, nextPosition);
		result += distance;
		position = nextPosition;
		nextPointIndex++;
	}
	return result;
}

static void func DrawBezier4(Canvas* canvas, Bezier4 bezier4, Vec4 color, Real32 lineWidth, Int32 segmentCount)
{
	Vec2 point = bezier4.points[0];

	for(Int32 i = 1; i <= segmentCount; ++i) 
	{
		Real32 ratio = (Real32)i / (Real32)segmentCount;
		Vec2 nextPoint = Bezier4Point(bezier4, ratio);

		DrawLine(canvas, point, nextPoint, color, lineWidth);

		point = nextPoint;
	}
}