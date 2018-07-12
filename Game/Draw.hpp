#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Debug.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Texture.hpp"
#include "Type.hpp"

struct Camera {
	V2 center;
	V2 screenPixelSize;

	F32 unitInPixels;
	F32 targetUnitInPixels;
};

struct Canvas {
	Bitmap bitmap;
	Camera* camera;
};

void ResizeCamera(Camera* camera, I32 width, I32 height);

U32* GetPixelAddress(Bitmap bitmap, I32 row, I32 col);

void FloodFill(Canvas canvas, V2 start, V4 color, MemArena* tmpArena);

void ApplyBitmapMask(Bitmap bitmap, Bitmap mask);

void SmoothZoom(Camera* camera, F32 pixelPerUnit);
void UpdateCamera(Camera* camera, F32 seconds);

F32 GetUnitDistanceInPixel(Camera* camera, F32 unitDistance);

I32 UnitXtoPixel(Camera* camera, F32 unitX);
I32 UnitYtoPixel(Camera* camera, F32 unitY);
V2 UnitToPixel(Camera* camera, V2 unit);

F32 PixelToUnitX(Camera* camera, F32 pixelX);
F32 PixelToUnitY(Camera* camera, F32 pixelY);
V2 PixelToUnit(Camera* camera, V2 pixel);

F32 CameraLeftSide(Camera* camera);
F32 CameraRightSide(Camera* camera);
F32 CameraTopSide(Camera* camera);
F32 CameraBottomSide(Camera* camera);

void ClearScreen(Canvas canvas, V4 color);

void Bresenham(Canvas canvas, V2 point1, V2 point2, V4 color);
void DrawHorizontalTrapezoid(Canvas canvas, V2 topLeft, V2 topRight, V2 bottomLeft, V2 bottomRight, V4 color);
void DrawVerticalTrapezoid(Canvas canvas, V2 topLeft, V2 topRight, V2 bottomLeft, V2 bottomRight, V4 color);
void DrawGridLine(Canvas canvas, V2 point1, V2 point2, V4 color, F32 lineWidth);
void DrawLine(Canvas canvas, V2 point1, V2 point2, V4 color, F32 lineWidth);
void DrawWorldTextureLine(Canvas canvas, V2 point1, V2 point2, F32 lineWidth, Texture texture);
void FillScreenWithWorldTexture(Canvas canvas, Texture texture);

void DrawRect(Canvas canvas, F32 left, F32 right, F32 top, F32 bottom, V4 color);
void DrawRectOutline(Canvas canvas, F32 left, F32 right, F32 top, F32 bottom, V4 color);

void WorldTextureRect(Canvas canvas, F32 left, F32 right, F32 top, F32 bottom, Texture texture);
void WorldTextureGridLine(Canvas canvas, V2 point1, V2 point2, F32 width, Texture texture);

void DrawPolyOutline(Canvas canvas, V2* points, I32 pointN, V4 color);
void DrawPoly(Canvas canvas, V2* points, I32 pointN, V4 color);
void DrawWorldTexturePoly(Canvas canvas, V2* points, I32 pointN, Texture texture);

void DrawQuad(Canvas canvas, Quad quad, V4 color);
void DrawWorldTextureQuad(Canvas canvas, Quad quad, Texture texture);
void DrawQuadPoints(Canvas canvas, V2 point1, V2 point2, V2 point3, V2 point4, V4 color);

void DrawScaledRotatedBitmap(Canvas canvas, Bitmap* bitmap, V2 position, F32 width, F32 height, F32 rotationAngle);
void DrawStretchedBitmap(Canvas canvas, Bitmap* bitmap, F32 left, F32 right, F32 top, F32 bottom);
