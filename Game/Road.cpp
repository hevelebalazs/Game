#include "Road.h"
#include <stdio.h>
#include <math.h>

void Road::draw(Bitmap bitmap){
	int top = 0;
	int left = 0;
	int bottom = 0;
	int right = 0;

	if (endPoint1.x == endPoint2.x) {
		left = (int)(endPoint1.x - width / 2);
		right = (int)(endPoint2.x + width / 2);
		top = (int)(endPoint1.y);
		bottom = (int)(endPoint2.y);
	}
	else if (endPoint1.y == endPoint2.y) {
		left = (int)(endPoint1.x);
		right = (int)(endPoint2.x);
		top = (int)(endPoint1.y - width / 2);
		bottom = (int)(endPoint2.y + width / 2);
	}
	else {
		printf("Road.draw: invalid endpoints!");
	}

	Color color = { 0.5, 0.5, 0.5 };
	bitmap.drawRect(top, left, bottom, right, color);
}