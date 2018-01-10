#pragma once

#include "Bitmap.h"
#include "Memory.h"
#include "Point.h"
#include "Renderer.h"

struct Camera {
	float pixelCoordRatio;

	Point center;
	Point screenSize;

	float zoomSpeed;
	float zoomTargetRatio;
};

// TODO: rename this to RenderContext or some other "passive" noun?
struct Renderer {
	Bitmap bitmap;
	Camera* camera;
};

void FloodFill(Renderer renderer, Point start, Color color, MemArena* tmpArena);

void ApplyBitmapMask(Bitmap bitmap, Bitmap mask);

void SmoothZoom(Camera* camera, float pixelCoordRatio);
void UpdateCamera(Camera* camera, float seconds);

float CoordXtoPixel(Camera camera, float coordX);
float CoordYtoPixel(Camera camera, float coordY);
Point PixelToCoord(Camera camera, Point pixel);
Point CoordToPixel(Camera camera, Point coord);

void ClearScreen(Renderer renderer, Color color);

void Bresenham(Renderer renderer, Point point1, Point point2, Color color);
void DrawGridLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth);
void DrawLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth);
void DrawRect(Renderer renderer, float top, float left, float bottom, float right, Color color);
void DrawQuad(Renderer renderer, Point points[4], Color color);