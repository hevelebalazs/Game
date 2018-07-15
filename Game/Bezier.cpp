#include "Bezier.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Type.hpp"

Bezier4 TurnBezier4(V4 startPoint, V4 endPoint)
{
	Bezier4 result = {};

	F32 turnRatio = Distance(startPoint.position, endPoint.position) * 0.33f;

	result.points[0] = startPoint.position;
	result.points[1] = startPoint.position + (turnRatio * startPoint.direction);
	result.points[2] = endPoint.position - (turnRatio * endPoint.direction);
	result.points[3] = endPoint.position;

	return result;
}

static V2 Bezier4Interpolation(V2 point1, F32 ratio1, V2 point2, F32 ratio2)
{
	V2 result = (ratio1 * point1) + (ratio2 * point2);
	return result;
}

V2 Bezier4Point(Bezier4 bezier4, F32 ratio)
{
	F32 ratio2 = (1.0f - ratio);

	V2 p12 = Bezier4Interpolation(bezier4.points[0], ratio2, bezier4.points[1], ratio);
	V2 p23 = Bezier4Interpolation(bezier4.points[1], ratio2, bezier4.points[2], ratio);
	V2 p34 = Bezier4Interpolation(bezier4.points[2], ratio2, bezier4.points[3], ratio);

	V2 p123 = Bezier4Interpolation(p12, ratio2, p23, ratio);
	V2 p234 = Bezier4Interpolation(p23, ratio2, p34, ratio);

	V2 p1234 = Bezier4Interpolation(p123, ratio2, p234, ratio);

	return p1234;
}

V4 Bezier4DirectedPoint(Bezier4 bezier4, F32 ratio)
{
	F32 ratio2 = (1.0f - ratio);

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

void DrawBezier4(Canvas canvas, Bezier4 bezier4, V4 color, F32 lineWidth, I32 segmentCount)
{
	V2 point = bezier4.points[0];

	for (I32 i = 1; i <= segmentCount; ++i) {
		F32 ratio = ((F32)i) / ((F32)segmentCount);
		V2 nextPoint = Bezier4Point(bezier4, ratio);

		DrawLine(canvas, point, nextPoint, color, lineWidth);

		point = nextPoint;
	}
}