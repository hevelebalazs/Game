#include "Bezier.h"
#include "Geometry.h"
#include "Point.h"

Bezier4 TurnBezier4(DirectedPoint startPoint, DirectedPoint endPoint) {
	Bezier4 result = {};

	float turnRatio = Distance(startPoint.position, endPoint.position) * 0.33f;

	result.points[0] = startPoint.position;
	result.points[1] = PointSum(startPoint.position, PointProd(turnRatio, startPoint.direction));
	result.points[2] = PointDiff(endPoint.position, PointProd(turnRatio, endPoint.direction));
	result.points[3] = endPoint.position;

	return result;
}

static Point Bezier4Interpolation(Point point1, float ratio1, Point point2, float ratio2) {
	Point part1 = PointProd(ratio1, point1);
	Point part2 = PointProd(ratio2, point2);

	Point result = PointSum(part1, part2);
	return result;
}

Point Bezier4Point(Bezier4 bezier4, float ratio) {
	float ratio2 = (1.0f - ratio);

	Point p12 = Bezier4Interpolation(bezier4.points[0], ratio2, bezier4.points[1], ratio);
	Point p23 = Bezier4Interpolation(bezier4.points[1], ratio2, bezier4.points[2], ratio);
	Point p34 = Bezier4Interpolation(bezier4.points[2], ratio2, bezier4.points[3], ratio);

	Point p123 = Bezier4Interpolation(p12, ratio2, p23, ratio);
	Point p234 = Bezier4Interpolation(p23, ratio2, p34, ratio);

	Point p1234 = Bezier4Interpolation(p123, ratio2, p234, ratio);

	return p1234;
}

DirectedPoint Bezier4DirectedPoint(Bezier4 bezier4, float ratio) {
	float ratio2 = (1.0f - ratio);

	Point p12 = Bezier4Interpolation(bezier4.points[0], ratio2, bezier4.points[1], ratio);
	Point p23 = Bezier4Interpolation(bezier4.points[1], ratio2, bezier4.points[2], ratio);
	Point p34 = Bezier4Interpolation(bezier4.points[2], ratio2, bezier4.points[3], ratio);

	Point p123 = Bezier4Interpolation(p12, ratio2, p23, ratio);
	Point p234 = Bezier4Interpolation(p23, ratio2, p34, ratio);

	Point p1234 = Bezier4Interpolation(p123, ratio2, p234, ratio);

	DirectedPoint result = {};
	result.position = p1234;
	result.direction = PointDirection(p123, p234);

	return result;
}

void DrawBezier4(Bezier4 bezier4, Renderer renderer, Color color, float lineWidth, int segmentCount) {
	Point point = bezier4.points[0];

	for (int i = 1; i <= segmentCount; ++i) {
		float ratio = ((float)i) / ((float)segmentCount);
		Point nextPoint = Bezier4Point(bezier4, ratio);

		DrawLine(renderer, point, nextPoint, color, lineWidth);

		point = nextPoint;
	}
}