#pragma once

#include "Geometry.h"
#include "Renderer.h"
#include "Point.h"

inline void LightSector(Renderer renderer, Point center, float minDistance, float maxDistance, float minAngle, float maxAngle, float baseBrightness) 
{
	Point radius = Point{maxDistance, maxDistance};
	Point topLeftCoord = PointDiff(center, radius);
	Point bottomRightCoord = PointSum(center, radius);

	Bitmap bitmap = renderer.bitmap;
	Camera* camera = renderer.camera;
	Point topLeftPixel = CoordToPixel(*camera, topLeftCoord);
	Point bottomRightPixel = CoordToPixel(*camera, bottomRightCoord);

	int top    = IntMax2((int)topLeftPixel.y, 0);
	int bottom = IntMin2((int)bottomRightPixel.y, bitmap.height - 1);
	int left   = IntMax2((int)topLeftPixel.x, 0);
	int right  = IntMin2((int)bottomRightPixel.x, bitmap.width - 1);

	for (int row = top; row <= bottom; ++row) {
		for (int col = left; col <= right; ++col) {
			Point pixelPosition = {};
			pixelPosition.x = (float)col;
			pixelPosition.y = (float)row;
			Point position = PixelToCoord(*camera, pixelPosition);
			float distance = Distance(center, position);
			float angle = LineAngle(center, position);

			float ratio = 0.0f;
			if (minDistance <= distance && distance <= maxDistance) {
				if (IsAngleBetween(minAngle, angle, maxAngle))
					ratio = 1.0f - (distance / maxDistance);
			}
			ratio *= baseBrightness;

			// TODO: make this code run faster
			// TODO: create a type for color code?
			unsigned int* pixel  = GetPixelAddress(bitmap, row, col);
			Color oldColor = ColorFromCode(*pixel);
			Color newColor = Color{ratio, ratio, ratio};
			Color color = ColorSum(oldColor, newColor);
			*pixel = ColorCode(color);
		}
	}
}

inline void LightCircle(Renderer renderer, Point center, float seeDistance, float baseBrightness = 1.0f) {
	LightSector(renderer, center, 0.0f, seeDistance, -PI, PI, baseBrightness);
}