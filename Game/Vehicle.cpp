#include "Vehicle.h"
#include <math.h>
#include <stdio.h>

static float PI = 3.14159265358979323f;

void Vehicle::draw(Bitmap bitmap) {
	Point addWidth = (width / 2.0f) * Point::rotation(angle + PI / 2.0f);

	Point side1 = position + addWidth;
	Point side2 = position - addWidth;

	Point addLength = (length / 2.0f) * Point::rotation(angle);

	Point points[4] = {
		side1 + addLength, side1 - addLength,
		side2 - addLength, side2 + addLength
	};

	bitmap.drawQuad(points, color);
}