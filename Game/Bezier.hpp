#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Math.hpp"
#include "Type.hpp"

struct Bezier4 {
	V2 points[4];
};

Bezier4 TurnBezier4(V4 startPoint, V4 endPoint);

V2 Bezier4Point(Bezier4 bezier4, F32 ratio);
V4 Bezier4DirectedPoint(Bezier4 bezier4, F32 ratio);

void DrawBezier4(Canvas canvas, Bezier4 bezier4, V4 color, F32 lineWidth, I32 segmentCount);
