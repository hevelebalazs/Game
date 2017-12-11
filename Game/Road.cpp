#include <stdio.h>
#include <math.h>

#include "Geometry.h"
#include "Math.h"
#include "Point.h"
#include "Road.h"

// TODO: introduce road angle and use it for calculations

float DistanceSquareFromRoad(Road road, Point point) {
	Point closest = ClosestRoadPoint(road, point);

	return DistanceSquare(point, closest);
}

Point ClosestRoadPoint(Road road, Point point) {
	Point result = {};

	if (road.endPoint1.x == road.endPoint2.x) {
		float minY = road.endPoint1.y;
		float maxY = road.endPoint2.y;

		if (minY > maxY) {
			float tmp = minY;
			minY = maxY;
			maxY = tmp;
		}

		result.x = road.endPoint1.x;

		if (minY <= point.y && point.y <= maxY) {
			result.y = point.y;
		}
		else if (point.y < minY) {
			result.y = minY;
		}
		else {
			result.y = maxY;
		}
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		float minX = road.endPoint1.x;
		float maxX = road.endPoint2.x;

		if (minX > maxX) {
			float tmp = minX;
			minX = maxX;
			maxX = tmp;
		}

		result.y = road.endPoint1.y;

		if (minX <= point.x && point.x <= maxX) {
			result.x = point.x;
		}
		else if (point.x < minX) {
			result.x = minX;
		}
		else {
			result.y = maxX;
		}
	}

	return result;
}

bool IsPointOnRoad(Point point, Road road) {
	float left   = Min2(road.endPoint1.x, road.endPoint2.x);
	float right  = Max2(road.endPoint1.x, road.endPoint2.x);
	float top    = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	if (left == right) {
		left  -= road.width * 0.5f;
		right += road.width * 0.5f;
	}

	if (top == bottom) {
		top    -= road.width * 0.5f;
		bottom += road.width * 0.5f;
	}

	if (point.x < left || point.x > right) return false;
	if (point.y < top || point.y > bottom) return false;

	return true;
}

void HighlightRoad(Renderer renderer, Road road, Color color) {
	float left = Min2(road.endPoint1.x, road.endPoint2.x);
	float right = Max2(road.endPoint1.x, road.endPoint2.x);
	float top = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	if (left == right) {
		left -= road.width * 0.5f;
		right += road.width * 0.5f;
	}

	if (top == bottom) {
		top -= road.width * 0.5f;
		bottom += road.width * 0.5f;
	}

	DrawRect(renderer, top, left, bottom, right, color);
}

void DrawRoad(Renderer renderer, Road road) {
	float stripeWidth = road.width / 20.0f;

	float top = 0.0f;
	float left = 0.0f;
	float bottom = 0.0f;
	float right = 0.0f;

	float stripeTop = 0.0f;
	float stripeLeft = 0.0f;
	float stripeBottom = 0.0f;
	float stripeRight = 0.0f;

	if (road.endPoint1.x == road.endPoint2.x) {
		left   = road.endPoint1.x - (road.width / 2.0f);
		right  = road.endPoint2.x + (road.width / 2.0f);
		top    = road.endPoint1.y;
		bottom = road.endPoint2.y;

		stripeLeft   = road.endPoint1.x - (stripeWidth / 2.0f);
		stripeRight  = road.endPoint2.x + (stripeWidth / 2.0f);
		stripeTop    = road.endPoint1.y;
		stripeBottom = road.endPoint2.y;
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		left   = road.endPoint1.x;
		right  = road.endPoint2.x;
		top    = road.endPoint1.y - (road.width / 2.0f);
		bottom = road.endPoint2.y + (road.width / 2.0f);

		stripeLeft   = road.endPoint1.x;
		stripeRight  = road.endPoint2.x;
		stripeTop    = road.endPoint1.y - (stripeWidth / 2.0f);
		stripeBottom = road.endPoint2.y + (stripeWidth / 2.0f);
	}

	Color roadColor = { 0.5f, 0.5f, 0.5f };
	DrawRect(renderer, top, left, bottom, right, roadColor);

	Color stripeColor = { 1.0f, 1.0f, 1.0f };
	DrawRect(renderer, stripeTop, stripeLeft, stripeBottom, stripeRight, stripeColor);
}