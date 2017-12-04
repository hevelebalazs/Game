#pragma once
#include "Renderer.h"
#include "Bitmap.h"
#include "Point.h"

struct Camera {
	float pixelCoordRatio;
	Point center;
	Point screenSize;
};

struct Renderer {
	Bitmap bitmap;
	Camera camera;
};

float CoordXtoPixel(Camera camera, float coordX);
float CoordYtoPixel(Camera camera, float coordY);
Point PixelToCoord(Camera camera, Point pixel);
Point CoordToPixel(Camera camera, Point coord);

void ClearScreen(Renderer renderer, Color color);
void DrawLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth);
void DrawRect(Renderer renderer, float top, float left, float bottom, float right, Color color);
void DrawQuad(Renderer renderer, Point points[4], Color color);