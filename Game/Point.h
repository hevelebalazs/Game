#pragma once

struct Point {
	static float distanceSquare(Point point1, Point point2);
	static float cityDistance(Point point1, Point point2);
	float x;
	float y;
	Point operator+(Point otherPoint);
	Point operator-(Point otherPoint);
	Point operator+=(Point otherPoint);
	Point operator-=(Point otherPoint);

	bool operator==(Point otherPoint);

	float length();

	static Point rotation(float angle);
};

Point operator*(float multiplier, Point point);
Point operator*(Point point, float multiplier);