#pragma once
#include "Renderer.h"
#include "Bitmap.h"
#include "Point.h"

struct Camera {
	float pixelCoordRatio;
	Point center;
	Point screenSize;

	float coordXtoPixel(float coordX);
	float coordYtoPixel(float coordY);
	Point pixelToCoord(Point pixel);
	Point coordToPixel(Point coord);
};

struct Renderer {
	Bitmap bitmap;
	Camera camera;

	void clear(Color color);
	void drawRect(float top, float left, float bottom, float right, Color color);
	void drawQuad(Point points[4], Color color);
};