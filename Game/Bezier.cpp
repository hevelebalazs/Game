#include "Bezier.h"
#include "Geometry.h"
#include "Point.h"

Bezier4 TurnBezier4(DirectedPoint startPoint, DirectedPoint endPoint) {
	Bezier4 result = {};

	float turnRatio = PointDistance(startPoint.position, endPoint.position) * 0.33f;

	result.points[0] = startPoint.position;
	result.points[1] = startPoint.position + (turnRatio * startPoint.direction);
	result.points[2] = endPoint.position - (turnRatio * endPoint.direction);
	result.points[3] = endPoint.position;

	return result;
}

Point Bezier4Point(Bezier4 bezier4, float ratio) {
	float ratio2 = (1.0f - ratio);

	Point p12 = ratio2 * bezier4.points[0] + ratio * bezier4.points[1];
	Point p23 = ratio2 * bezier4.points[1] + ratio * bezier4.points[2];
	Point p34 = ratio2 * bezier4.points[2] + ratio * bezier4.points[3];

	Point p123 = ratio2 * p12 + ratio * p23;
	Point p234 = ratio2 * p23 + ratio * p34;

	Point p1234 = ratio2 * p123 + ratio * p234;

	return p1234;
}

DirectedPoint Bezier4DirectedPoint(Bezier4 bezier4, float ratio) {
	float ratio2 = (1.0f - ratio);

	Point p12 = ratio2 * bezier4.points[0] + ratio * bezier4.points[1];
	Point p23 = ratio2 * bezier4.points[1] + ratio * bezier4.points[2];
	Point p34 = ratio2 * bezier4.points[2] + ratio * bezier4.points[3];

	Point p123 = ratio2 * p12 + ratio * p23;
	Point p234 = ratio2 * p23 + ratio * p34;

	Point p1234 = ratio2 * p123 + ratio * p234;

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

		renderer.DrawLine(point, nextPoint, color, lineWidth);

		point = nextPoint;
	}
}