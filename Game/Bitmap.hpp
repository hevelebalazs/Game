#pragma once

#include <windows.h>

#include "Debug.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

struct Bitmap
{
	I32 width;
	I32 height;
	U32* memory;
	// TODO: get rid of this Windows specific thing
	BITMAPINFO info;
};

#define BitmapBytesPerPixel 4

static void ResizeBitmap(Bitmap* bitmap, I32 width, I32 height)
{
	if (bitmap->memory)
	{
		delete bitmap->memory;
	}

	bitmap->width = width;
	bitmap->height = height;

	BITMAPINFOHEADER* header = &bitmap->info.bmiHeader;
	header->biSize = sizeof(*header);
	header->biWidth = bitmap->width;
	header->biHeight = -bitmap->height;
	header->biPlanes = 1;
	header->biBitCount = 32;
	header->biCompression = BI_RGB;

	I32 bitmapMemorySize = (bitmap->width * bitmap->height);
	bitmap->memory = new U32[bitmapMemorySize];
}

static V4 MakeColor(F32 red, F32 green, F32 blue)
{
	V4 color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = 1.0f;
	return color;
}

static U32 GetColorCode(V4 color)
{
	U8 alpha = (U8)(color.alpha * 255);
	U8 red   = (U8)(color.red * 255);
	U8 green = (U8)(color.green * 255);
	U8 blue  = (U8)(color.blue * 255);

	U32 colorCode = (alpha << 24) + (red << 16) + (green << 8) + (blue);
	return colorCode;
}

static V4 GetRandomColor()
{
	V4 randomColor = MakeColor(
		RandomBetween(0.0f, 1.0f),
		RandomBetween(0.0f, 1.0f),
		RandomBetween(0.0f, 1.0f)
	);

	return randomColor;
}

static U32 GetRandomColorCode()
{
	V4 color = GetRandomColor();
	I32 colorCode = GetColorCode(color);
	return colorCode;
}

static V4 MakeAlphaColor(F32 red, F32 green, F32 blue, F32 alpha)
{
	V4 color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	return color;
}

static V4 GetColorFromColorCode(U32 colorCode)
{
	V4 color = {};
	color.alpha = (F32)((colorCode & 0xff000000) >> 24) * (1.0f / 255.0f);
	color.red   = (F32)((colorCode & 0x00ff0000) >> 16) * (1.0f / 255.0f);
	color.green = (F32)((colorCode & 0x0000ff00) >>  8) * (1.0f / 255.0f);
	color.blue  = (F32)((colorCode & 0x000000ff) >>  0) * (1.0f / 255.0f);
	return color;
}

static V4 AddColors(V4 color1, V4 color2)
{
	V4 result = {};
	result.red   = (color1.red   + color2.red);
	result.green = (color1.green + color2.green);
	result.blue  = (color1.blue  + color2.blue);
	return result;
}

static V4 InterpolateColors(V4 color1, F32 ratio, V4 color2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	V4 result = {};
	result.red   = Lerp(color1.red,   ratio, color2.red);
	result.green = Lerp(color1.green, ratio, color2.green);
	result.blue  = Lerp(color1.blue,  ratio, color2.blue);
	return result;
}

static U32* GetBitmapPixelAddress(Bitmap* bitmap, I32 row, I32 col)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);
	U32* pixelAddress = bitmap->memory + row * bitmap->width + col;
	return pixelAddress;
}

static V4 GetBitmapPixelColor(Bitmap* bitmap, I32 row, I32 col)
{
	U32* address = GetBitmapPixelAddress(bitmap, row, col);
	U32 colorCode = *address;
	V4 color = GetColorFromColorCode(colorCode);
	return color;
}

static U32 GetClosestBitmapColorCode(Bitmap* bitmap, F32 heightRatio, F32 widthRatio)
{
	I32 row = Floor(heightRatio * (bitmap->height - 1));
	I32 col = Floor(widthRatio * (bitmap->width - 1));
	Assert(IsIntBetween(row, 0, bitmap->height - 1));
	Assert(IsIntBetween(col, 0, bitmap->width - 1));
	U32* address = GetBitmapPixelAddress(bitmap, row, col);
	U32 colorCode = *address;
	return colorCode;
}

static void PaintBitmapPixel(Bitmap* bitmap, I32 row, I32 col, U32 colorCode)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);

	U32* pixelAddress = GetBitmapPixelAddress(bitmap, row, col);
	*pixelAddress = colorCode;
}

struct BresenhamData 
{
	I32 row1, col1;
	I32 row2, col2;
	I32 rowAbs, colAbs;
	I32 rowAdd, colAdd;
	I32 error1, error2;
};

static BresenhamData InitBresenham(I32 row1, I32 col1, I32 row2, I32 col2)
{
	BresenhamData data = {};
	data.col1 = col1;
	data.row1 = row1;
	data.col2 = col2;
	data.row2 = row2;

	data.rowAbs = IntAbs(data.row1 - data.row2);
	data.colAbs = IntAbs(data.col1 - data.col2);

	data.colAdd = 1;
	if (data.col1 > data.col2)
	{
		data.colAdd = -1;
	}

	data.rowAdd = 1;
	if (data.row1 > data.row2)
	{
		data.rowAdd = -1;
	}

	data.error1 = 0;
	if (data.colAbs > data.rowAbs)
	{
		data.error1 = data.colAbs / 2;
	}
	else
	{
		data.error1 = -data.rowAbs / 2;
	}

	data.error2 = 0;
	return data;
}

static void AdvanceBresenham(BresenhamData* data)
{
	data->error2 = data->error1;
	if (data->error2 > -data->colAbs) 
	{
		data->error1 -= data->rowAbs;
		data->col1 += data->colAdd;
	}

	if (data->error2 < data->rowAbs) 
	{
		data->error1 += data->colAbs;
		data->row1 += data->rowAdd;
	}
}

static void DrawBitmapBresenhamLine(Bitmap* bitmap, I32 row1, I32 col1, I32 row2, I32 col2, V4 color)
{
	U32 colorCode = GetColorCode(color);
	BresenhamData data = InitBresenham(row1, col1, row2, col2);
	while (1) 
	{
		PaintBitmapPixel(bitmap, data.row1, data.col1, colorCode);

		if (data.row1 == data.row2 && data.col1 == data.col2)
		{
			break;
		}

		AdvanceBresenham(&data);
	}
}

static void FloodfillBitmap(Bitmap* bitmap, I32 row, I32 col, V4 color, MemArena* tmpArena)
{
	U32 paintColorCode = GetColorCode(color);
	U32 baseColorCode  = *GetBitmapPixelAddress(bitmap, row, col);
	Assert(paintColorCode != baseColorCode);
	I32 positionN = 0;
	I32* positions = ArenaPushArray(tmpArena, I32, 0);
	ArenaPush(tmpArena, I32, row);
	ArenaPush(tmpArena, I32, col);
	positionN++;

	// NOTE: Initial position has to be inserted twice, one for horizontal, one for vertical fill.
	ArenaPush(tmpArena, I32, row);
	ArenaPush(tmpArena, I32, col);
	positionN++;

	B32 fillHorizontally = true;
	I32 directionSwitchPosition = 1;

	PaintBitmapPixel(bitmap, row, col, paintColorCode);

	for (I32 i = 0; i < positionN; ++i) 
	{
		if (i == directionSwitchPosition) 
		{
			fillHorizontally = !fillHorizontally;
			directionSwitchPosition = positionN;
		}
		I32 row = positions[2 * i];
		I32 col = positions[2 * i + 1];

		U32* pixelStart = GetBitmapPixelAddress(bitmap, row, col);
		U32* pixel = 0;

		if (fillHorizontally) 
		{
			pixel = pixelStart;
			for (I32 left = col - 1; left >= 0; --left) 
			{
				pixel--;
				if (*pixel != baseColorCode)
				{
					break;
				}
				*pixel = paintColorCode;
				ArenaPush(tmpArena, I32, row);
				ArenaPush(tmpArena, I32, left);
				positionN++;
			}

			pixel = pixelStart;
			for (I32 right = col + 1; right < bitmap->width; ++right) 
			{
				pixel++;
				if (*pixel != baseColorCode)
				{
					break;
				}
				*pixel = paintColorCode;
				ArenaPush(tmpArena, I32, row);
				ArenaPush(tmpArena, I32, right);
				positionN++;
			}
		} 
		else 
		{
			pixel = pixelStart;
			for (I32 top = row - 1; top >= 0; --top) 
			{
				pixel -= bitmap->width;
				if (*pixel != baseColorCode)
				{
					break;
				}
				*pixel = paintColorCode;
				ArenaPush(tmpArena, I32, top);
				ArenaPush(tmpArena, I32, col);
				positionN++;
			}

			pixel = pixelStart;
			for (I32 bottom = row + 1; bottom < bitmap->height; ++bottom) 
			{
				pixel += bitmap->width;
				if (*pixel != baseColorCode)
				{
					break;
				}
				*pixel = paintColorCode;
				ArenaPush(tmpArena, I32, bottom);
				ArenaPush(tmpArena, I32, col);
				positionN++;
			}
		}
	}
	ArenaPopTo(tmpArena, positions);
}

static void FillBitmapWithColor(Bitmap* bitmap, V4 color)
{
	U32 colorCode = GetColorCode(color);
	U32* pixel = (U32*)bitmap->memory;
	for (I32 row = 0; row < bitmap->height; ++row) 
	{
		for (I32 col = 0; col < bitmap->width; ++col) 
		{
			*pixel = colorCode;
			pixel++;
		}
	}
}

static void CopyScaledRotatedBitmap(Bitmap* fromBitmap, Bitmap* toBitmap, I32 toCenterRow, I32 toCenterCol, F32 width, F32 height, F32 rotationAngle)
{
	V2 center = {};
	center.x = (F32)toCenterCol;
	center.y = (F32)toCenterRow;

	F32 inverseHeight = Invert(height);
	F32 inverseWidth  = Invert(width);

	V2 heightUnitVector = RotationVector(rotationAngle);
	V2 widthUnitVector = TurnVectorToRight(heightUnitVector);

	V2 inverseHeightVector = heightUnitVector;
	inverseHeightVector.x *= inverseHeight;
	inverseHeightVector.y *= inverseHeight;

	V2 inverseWidthVector = widthUnitVector;
	inverseWidthVector.x *= inverseWidth;
	inverseWidthVector.y *= inverseWidth;

	F32 halfHeight = (F32)height * 0.5f;
	F32 halfWidth  = (F32)width * 0.5f;
	V2 frontCenter = center + (halfHeight * heightUnitVector);
	V2 frontLeft   = frontCenter - (halfWidth * widthUnitVector);
	V2 frontRight  = frontCenter + (halfWidth * widthUnitVector);

	V2 backCenter  = center - (halfHeight * heightUnitVector);
	V2 backLeft    = backCenter - (halfWidth * widthUnitVector);
	V2 backRight   = backCenter + (halfWidth * widthUnitVector);

	I32 leftBoundary   = (I32)Min4(frontLeft.x, frontRight.x, backLeft.x, backRight.x);
	I32 rightBoundary  = (I32)Max4(frontLeft.x, frontRight.x, backLeft.x, backRight.x);
	I32 topBoundary    = (I32)Min4(frontLeft.y, frontRight.y, backLeft.y, backRight.y);
	I32 bottomBoundary = (I32)Max4(frontLeft.y, frontRight.y, backLeft.y, backRight.y);

	I32 left   = IntMax2(leftBoundary, 0);
	I32 right  = IntMin2(rightBoundary, toBitmap->width - 1);
	I32 top    = IntMax2(topBoundary, 0);
	I32 bottom = IntMin2(bottomBoundary, toBitmap->height - 1);

	for (I32 toRow = top; toRow <= bottom; ++toRow) 
	{
		for (I32 toCol = left; toCol <= right; ++toCol) 
		{
			V2 position = {};
			position.x = (F32)toCol;
			position.y = (F32)toRow;
			V2 positionVector = (position - center);
			F32 heightCoordinate = DotProduct(positionVector, inverseHeightVector);
			F32 widthCoordinate = DotProduct(positionVector, inverseWidthVector);

			F32 heightRatio = 1.0f - (0.5f + (heightCoordinate));
			F32 widthRatio  = 1.0f - (0.5f + (widthCoordinate));
			if (!IsBetween(heightRatio, 0.0f, 1.0f))
			{
				continue;
			}

			if (!IsBetween(widthRatio, 0.0f, 1.0f))
			{
				continue;
			}

			V4 fillColor = MakeColor(1.0f, 0.0f, 1.0f);
			U32 fromColorCode = GetClosestBitmapColorCode(fromBitmap, heightRatio, widthRatio);
			V4 fromColor = GetColorFromColorCode(fromColorCode);
			if (fromColor.alpha != 0.0f) 
			{
				U32* pixelAddress = GetBitmapPixelAddress(toBitmap, toRow, toCol);
				*pixelAddress = fromColorCode;
			}
		}
	}
}

static void CopyStretchedBitmap(Bitmap* fromBitmap, Bitmap* toBitmap, I32 toLeft, I32 toRight, I32 toTop, I32 toBottom)
{
	F32 toWidth  = (F32)(toRight - toLeft);
	F32 toHeight = (F32)(toBottom - toTop);

	I32 left   = ClipInt(toLeft,    0, toBitmap->width);
	I32 right  = ClipInt(toRight,  -1, toBitmap->width - 1);
	I32 top    = ClipInt(toTop,     0, toBitmap->height);
	I32 bottom = ClipInt(toBottom, -1, toBitmap->height - 1);

	
	F32 fromHeightStart = ((F32)fromBitmap->height - 1.0f) * ((F32)top - toTop) / toHeight;
	F32 fromHeightAdd   = ((F32)fromBitmap->height - 1.0f) / toHeight;
	F32 fromWidthStart  = ((F32)fromBitmap->width  - 1.0f) * ((F32)left - toLeft) / toWidth;
	F32 fromWidthAdd    = ((F32)fromBitmap->width  - 1.0f) / toWidth;

	F32 fromHeight = fromHeightStart;
	F32 fromWidth = fromWidthStart;

	if (top <= bottom && left <= right) 
	{
		U32* fromRowFirstPixelAddress = GetBitmapPixelAddress(fromBitmap, (I32)fromHeight, (I32)fromWidth);
		U32* toRowFirstPixelAddress = GetBitmapPixelAddress(toBitmap, top, left);

		I32 prevFromRow = (I32)fromHeight;
		for (I32 toRow = top; toRow <= bottom; ++toRow) 
		{
			I32 fromRow = (I32)fromHeight;
			fromWidth = fromWidthStart;
			I32 prevFromCol = (I32)fromWidth;
			U32* fromPixelAddress = fromRowFirstPixelAddress;		
			U32* toPixelAddress = toRowFirstPixelAddress;
			for (I32 toCol = left; toCol <= right; ++toCol) 
			{
				I32 fromCol = (I32)fromWidth;
				fromPixelAddress += (fromCol - prevFromCol);
				prevFromCol = fromCol;

				toPixelAddress++;

				*toPixelAddress = *fromPixelAddress;
				fromWidth += fromWidthAdd;
			}

			fromHeight += fromHeightAdd;
			fromRowFirstPixelAddress += fromBitmap->width * (fromRow - prevFromRow);
			prevFromRow = fromRow;

			toRowFirstPixelAddress += toBitmap->width;
		}
	}
}

static void DrawBitmapPolyOutline(Bitmap* bitmap, I32 polyN, I32* polyColRow, V4 color)
{
	for (I32 i = 0; i < polyN; ++i) 
	{
		I32 next = 0;
		if (i < polyN - 1)
		{
			next = i + 1;
		}
		else
		{
			next = 0;
		}
		I32 row1 = polyColRow[2 * i];
		I32 col1 = polyColRow[2 * i + 1];
		I32 row2 = polyColRow[2 * next];
		I32 col2 = polyColRow[2 * next + 1];
		DrawBitmapBresenhamLine(bitmap, row1, col1, row2, col2, color);
	}
}

static void DrawBitmapRect(Bitmap* bitmap, I32 left, I32 right, I32 top, I32 bottom, V4 color)
{
	U32 colorCode = GetColorCode(color);
	left = ClipInt(left, 0, bitmap->width - 1);
	right = ClipInt(right, 0, bitmap->width - 1);
	top = ClipInt(top, 0, bitmap->height - 1);
	bottom = ClipInt(bottom, 0, bitmap->height - 1);
	for (I32 col = left; col <= right; ++col)
	{
		for (I32 row = top; row <= bottom; ++row)
		{
			U32* pixelAddress = GetBitmapPixelAddress(bitmap, row, col);
			*pixelAddress = colorCode;
		}
	}
}