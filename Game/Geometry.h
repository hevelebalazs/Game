#pragma once
#include "Point.h"

union Line {
	struct {
		Point p1;
		Point p2;
	};

	struct {
		float x1, y1;
		float x2, y2;
	};
};

float DistanceSquare(Point point1, Point point2);
float CityDistance(Point point1, Point point2);
float Distance(Point point1, Point point2);

float VectorLength(Point vector);
float VectorAngle(Point vector);

Point RotationVector(float angle);

Point PointDirection(Point startPoint, Point endPoint);
bool TurnsRight(Point point1, Point point2, Point point3);
bool DoLinesCross(Point line11, Point line12, Point line21, Point line22);
Point LineIntersection(Point line11, Point line12, Point line21, Point line22);

float DotProduct(Point vector1, Point vector2);
Point NormalVector(Point vector);
Point ParallelVector(Point vector, Point base);