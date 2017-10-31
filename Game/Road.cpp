#include "Road.h"
#include <stdio.h>
#include <math.h>

Point Road::enterPoint(int endPointIndex) {
	Point result = {};

	if (endPointIndex == 1) {
		result = endPoint1;
		if (endPoint1.x < endPoint2.x) result.y += (width / 4.0f);
		else if (endPoint1.x > endPoint2.x) result.y -= (width / 4.0f);
		else if (endPoint1.y < endPoint2.y) result.x -= (width / 4.0f);
		else result.x += (width / 4.0f);
	}
	else {
		result = endPoint2;
		if (endPoint1.x < endPoint2.x) result.y -= (width / 4.0f);
		else if (endPoint1.x > endPoint2.x) result.y += (width / 4.0f);
		else if (endPoint1.y < endPoint2.y) result.x += (width / 4.0f);
		else result.x -= (width / 4.0f);
	}

	return result;
}

Point Road::leavePoint(int endPointIndex) {
	Point result = {};

	if (endPointIndex == 1) {
		result = endPoint1;
		if (endPoint1.x < endPoint2.x) result.y -= (width / 4.0f);
		else if (endPoint1.x > endPoint2.x) result.y += (width / 4.0f);
		else if (endPoint1.y < endPoint2.y) result.x += (width / 4.0f);
		else result.x -= (width / 4.0f);
	}
	else {
		result = endPoint2;
		if (endPoint1.x < endPoint2.x) result.y += (width / 4.0f);
		else if (endPoint1.x > endPoint2.x) result.y -= (width / 4.0f);
		else if (endPoint1.y < endPoint2.y) result.x -= (width / 4.0f);
		else result.x += (width / 4.0f);
	}

	return result;
}

Intersection* Road::otherIntersection(Intersection *intersection) {
	if (intersection1 == intersection) return intersection2;
	else return intersection1;
}

void Road::draw(Renderer renderer) {
	float stripeWidth = width / 20.0f;

	float top = 0.0f;
	float left = 0.0f;
	float bottom = 0.0f;
	float right = 0.0f;

	float stripeTop = 0.0f;
	float stripeLeft = 0.0f;
	float stripeBottom = 0.0f;
	float stripeRight = 0.0f;

	if (endPoint1.x == endPoint2.x) {
		left = endPoint1.x - (width / 2.0f);
		right = endPoint2.x + (width / 2.0f);
		top = endPoint1.y;
		bottom = endPoint2.y;

		stripeLeft = endPoint1.x - (stripeWidth / 2.0f);
		stripeRight = endPoint2.x + (stripeWidth / 2.0f);
		stripeTop = endPoint1.y;
		stripeBottom = endPoint2.y;
	}
	else if (endPoint1.y == endPoint2.y) {
		left = endPoint1.x;
		right = endPoint2.x;
		top = endPoint1.y - (width / 2.0f);
		bottom = endPoint2.y + (width / 2.0f);

		stripeLeft = endPoint1.x;
		stripeRight = endPoint2.x;
		stripeTop = endPoint1.y - (stripeWidth / 2.0f);
		stripeBottom = endPoint2.y + (stripeWidth / 2.0f);
	}
	else {
		printf("Road.draw: invalid endpoints!");
	}

	Color roadColor = { 0.5f, 0.5f, 0.5f };
	renderer.drawRect(top, left, bottom, right, roadColor);

	Color stripeColor = { 1.0f, 1.0f, 1.0f };
	renderer.drawRect(stripeTop, stripeLeft, stripeBottom, stripeRight, stripeColor);
}