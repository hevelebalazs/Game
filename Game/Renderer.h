#pragma once
#include "Renderer.h"
#include "Bitmap.h"
#include "Point.h"

struct Camera {
	float pixelCoordRatio;
	Point center;
	Point screenSize;

	float CoordXtoPixel(float coordX);
	float CoordYtoPixel(float coordY);
	Point PixelToCoord(Point pixel);
	Point CoordToPixel(Point coord);
};

struct Renderer {
	Bitmap bitmap;
	Camera camera;

	void Clear(Color color);
	void DrawRect(float top, float left, float bottom, float right, Color color);
	void DrawQuad(Point points[4], Color color);
};