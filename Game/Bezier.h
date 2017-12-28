#pragma once

#include "Bitmap.h"
#include "Point.h"
#include "Renderer.h"

struct Bezier4 {
	Point points[4];
};

Bezier4 TurnBezier4(DirectedPoint startPoint, DirectedPoint endPoint);

Point Bezier4Point(Bezier4 bezier4, float ratio);
DirectedPoint Bezier4DirectedPoint(Bezier4 bezier4, float ratio);

void DrawBezier4(Renderer renderer, Bezier4 bezier4, Color color, float lineWidth, int segmentCount);