#pragma once

#include "Bitmap.h"
#include "Debug.h"
#include "Math.h"
#include "Memory.h"
#include "Point.h"
#include "Renderer.h"

struct Camera {
	Point center;
	// TODO: should half of this, a screenRadius be saved instead?
	Point screenSize;

	float moveSpeed;
	float targetAltitude;
	// TODO: rename altitude to z?
	float altitude;
};

// TODO: rename this to RenderContext or some other "passive" noun?
struct Renderer {
	Bitmap bitmap;
	Camera* camera;
};

void FloodFill(Renderer renderer, Point start, Color color, MemArena* tmpArena);

void ApplyBitmapMask(Bitmap bitmap, Bitmap mask);

void SmoothZoom(Camera* camera, float altitude);
void UpdateCamera(Camera* camera, float seconds);

// TODO: make these inline functions
// TODO: rename CoordX to X and CoordY to Y?
float CoordXtoPixel(Camera camera, float coordX);
float CoordYtoPixel(Camera camera, float coordY);
Point CoordToPixel(Camera camera, Point coord);

float PixelToCoordX(Camera camera, float pixelX);
float PixelToCoordY(Camera camera, float pixelY);
Point PixelToCoord(Camera camera, Point pixel);

inline float CameraLeftCoord(Camera* camera) {
	return PixelToCoordX(*camera, 0);
}

inline float CameraRightCoord(Camera* camera) {
	return PixelToCoordX(*camera, camera->screenSize.x - 1);
}

inline float CameraTopCoord(Camera* camera) {
	return PixelToCoordY(*camera, 0);
}

inline float CameraBottomCoord(Camera* camera) {
	return PixelToCoordY(*camera, camera->screenSize.y - 1);
}

// TODO: handle Z coordinate at render level only
//       outside code shouldn't need to call a project-to-ground and then a render function
inline float ProjectXToGround(Camera camera, float x, float z) {
	float result = 0.0f;

	float ratio = 0.0f;
	float distZ = (camera.altitude - z);
	if (distZ <= 0.0f) {
		ratio = 100.0f;
	}
	else {
		ratio = (camera.altitude / distZ);
	}
	
	result = camera.center.x + (x - camera.center.x) * ratio;
	return result;
}

inline float ProjectYToGround(Camera camera, float y, float z) {
	float result = 0.0f;

	float ratio = 0.0f;
	float distZ = (camera.altitude - z);
	if (distZ <= 0.0f)
		ratio = 100.0f;
	else
		ratio = (camera.altitude / distZ);

	result = camera.center.y + (y - camera.center.y) * ratio;
	return result;
}

inline Point ProjectPointToGround(Camera camera, Point point, float z) {
	Point result = {};
	
	float ratio = 0.0f;
	float distZ = (camera.altitude - z);
	if (distZ <= 0.0f) {
		float widthCoord = (CameraRightCoord(&camera) - CameraLeftCoord(&camera)) * 0.5f;
		float absX = Abs(point.x - camera.center.x);
		float ratioX = (widthCoord / absX);

		float heightCoord = (CameraBottomCoord(&camera) - CameraTopCoord(&camera)) * 0.5f;
		float absY = Abs(point.y - camera.center.y);
		float ratioY = (heightCoord / absY);

		ratio = Max2(ratioX, ratioY);
	}
	else {
		ratio = (camera.altitude / distZ);
	}

	result.x = camera.center.x + (point.x - camera.center.x) * ratio;
	result.y = camera.center.y + (point.y - camera.center.y) * ratio;
	return result;
}

void ClearScreen(Renderer renderer, Color color);

void Bresenham(Renderer renderer, Point point1, Point point2, Color color);
void DrawHorizontalTrapezoid(Renderer renderer, Point topLeft, Point topRight, Point bottomLeft, Point bottomRight, Color color);
void DrawVerticalTrapezoid(Renderer renderer, Point topLeft, Point topRight, Point bottomLeft, Point bottomRight, Color color);
void DrawGridLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth);
void DrawLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth);
void DrawRect(Renderer renderer, float top, float left, float bottom, float right, Color color);
void DrawQuad(Renderer renderer, Point points[4], Color color);
void DrawQuadPoints(Renderer renderer, Point point1, Point point2, Point point3, Point point4, Color color);