#pragma once

struct Point {
	static float DistanceSquare(Point point1, Point point2);
	static float CityDistance(Point point1, Point point2);
	static float DotProduct(Point point1, Point point2);

	float x;
	float y;
	Point operator+(Point otherPoint);
	Point operator-(Point otherPoint);
	Point operator+=(Point otherPoint);
	Point operator-=(Point otherPoint);

	bool operator==(Point otherPoint);

	float Length();

	static Point Rotation(float angle);
};

Point operator*(float multiplier, Point point);
Point operator*(Point point, float multiplier);