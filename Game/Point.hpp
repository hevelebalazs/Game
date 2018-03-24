#pragma once

// TODO: move this whole thing to Geometry?
// TODO: rename this to Vector?
struct Point {
	float x;
	float y;
};

struct DirectedPoint {
	Point position;
	Point direction;
};

Point PointSum(Point point1, Point point2);
Point PointDiff(Point point1, Point point2);
Point PointProd(float times, Point point);
bool PointEqual(Point point1, Point point2);