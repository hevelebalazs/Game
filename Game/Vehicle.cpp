#include <math.h>
#include <stdio.h>

#include "Geometry.h"
#include "Vehicle.h"

// TODO: move this to a math file?
static float PI = 3.14159265358979323f;

void Vehicle::MoveTo(DirectedPoint point) {
	position = point.position;
	angle = VectorAngle(point.direction);
}

void Vehicle::Draw(Renderer renderer) {
	Point addWidth = (width / 2.0f) * Point::Rotation(angle + PI / 2.0f);

	Point side1 = position + addWidth;
	Point side2 = position - addWidth;

	Point addLength = (length / 2.0f) * Point::Rotation(angle);

	Point points[4] = {
		side1 + addLength, side1 - addLength,
		side2 - addLength, side2 + addLength
	};

	renderer.DrawQuad(points, color);
}