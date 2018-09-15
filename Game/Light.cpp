#include "Light.hpp"

void LightSector(Canvas canvas, V2 center, F32 minDistance, F32 maxDistance, F32 minAngle, F32 maxAngle, F32 baseBrightness) 
{
	V2 radius = MakePoint(maxDistance, maxDistance);
	V2 topLeft = (center - radius);
	V2 bottomRight = (center + radius);

	Bitmap bitmap = canvas.bitmap;
	Camera* camera = canvas.camera;
	V2 topLeftPixel = UnitToPixel(camera, topLeft);
	V2 bottomRightPixel = UnitToPixel(camera, bottomRight);

	I32 top    = IntMax2((I32)topLeftPixel.y, 0);
	I32 bottom = IntMin2((I32)bottomRightPixel.y, bitmap.height - 1);
	I32 left   = IntMax2((I32)topLeftPixel.x, 0);
	I32 right  = IntMin2((I32)bottomRightPixel.x, bitmap.width - 1);

	for (I32 row = top; row <= bottom; ++row) 
	{
		for (I32 col = left; col <= right; ++col) 
		{
			V2 pixelPosition = {};
			pixelPosition.x = (F32)col;
			pixelPosition.y = (F32)row;
			V2 position = PixelToUnit(camera, pixelPosition);
			F32 distance = Distance(center, position);
			F32 angle = LineAngle(center, position);

			F32 ratio = 0.0f;
			if (minDistance <= distance && distance <= maxDistance) 
			{
				if (IsAngleBetween(minAngle, angle, maxAngle))
					ratio = 1.0f - (distance / maxDistance);
			}
			ratio *= baseBrightness;

			// TODO: make this code run faster
			// TODO: create a type for color code?
			U32* pixel  = GetPixelAddress(bitmap, row, col);
			V4 oldColor = GetColorFromColorCode(*pixel);
			V4 newColor = MakeColor(ratio, ratio, ratio);
			V4 color = AddColors(oldColor, newColor);
			*pixel = GetColorCode(color);
		}
	}
}

void LightCircle(Canvas canvas, V2 center, F32 seeDistance, F32 baseBrightness)
{
	LightSector(canvas, center, 0.0f, seeDistance, -PI, PI, baseBrightness);
}