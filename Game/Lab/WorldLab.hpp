#pragma once

#include <Windows.h>

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Geometry.hpp"
#include "../UserInput.hpp"

#define WorldLabRandomTableN 16384
#define WorldLabLevelSwitchRatio 0.67f
#define WorldLabMaxValue (1.0f / (1.0f - WorldLabLevelSwitchRatio))

struct WorldLabState
{
	Real32 randomTable[WorldLabRandomTableN];
	Bitmap bitmap;
};

static void func InitRandomTable(WorldLabState* labState)
{
	for (Int32 i = 0; i < WorldLabRandomTableN; ++i)
	{
		labState->randomTable[i] = RandomBetween(-1.0f, 0.0f);
	}
}

static Real32 func SmoothRatio(Real32 x)
{
	Real32 result = x * x * (3.0f - 2.0f * x);
	return result;
}

static Real32 func GetRandomValueFromTable(WorldLabState* labState, Int32 index)
{
	Assert(index >= 0);
	Real32 value = labState->randomTable[index % WorldLabRandomTableN];
	return value;
}

static Real32 func GetGridPointValue(WorldLabState* labState, Int32 gridLevel, Int32 row, Int32 col)
{
	Int32 gridN = (1 << gridLevel);
	Assert(IsIntBetween(row, 0, gridN));
	Assert(IsIntBetween(col, 0, gridN));

	Int32 firstIndexOnLevel = 0;
	for(Int32 i = 0; i < gridLevel; i++)
	{
		firstIndexOnLevel += IntSquare ((1 << i) + 1);
	}

	Real32 value = GetRandomValueFromTable(labState, firstIndexOnLevel + (gridN + 1) * row + col);
	return value;
}

static Real32 func GetPointValue(WorldLabState* labState, Int32 gridLevel, Real32 x, Real32 y)
{
	Int32 gridN = (1 << gridLevel);
	Assert(IsBetween(x, 0.0f, 1.0f));
	Assert(IsBetween(y, 0.0f, 1.0f));

	Int32 left   = Floor(x * gridN);
	Int32 right  = left + 1;
	Int32 top    = Floor(y * gridN);
	Int32 bottom = top + 1;

	Real32 ratioX = SmoothRatio(Fraction(x * gridN));
	Real32 ratioY = SmoothRatio(Fraction(y * gridN));

	Real32 topLeftValue     = GetGridPointValue(labState, gridLevel, top, left);
	Real32 topRightValue    = GetGridPointValue(labState, gridLevel, top, right);
	Real32 bottomLeftValue  = GetGridPointValue(labState, gridLevel, bottom, left);
	Real32 bottomRightValue = GetGridPointValue(labState, gridLevel, bottom, right);

	Real32 topValue    = Lerp(topLeftValue, ratioX, topRightValue);
	Real32 bottomValue = Lerp(bottomLeftValue, ratioX, bottomRightValue);

	Real32 value = Lerp(topValue, ratioY, bottomValue);

	return value;
}

static void func GenerateGridBitmap(WorldLabState* labState, Bitmap* bitmap)
{
	Assert(bitmap != 0);
	ResizeBitmap(bitmap, 512, 512);

	Real32* values = new Real32[bitmap->width * bitmap->height];
	Int32 valueIndex = 0;
	for(Int32 row = 0; row < bitmap->height; row++)
	{
		for(Int32 col = 0; col < bitmap->width; col++)
		{
			values[valueIndex] = 0.0f;
			valueIndex++;
		}
	}

	Real32 left   = 0.0f;
	Real32 right  = 1.0f;
	Real32 top    = 0.0f;
	Real32 bottom = 1.0f;

	Real32 valueRatio = 1.0f;
	for(Int32 level = 2; level <= 8; level++)
	{
		Int32 valueIndex = 0;
		for(Int32 row = 0; row < bitmap->height; row++)
		{
			for(Int32 col = 0; col < bitmap->width; col++)
			{
				Real32 xRatio = ((Real32)col / (Real32)bitmap->width);
				Real32 x = Lerp(left, xRatio, right);

				Real32 yRatio = ((Real32)row / (Real32)bitmap->height);
				Real32 y = Lerp(top, yRatio, bottom);

				Real32 value = GetPointValue(labState, level, x, y);

				values[valueIndex] += value * valueRatio;
				valueIndex++;
			}
		}

		valueRatio *= WorldLabLevelSwitchRatio;
	}

	valueIndex = 0;
	for(Int32 row = 0; row < bitmap->height; row++)
	{
		for(Int32 col = 0; col < bitmap->width; col++)
		{
			Real32 oldValue = values[valueIndex];
			Real32 newValue = SmoothRatio((oldValue / WorldLabMaxValue) + 1.0f) * 0.5f;
			Assert(IsBetween(newValue, 0.0f, 1.0f));
			newValue = SmoothRatio(newValue);
			values[valueIndex] = newValue;
			valueIndex++;
		}
	}

	UInt32* pixel = bitmap->memory;
	Vec4 darkBlue   = MakeColor(0.0f, 0.0f, 0.1f);
	Vec4 lightBlue  = MakeColor(0.0f, 0.0f, 1.0f);
	Vec4 darkGreen  = MakeColor(0.8f, 0.8f, 0.0f);
	Vec4 lightGreen = MakeColor(0.0f, 0.8f, 0.0f);
	Vec4 lightBrown = MakeColor(0.4f, 0.4f, 0.0f);
	Vec4 darkBrown  = MakeColor(0.8f, 0.8f, 0.0f);
	Vec4 lightWhite = MakeColor(0.1f, 0.1f, 0.0f);
	Vec4 darkWhite  = MakeColor(0.4f, 0.4f, 0.0f);

	valueIndex = 0;
	for(Int32 row = 0; row < bitmap->height; row++)
	{
		for(Int32 col = 0; col < bitmap->width; col++)
		{
			Real32 value = values[valueIndex];
			valueIndex++;
			Assert(IsBetween(value, 0.0f, 1.0f));

			Vec4 color = {};

			Real32 seaLevel = 0.18f;
			Real32 hillLevel = 0.3f;
			Real32 mountainLevel = 0.4f;
			Real32 topLevel = 0.5f;

			if(IsBetween(value, 0.0f, seaLevel))
			{
				color = InterpolateColors(darkBlue, (value - 0.0f) / (seaLevel - 0.0f), lightBlue);
			}
			else if(IsBetween(value, seaLevel, hillLevel))
			{
				color = InterpolateColors(lightGreen, (value - seaLevel) / (hillLevel - seaLevel), darkGreen);
			}
			else if(IsBetween(value, hillLevel, mountainLevel))
			{
				color = InterpolateColors(darkBrown, (value - hillLevel) / (mountainLevel - hillLevel), lightBrown);
			}
			else if(IsBetween(value, mountainLevel, topLevel))
			{
				color = InterpolateColors(darkWhite, (value - mountainLevel) / (topLevel - mountainLevel), lightWhite);
			}
			else
			{
				DebugBreak();
			}

			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	delete[] values;
}

static void func WorldLabInit(WorldLabState* labState, Canvas* canvas)
{
	InitRandom();

	InitRandomTable(labState);
	GenerateGridBitmap(labState, &labState->bitmap);

	Camera* camera = canvas->camera;
	camera->unitInPixels = 10.0f;
	camera->center = MakePoint(0.0f, 0.0f);
}

static void func WorldLabUpdate(WorldLabState* labState, Canvas* canvas, Real32 seconds, UserInput* userInput)
{
	Bitmap* bitmap = &canvas->bitmap;
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	Real32 tileSide = 1.0f;
	Int32 tileRowN = 512;
	Int32 tileColN = 512;

	Real32 worldLeft   = 0.0f;
	Real32 worldRight  = tileSide * tileColN;
	Real32 worldTop    = 0.0f;
	Real32 worldBottom = tileSide * tileRowN;

	Real32 cameraMoveSpeed = seconds * 50.0f;
	Camera* camera = canvas->camera;

	if(IsKeyDown(userInput, 'A'))
	{
		camera->center.x -= cameraMoveSpeed;
	}
	if(IsKeyDown(userInput, 'D'))
	{
		camera->center.x += cameraMoveSpeed;
	}

	if(IsKeyDown(userInput, 'W'))
	{
		camera->center.y -= cameraMoveSpeed;
	}
	if(IsKeyDown(userInput, 'S'))
	{
		camera->center.y += cameraMoveSpeed;
	}

	Real32 cameraLeft   = CameraLeftSide(camera);
	Real32 cameraRight  = CameraRightSide(camera);
	Real32 cameraTop    = CameraTopSide(camera);
	Real32 cameraBottom = CameraBottomSide(camera);

	for(Int32 row = 0; row < tileRowN; row++)
	{
		Real32 tileTop    = worldTop + tileSide * row;
		Real32 tileBottom = tileTop + tileSide;
		if(tileBottom < cameraTop || tileTop > cameraBottom)
		{
			continue;
		}

		for(Int32 col = 0; col < tileColN; col++)
		{
			Real32 tileLeft  = worldLeft + tileSide * col;
			Real32 tileRight = tileLeft + tileSide;
			if(tileRight < cameraLeft || tileLeft > cameraRight)
			{
				continue;
			}

			Vec4 color = GetBitmapPixelColor(&labState->bitmap, row, col);
			DrawRectLRTB(canvas, tileLeft, tileRight, tileTop, tileBottom, color);
		}
	}

	/*
	Vec4 gridColor = MakeColor(1.0f, 1.0f, 0.0f);

	Real32 gridLeft  = Max2(worldLeft, cameraLeft);
	Real32 gridRight = Min2(worldRight, cameraRight);
	for(Int32 row = 0; row < tileRowN; row++)
	{
		Real32 tileTop    = worldTop + tileSide * row;
		Real32 tileBottom = tileTop + tileSide;

		if(tileBottom < cameraTop)
		{
			continue;
		}
		if(tileTop > cameraBottom)
		{
			break;
		}

		if(row == 0)
		{
			Bresenham(canvas, MakePoint(gridLeft, tileTop), MakePoint(gridRight, tileTop), gridColor);
		}

		Bresenham(canvas, MakePoint(gridLeft, tileBottom), MakePoint(gridRight, tileBottom), gridColor);
	}

	Real32 gridTop    = Max2(worldTop, cameraTop);
	Real32 gridBottom = Min2(worldBottom, cameraBottom);
	for(Int32 col = 0; col < tileColN; col++)
	{
		Real32 tileLeft  = worldLeft + tileSide * col;
		Real32 tileRight = tileLeft + tileSide;

		if(tileRight < cameraLeft)
		{
			continue;
		}
		if(tileLeft > cameraRight)
		{
			break;
		}

		if(col == 0)
		{
			Bresenham(canvas, MakePoint(tileLeft, gridTop), MakePoint(tileLeft, gridBottom), gridColor);
		}
		
		Bresenham(canvas, MakePoint(tileRight, gridTop), MakePoint(tileRight, gridBottom), gridColor);
	}
	*/
}