#pragma once

#include "Bitmap.hpp"
#include "Debug.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Point.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"

struct Camera {
	Point center;
	Point screenPixelSize;

	float unitInPixels;
	float targetUnitInPixels;
};

// TODO: rename this to RenderContext or some other "passive" noun?
struct Renderer {
	Bitmap bitmap;
	Camera* camera;
};

void ResizeCamera(Camera* camera, int width, int height);

inline unsigned int* GetPixelAddress(Bitmap bitmap, int row, int col) {
	return (unsigned int*)bitmap.memory + row * bitmap.width + col;
}

void FloodFill(Renderer renderer, Point start, Color color, MemArena* tmpArena);

void ApplyBitmapMask(Bitmap bitmap, Bitmap mask);

void SmoothZoom(Camera* camera, float pixelPerUnit);
void UpdateCamera(Camera* camera, float seconds);

float GetUnitDistanceInPixel(Camera camera, float unitDistance);

// TODO: Why do these return floats and not ints?
float UnitXtoPixel(Camera camera, float unitX);
float UnitYtoPixel(Camera camera, float unitY);
Point UnitToPixel(Camera camera, Point unit);

float PixelToUnitX(Camera camera, float pixelX);
float PixelToUnitY(Camera camera, float pixelY);
Point PixelToUnit(Camera camera, Point pixel);

float CameraLeftSide(Camera* camera);
float CameraRightSide(Camera* camera);
float CameraTopSide(Camera* camera);
float CameraBottomSide(Camera* camera);

void ClearScreen(Renderer renderer, Color color);

void Bresenham(Renderer renderer, Point point1, Point point2, Color color);
void DrawHorizontalTrapezoid(Renderer renderer, Point topLeft, Point topRight, Point bottomLeft, Point bottomRight, Color color);
void DrawVerticalTrapezoid(Renderer renderer, Point topLeft, Point topRight, Point bottomLeft, Point bottomRight, Color color);
void DrawGridLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth);
void DrawLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth);
void DrawWorldTextureLine(Renderer renderer, Point point1, Point point2, float lineWidth, Texture texture);
void DrawRect(Renderer renderer, float top, float left, float bottom, float right, Color color);

inline void DrawRectOutline(Renderer renderer, float top, float left, float bottom, float right, Color color) {
	Point topLeft     = Point{left, top};
	Point topRight    = Point{right, top};
	Point bottomLeft  = Point{left, bottom};
	Point bottomRight = Point{right, bottom};

	Bresenham(renderer, topLeft, topRight, color);
	Bresenham(renderer, topRight, bottomRight, color);
	Bresenham(renderer, bottomRight, bottomLeft, color);
	Bresenham(renderer, bottomLeft, topLeft, color);
}

// TODO: change the order of parameters to left, right, top, bottom
inline void WorldTextureRect(Renderer renderer, float top, float left, float bottom, float right, Texture texture) {
	Camera camera = *renderer.camera;
	int topPixel =    (int)UnitYtoPixel(camera, top);
	int leftPixel =   (int)UnitXtoPixel(camera, left);
	int bottomPixel = (int)UnitYtoPixel(camera, bottom);
	int rightPixel =  (int)UnitXtoPixel(camera, right);

	if (topPixel > bottomPixel) {
		int tmp = topPixel;
		topPixel = bottomPixel;
		bottomPixel = tmp;
	}

	if (leftPixel > rightPixel) {
		int tmp = leftPixel;
		leftPixel = rightPixel;
		rightPixel = tmp;
	}

	Bitmap bitmap = renderer.bitmap;

	if (topPixel < 0)
		topPixel = 0;
	if (bottomPixel >= bitmap.height)
		bottomPixel = (bitmap.height - 1);

	if (leftPixel < 0)
		leftPixel = 0;
	if (rightPixel >= bitmap.width)
		rightPixel = (bitmap.width - 1);

	float textureZoom = 16.0f;

	Point topLeftPixel = {(float)leftPixel, (float)topPixel};
	Point topLeftUnit = PixelToUnit(camera, topLeftPixel);
	// NOTE: optimization stuff
	topLeftUnit.x *= textureZoom;
	topLeftUnit.y *= textureZoom;

	float unitPerPixel = Invert(camera.unitInPixels);
	// NOTE: optimization stuff
	unitPerPixel *= textureZoom;

	int subUnitPerPixel = (int)(unitPerPixel * 255.0f);

	unsigned int* topLeft = (unsigned int*)bitmap.memory + topPixel * bitmap.width + leftPixel;

	// TODO: try to optimize this code
	Point worldUnit = topLeftUnit;
	int leftx  = (((int)(worldUnit.x)) & (texture.side - 1));
	int topy   = (((int)(worldUnit.y)) & (texture.side - 1));

	int leftsubx = (int)((worldUnit.x - Floor(worldUnit.x)) * 255.0f);
	int topsuby = (int)((worldUnit.y - Floor(worldUnit.y)) * 255.0f);

	int x = leftx;
	int y = topy;

	int subx = leftsubx;
	int suby = topsuby;

	for (int row = topPixel; row < bottomPixel; ++row) {
		x = leftx;
		subx = leftsubx;

		unsigned int* pixel = topLeft;

		for (int col = leftPixel; col < rightPixel; ++col) {
			*pixel = TextureColorCodeInt(texture, y, x);
			//*pixel = TextureColorCode(texture, x, subx, y, suby);
			pixel++;

			subx += subUnitPerPixel;
			if (subx > 0xFF) {
				x = ((x + (subx >> 8)) & (texture.side - 1));
				subx = (subx & 0xFF);
			}
		}

		suby += subUnitPerPixel;
		if (suby > 0xFF) {
			y = ((y + (suby >> 8)) & (texture.side - 1));
			suby = (suby & 0xFF);
		}

		topLeft += bitmap.width;
	}
}

inline void WorldTextureGridLine(Renderer renderer, Point point1, Point point2, float width, Texture texture) {
	float left   = Min2(point1.x, point2.x);
	float right  = Max2(point1.x, point2.x);
	float top    = Min2(point1.y, point2.y);
	float bottom = Max2(point1.y, point2.y);

	// TODO: this Assert was triggered, check what's happening
	Assert((left == right) || (top == bottom));
	if (left == right) {
		left  -= width * 0.5f;
		right += width * 0.5f;
	}
	else if (top == bottom) {
		top    -= width * 0.5f;
		bottom += width * 0.5f;
	}

	WorldTextureRect(renderer, top, left, bottom, right, texture);
}

void DrawPolyOutline(Renderer renderer, Point* points, int pointN, Color color);
void DrawPoly(Renderer renderer, Point* points, int pointN, Color color);
void DrawWorldTexturePoly(Renderer renderer, Point* points, int pointN, Texture texture);

void DrawQuad(Renderer renderer, Quad quad, Color color);
void DrawWorldTextureQuad(Renderer renderer, Quad quad, Texture texture);
void DrawQuadPoints(Renderer renderer, Point point1, Point point2, Point point3, Point point4, Color color);

void DrawScaledRotatedBitmap(Renderer renderer, Bitmap* bitmap, Point position, float width, float height, float rotationAngle);
void DrawStretchedBitmap(Renderer renderer, Bitmap* bitmap, float left, float right, float top, float bottom);