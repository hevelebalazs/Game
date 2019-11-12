#pragma once

#include <windows.h>

#include "Debug.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "String.hpp"
#include "Text.hpp"
#include "Type.hpp"

struct Bitmap
{
	Int32 width;
	Int32 height;
	UInt32 *memory;
};

#define BitmapBytesPerPixel 4

static BITMAPINFO
func GetBitmapInfo(Bitmap *bitmap)
{
	BITMAPINFO info = {};
	BITMAPINFOHEADER* header = &info.bmiHeader;
	header->biSize = sizeof(*header);
	header->biWidth = bitmap->width;
	header->biHeight = -bitmap->height;
	header->biPlanes = 1;
	header->biBitCount = 32;
	header->biCompression = BI_RGB;
	return info;
}

static void
func ResizeBitmap(Bitmap *bitmap, Int32 width, Int32 height)
{
	if(bitmap->memory)
	{
		delete bitmap->memory;
	}
    
	bitmap->width = width;
	bitmap->height = height;
    
	Int32 bitmapMemorySize = (bitmap->width * bitmap->height);
	bitmap->memory = new UInt32[bitmapMemorySize];
}

static Vec4
func MakeColor(Real32 red, Real32 green, Real32 blue)
{
	Vec4 color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = 1.0f;
	return color;
}

static UInt32
func GetColorCode(Vec4 color)
{
	UInt8 alpha = (UInt8)(color.alpha * 255);
	UInt8 red   = (UInt8)(color.red * 255);
	UInt8 green = (UInt8)(color.green * 255);
	UInt8 blue  = (UInt8)(color.blue * 255);
    
	UInt32 colorCode = (alpha << 24) + (red << 16) + (green << 8) + (blue);
	return colorCode;
}

static UInt32
func MakeColorCode(Real32 red, Real32 green, Real32 blue)
{
	Vec4 color = MakeColor(red, green, blue);
	UInt32 colorCode = GetColorCode(color);
	return colorCode;
}

static Vec4
func GetRandomColor()
{
	Vec4 randomColor = MakeColor(RandomBetween(0.0f, 1.0f),
								 RandomBetween(0.0f, 1.0f),
								 RandomBetween(0.0f, 1.0f)
                                 );
	return randomColor;
}

static UInt32
func GetRandomColorCode()
{
	Vec4 color = GetRandomColor ();
	Int32 colorCode = GetColorCode(color);
	return colorCode;
}

static Vec4
func MakeAlphaColor(Real32 red, Real32 green, Real32 blue, Real32 alpha)
{
	Vec4 color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	return color;
}

static UInt32
func MakeAlphaColorCode(Real32 red, Real32 green, Real32 blue, Real32 alpha)
{
	Vec4 color = MakeAlphaColor(red, green, blue, alpha);
	UInt32 colorCode = GetColorCode(color);
	return colorCode;
}

static Vec4
func GetColorFromColorCode(UInt32 colorCode)
{
	Vec4 color = {};
	color.alpha = (Real32)((colorCode & 0xff000000) >> 24) * (1.0f / 255.0f);
	color.red   = (Real32)((colorCode & 0x00ff0000) >> 16) * (1.0f / 255.0f);
	color.green = (Real32)((colorCode & 0x0000ff00) >>  8) * (1.0f / 255.0f);
	color.blue  = (Real32)((colorCode & 0x000000ff) >>  0) * (1.0f / 255.0f);
	return color;
}

static Vec4
func AddColors(Vec4 color1, Vec4 color2)
{
	Vec4 result = {};
	result.red   = (color1.red   + color2.red);
	result.green = (color1.green + color2.green);
	result.blue  = (color1.blue  + color2.blue);
	return result;
}

static Vec4
func MixColors(Vec4 baseColor, Vec4 colorToAdd)
{
	Vec4 result = {};
	Assert(IsBetween(baseColor.alpha, 0.0f, 1.0f));
	Assert(IsBetween(colorToAdd.alpha, 0.0f, 1.0f));
	result.alpha = baseColor.alpha + (1.0f - baseColor.alpha) * colorToAdd.alpha;
	Assert(IsBetween(result.alpha, 0.0f, 1.0f));
	result.red = colorToAdd.red + (baseColor.red * (1.0f - colorToAdd.alpha));
	result.green = colorToAdd.green + (baseColor.green * (1.0f - colorToAdd.alpha));
	result.blue = colorToAdd.blue + (baseColor.blue * (1.0f - colorToAdd.alpha));
	return result;
}

static UInt32
func MixColorCodes(UInt32 baseColorCode, UInt32 colorCodeToAdd)
{
	Vec4 baseColor = GetColorFromColorCode(baseColorCode);
	Vec4 colorToAdd = GetColorFromColorCode(colorCodeToAdd);
	Vec4 resultColor = MixColors(baseColor, colorToAdd);
	UInt32 resultColorCode = GetColorCode(resultColor);
	return resultColorCode;
}

static Vec4
func InterpolateColors(Vec4 color1, Real32 ratio, Vec4 color2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	Vec4 result = {};
	result.red   = Lerp(color1.red,   ratio, color2.red);
	result.green = Lerp(color1.green, ratio, color2.green);
	result.blue  = Lerp(color1.blue,  ratio, color2.blue);
	return result;
}

static UInt32 *
func GetBitmapPixelAddress(Bitmap *bitmap, Int32 row, Int32 col)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);
	UInt32 *pixelAddress = bitmap->memory + row * bitmap->width + col;
	return pixelAddress;
}

static Vec4
func GetBitmapPixelColor(Bitmap *bitmap, Int32 row, Int32 col)
{
	UInt32 *address = GetBitmapPixelAddress(bitmap, row, col);
	UInt32 colorCode = *address;
	Vec4 color = GetColorFromColorCode(colorCode);
	return color;
}

static UInt32
func GetClosestBitmapColorCode(Bitmap *bitmap, Real32 heightRatio, Real32 widthRatio)
{
	Int32 row = Floor(heightRatio * (bitmap->height - 1));
	Int32 col = Floor(widthRatio * (bitmap->width - 1));
	Assert(IsIntBetween(row, 0, bitmap->height - 1));
	Assert(IsIntBetween(col, 0, bitmap->width - 1));
	UInt32 *address = GetBitmapPixelAddress(bitmap, row, col);
	UInt32 colorCode = *address;
	return colorCode;
}

static Bool32
func IsValidBitmapPixel(Bitmap *bitmap, Int32 row, Int32 col)
{
	Bool32 isValid = (IsIntBetween(row, 0, bitmap->height - 1) && 
					  IsIntBetween(col, 0, bitmap->width - 1));
	return isValid;
}

static void
func PaintBitmapPixel(Bitmap *bitmap, Int32 row, Int32 col, UInt32 colorCode)
{
	Assert(IsValidBitmapPixel(bitmap, row, col));
    
	UInt32 *pixelAddress = GetBitmapPixelAddress(bitmap, row, col);
	*pixelAddress = colorCode;
}

static void
func MixBitmapPixel(Bitmap *bitmap, Int32 row, Int32 col, Vec4 newColor)
{
	Assert(IsValidBitmapPixel(bitmap, row, col));
	Vec4 oldColor = GetBitmapPixelColor(bitmap, row, col);
	Vec4 mixedColor = MixColors(oldColor, newColor);
	PaintBitmapPixel(bitmap, row, col, GetColorCode(mixedColor));
}

struct BresenhamData 
{
	Int32 row1, col1;
	Int32 row2, col2;
	Int32 rowAbs, colAbs;
	Int32 rowAdd, colAdd;
	Int32 error1, error2;
};

static BresenhamData
func InitBresenham(Int32 row1, Int32 col1, Int32 row2, Int32 col2)
{
	BresenhamData data = {};
	data.col1 = col1;
	data.row1 = row1;
	data.col2 = col2;
	data.row2 = row2;
    
	data.rowAbs = IntAbs(data.row1 - data.row2);
	data.colAbs = IntAbs(data.col1 - data.col2);
    
	data.colAdd = 1;
	if(data.col1 > data.col2)
	{
		data.colAdd = -1;
	}
    
	data.rowAdd = 1;
	if(data.row1 > data.row2)
	{
		data.rowAdd = -1;
	}
    
	data.error1 = 0;
	if(data.colAbs > data.rowAbs)
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

static void
func AdvanceBresenham(BresenhamData *data)
{
	data->error2 = data->error1;
	if(data->error2 > -data->colAbs) 
	{
		data->error1 -= data->rowAbs;
		data->col1 += data->colAdd;
	}
    
	if(data->error2 < data->rowAbs) 
	{
		data->error1 += data->colAbs;
		data->row1 += data->rowAdd;
	}
}

static void
func DrawBitmapBresenhamLine(Bitmap *bitmap, Int32 row1, Int32 col1, Int32 row2, Int32 col2, Vec4 color)
{
	UInt32 colorCode = GetColorCode(color);
	BresenhamData data = InitBresenham(row1, col1, row2, col2);
	while(1) 
	{
		if(IsValidBitmapPixel(bitmap, data.row1, data.col1))
		{
			PaintBitmapPixel(bitmap, data.row1, data.col1, colorCode);
		}
        
		if (data.row1 == data.row2 && data.col1 == data.col2)
		{
			break;
		}
        
		AdvanceBresenham(&data);
	}
}

static void
func FloodfillBitmap(Bitmap *bitmap, Int32 row, Int32 col, Vec4 color, MemArena* tmpArena)
{
	UInt32 paintColorCode = GetColorCode(color);
	UInt32 baseColorCode  = *GetBitmapPixelAddress(bitmap, row, col);
	Assert(paintColorCode != baseColorCode);
	Int32 positionN = 0;
	Int32 *positions = ArenaAllocArray(tmpArena, Int32, 0);
	ArenaPushVar(tmpArena, row);
	ArenaPushVar(tmpArena, col);
	positionN++;
    
	// NOTE: Initial position has to be inserted twice, one for horizontal, one for vertical fill.
	ArenaPushVar(tmpArena, row);
	ArenaPushVar(tmpArena, col);
	positionN++;
    
	Bool32 fillHorizontally = true;
	Int32 directionSwitchPosition = 1;
    
	PaintBitmapPixel(bitmap, row, col, paintColorCode);
    
	for(Int32 i = 0; i < positionN; ++i) 
	{
		if(i == directionSwitchPosition) 
		{
			fillHorizontally = !fillHorizontally;
			directionSwitchPosition = positionN;
		}
		Int32 row = positions[2 * i];
		Int32 col = positions[2 * i + 1];
        
		UInt32 *pixelStart = GetBitmapPixelAddress(bitmap, row, col);
		UInt32 *pixel = 0;
        
		if(fillHorizontally) 
		{
			pixel = pixelStart;
			for(Int32 left = col - 1; left >= 0; --left) 
			{
				pixel--;
				if(*pixel != baseColorCode)
				{
					break;
				}
				*pixel = paintColorCode;
				ArenaPushVar(tmpArena, row);
				ArenaPushVar(tmpArena, left);
				positionN++;
			}
            
			pixel = pixelStart;
			for(Int32 right = col + 1; right < bitmap->width; ++right) 
			{
				pixel++;
				if(*pixel != baseColorCode)
				{
					break;
				}
				*pixel = paintColorCode;
				ArenaPushVar(tmpArena, row);
				ArenaPushVar(tmpArena, right);
				positionN++;
			}
		} 
		else 
		{
			pixel = pixelStart;
			for(Int32 top = row - 1; top >= 0; --top) 
			{
				pixel -= bitmap->width;
				if(*pixel != baseColorCode)
				{
					break;
				}
				*pixel = paintColorCode;
				ArenaPushVar(tmpArena, top);
				ArenaPushVar(tmpArena, col);
				positionN++;
			}
            
			pixel = pixelStart;
			for(Int32 bottom = row + 1; bottom < bitmap->height; ++bottom) 
			{
				pixel += bitmap->width;
				if(*pixel != baseColorCode)
				{
					break;
				}
				*pixel = paintColorCode;
				ArenaPushVar(tmpArena, bottom);
				ArenaPushVar(tmpArena, col);
				positionN++;
			}
		}
	}
	ArenaPopTo(tmpArena, positions);
}

static void
func FillBitmapWithColor(Bitmap *bitmap, Vec4 color)
{
	UInt32 colorCode = GetColorCode(color);
	UInt32 *pixel = bitmap->memory;
	for(Int32 row = 0; row < bitmap->height; ++row) 
	{
		for(Int32 col = 0; col < bitmap->width; ++col) 
		{
			*pixel = colorCode;
			pixel++;
		}
	}
}

static void
func CopyScaledRotatedBitmap(Bitmap *fromBitmap, Bitmap *toBitmap, Int32 toCenterRow, Int32 toCenterCol, Real32 width, Real32 height, Real32 rotationAngle)
{
	Vec2 center = {};
	center.x = (Real32)toCenterCol;
	center.y = (Real32)toCenterRow;
    
	Real32 inverseHeight = Invert(height);
	Real32 inverseWidth  = Invert(width);
    
	Vec2 heightUnitVector = RotationVector(rotationAngle);
	Vec2 widthUnitVector = TurnVectorToRight(heightUnitVector);
    
	Vec2 inverseHeightVector = heightUnitVector;
	inverseHeightVector.x *= inverseHeight;
	inverseHeightVector.y *= inverseHeight;
    
	Vec2 inverseWidthVector = widthUnitVector;
	inverseWidthVector.x *= inverseWidth;
	inverseWidthVector.y *= inverseWidth;
    
	Real32 halfHeight = (Real32)height * 0.5f;
	Real32 halfWidth  = (Real32)width * 0.5f;
	Vec2 frontCenter = center + (halfHeight * heightUnitVector);
	Vec2 frontLeft   = frontCenter - (halfWidth * widthUnitVector);
	Vec2 frontRight  = frontCenter + (halfWidth * widthUnitVector);
    
	Vec2 backCenter  = center - (halfHeight * heightUnitVector);
	Vec2 backLeft    = backCenter - (halfWidth * widthUnitVector);
	Vec2 backRight   = backCenter + (halfWidth * widthUnitVector);
    
	Int32 leftBoundary   = (Int32)Min4(frontLeft.x, frontRight.x, backLeft.x, backRight.x);
	Int32 rightBoundary  = (Int32)Max4(frontLeft.x, frontRight.x, backLeft.x, backRight.x);
	Int32 topBoundary    = (Int32)Min4(frontLeft.y, frontRight.y, backLeft.y, backRight.y);
	Int32 bottomBoundary = (Int32)Max4(frontLeft.y, frontRight.y, backLeft.y, backRight.y);
    
	Int32 left   = IntMax2(leftBoundary, 0);
	Int32 right  = IntMin2(rightBoundary, toBitmap->width - 1);
	Int32 top    = IntMax2(topBoundary, 0);
	Int32 bottom = IntMin2(bottomBoundary, toBitmap->height - 1);
    
	for(Int32 toRow = top; toRow <= bottom; ++toRow) 
	{
		for(Int32 toCol = left; toCol <= right; ++toCol) 
		{
			Vec2 position = {};
			position.x = (Real32)toCol;
			position.y = (Real32)toRow;
			Vec2 positionVector = (position - center);
			Real32 heightCoordinate = DotProduct(positionVector, inverseHeightVector);
			Real32 widthCoordinate = DotProduct(positionVector, inverseWidthVector);
            
			Real32 heightRatio = 1.0f - (0.5f + (heightCoordinate));
			Real32 widthRatio  = 1.0f - (0.5f + (widthCoordinate));
			if(!IsBetween(heightRatio, 0.0f, 1.0f))
			{
				continue;
			}
            
			if(!IsBetween(widthRatio, 0.0f, 1.0f))
			{
				continue;
			}
            
			Vec4 fillColor = MakeColor(1.0f, 0.0f, 1.0f);
			UInt32 fromColorCode = GetClosestBitmapColorCode(fromBitmap, heightRatio, widthRatio);
			Vec4 fromColor = GetColorFromColorCode(fromColorCode);
			if(fromColor.alpha != 0.0f) 
			{
				UInt32 *pixelAddress = GetBitmapPixelAddress(toBitmap, toRow, toCol);
				*pixelAddress = fromColorCode;
			}
		}
	}
}

static void
func CopyBitmap(Bitmap *fromBitmap, Bitmap *toBitmap, Int32 toLeft, Int32 toTop)
{
	Int32 toRight = toLeft + fromBitmap->width - 1;
	Int32 toBottom = toTop + fromBitmap->height - 1;
    
	Int32 fromLeft = 0;
	if(toLeft < 0)
	{
		fromLeft = -toLeft;
	}
    
	Int32 fromRight = fromBitmap->width - 1;
	if(toRight > toBitmap->width - 1)
	{
		fromRight -= (toRight - (toBitmap->width - 1));
	}
    
	Int32 fromTop = 0;
	if(toTop < 0)
	{
		fromTop = -toTop;
	}
    
	Int32 fromBottom = fromBitmap->height - 1;
	if(toBottom > toBitmap->height - 1)
	{
		fromBottom -= (toBottom - (toBitmap->height - 1));
	}
    
	for(Int32 fromRow = fromTop; fromRow <= fromBottom; ++fromRow)
	{
		for(Int32 fromCol = fromLeft; fromCol <= fromRight; ++fromCol)
		{
			Int32 toRow = toTop + fromRow;
			Int32 toCol = toLeft + fromCol;
            
			Vec4 fromColor = GetBitmapPixelColor(fromBitmap, fromRow, fromCol);
			Vec4 toColor = GetBitmapPixelColor(toBitmap, toRow, toCol);
			Vec4 mixedColor = MixColors(toColor, fromColor);
			*GetBitmapPixelAddress(toBitmap, toRow, toCol) = GetColorCode(mixedColor);
		}
	}
}

static void
func CopyStretchedBitmap(Bitmap *fromBitmap, Bitmap *toBitmap, Int32 toLeft, Int32 toRight, Int32 toTop, Int32 toBottom)
{
	Real32 toWidth  = (Real32)(toRight - toLeft);
	Real32 toHeight = (Real32)(toBottom - toTop);
    
	Int32 left   = ClipInt(toLeft,    0, toBitmap->width);
	Int32 right  = ClipInt(toRight,  -1, toBitmap->width - 1);
	Int32 top    = ClipInt(toTop,     0, toBitmap->height);
	Int32 bottom = ClipInt(toBottom, -1, toBitmap->height - 1);
    
	Real32 fromHeightStart = ((Real32)fromBitmap->height - 1.0f) * ((Real32)top - toTop) / toHeight;
	Real32 fromHeightAdd   = ((Real32)fromBitmap->height - 1.0f) / toHeight;
	Real32 fromWidthStart  = ((Real32)fromBitmap->width  - 1.0f) * ((Real32)left - toLeft) / toWidth;
	Real32 fromWidthAdd    = ((Real32)fromBitmap->width  - 1.0f) / toWidth;
    
	Real32 fromHeight = fromHeightStart;
	Real32 fromWidth = fromWidthStart;
    
	if(top <= bottom && left <= right) 
	{
		UInt32 *fromRowFirstPixelAddress = GetBitmapPixelAddress(fromBitmap, (Int32)fromHeight, (Int32)fromWidth);
		UInt32 *toRowFirstPixelAddress = GetBitmapPixelAddress(toBitmap, top, left);
        
		Int32 prevFromRow = (Int32)fromHeight;
		for(Int32 toRow = top; toRow <= bottom; ++toRow) 
		{
			Int32 fromRow = (Int32)fromHeight;
			fromWidth = fromWidthStart;
			Int32 prevFromCol = (Int32)fromWidth;
			UInt32* fromPixelAddress = fromRowFirstPixelAddress;		
			UInt32* toPixelAddress = toRowFirstPixelAddress;
			for(Int32 toCol = left; toCol <= right; ++toCol) 
			{
				Int32 fromCol = (Int32)fromWidth;
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

static void
func DrawBitmapPolyOutline(Bitmap *bitmap, Int32 polyN, Int32 *polyColRow, Vec4 color)
{
	for(Int32 i = 0; i < polyN; i++) 
	{
		Int32 next = 0;
		if(i < polyN - 1)
		{
			next = i + 1;
		}
		else
		{
			next = 0;
		}
		Int32 row1 = polyColRow[2 * i];
		Int32 col1 = polyColRow[2 * i + 1];
		Int32 row2 = polyColRow[2 * next];
		Int32 col2 = polyColRow[2 * next + 1];
		DrawBitmapBresenhamLine(bitmap, row1, col1, row2, col2, color);
	}
}

static IntRect
func GetBitmapBounds(Bitmap *bitmap)
{
	IntRect bounds = {};
	bounds.left   = 0;
	bounds.right  = (bitmap->width - 1);
	bounds.top    = 0;
	bounds.bottom = (bitmap->height - 1);
	return bounds;
}

static void
func DrawBitmapRect(Bitmap *bitmap, IntRect rect, Vec4 color)
{
	IntRect bitmapBounds = GetBitmapBounds(bitmap);
	IntRect drawRect = GetIntRectIntersection(rect, bitmapBounds);

	UInt32 colorCode = GetColorCode(color);
	for(Int32 col = drawRect.left; col <= drawRect.right; col++)
	{
		for(Int32 row = drawRect.top; row <= drawRect.bottom; row++)
		{
			UInt32 *pixelAddress = GetBitmapPixelAddress(bitmap, row, col);
			*pixelAddress = colorCode;
		}
	}
}

static void
func DrawBitmapRectOutline(Bitmap *bitmap, IntRect rect, Vec4 color)
{
	DrawBitmapBresenhamLine(bitmap, rect.top, rect.left, rect.top, rect.right, color);
	DrawBitmapBresenhamLine(bitmap, rect.top, rect.right, rect.bottom, rect.right, color);
	DrawBitmapBresenhamLine(bitmap, rect.bottom, rect.right, rect.bottom, rect.left, color);
	DrawBitmapBresenhamLine(bitmap, rect.bottom, rect.left, rect.top, rect.left, color);
}

static void
func DrawBitmapGlyph(Bitmap *bitmap, Glyph *glyph, Int32 x, Int32 baseLineY, Vec4 color)
{
	Int32 startCol = x + (Int32)glyph->offsetX;
	Int32 startRow = baseLineY + (Int32)glyph->offsetY;
    
	for(Int32 row = 0; row < 32; row++)
	{
		for(Int32 col = 0; col < 32; col++)
		{
			UInt8 alphaValue = glyph->alpha[row][col];
			if(alphaValue == 0)
			{
				continue;
			}
            
			Real32 alpha = (Real32)alphaValue / 255.0f;
			Vec4 newColor = {};
			newColor.red   = color.red * alpha;
			newColor.green = color.green * alpha;
			newColor.blue  = color.blue * alpha;
			newColor.alpha = alpha;
            
			Int32 toRow = startRow + row;
			Int32 toCol = startCol + col;
            
			if(IsValidBitmapPixel(bitmap, toRow, toCol))
			{
				MixBitmapPixel(bitmap, toRow, toCol, newColor);
			}
		}
	}
}

static void
func DrawBitmapTextLine(Bitmap *bitmap, Int8 *text, GlyphData *glyphData, Int32 left, Int32 baseLineY, Vec4 color)
{
	Assert(glyphData);
	Real32 textX = (Real32)left;
	Real32 textY = (Real32)baseLineY;
    
	for(Int32 i = 0; text[i]; i++)
	{
		UInt8 c = text[i];
		if(c == '\n')
		{
			continue;
		}
        
		Glyph *glyph = &glyphData->glyphs[c];
        
		Real32 right = textX + glyph->advanceX;
		UInt8 nextC = text[i + 1];
		if(nextC > 0)
		{
			right += glyphData->kerningTable[c][nextC];
		}
        
		DrawBitmapGlyph(bitmap, glyph, (Int32)textX, (Int32)textY, color);
		textX = right;
	}
}

static Real32
func GetTextPixelWidth(Int8 *text, GlyphData *glyphData)
{
	Assert(glyphData != 0);
    
	Real32 width = 0.0f;
	for(Int32 i = 0; text[i]; i++)
	{
		UInt8 c = text[i];
		Glyph *glyph = &glyphData->glyphs[c];
		width += glyph->advanceX;
		
		UInt8 nextC = text[i + 1];
		if(nextC > 0)
		{
			width += glyphData->kerningTable[c][nextC];
		}
	}
    
	return width;
}

static void
func DrawBitmapTextLineCentered(Bitmap *bitmap, Int8 *text, GlyphData *glyphData, IntRect rect, Vec4 color)
{
	Int32 textHeight = TextHeightInPixels;
	Int32 textWidth = (Int32)GetTextPixelWidth(text, glyphData);
	Int32 textLeft = (rect.left + rect.right) / 2 - textWidth / 2;
	Int32 textTop = (rect.bottom + rect.top) / 2 - textHeight / 2;
	Int32 textBaseLineY = textTop + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyphData, textLeft, textBaseLineY, color);
}

static void
func DrawBitmapTextLineTopLeft(Bitmap *bitmap, Int8 *text, GlyphData *glyphData, Int32 left, Int32 top, Vec4 color)
{
	Int32 baseLineY = top + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyphData, left, baseLineY, color);
}

static void
func DrawBitmapTextLineTopRight(Bitmap *bitmap, Int8 *text, GlyphData *glyphData, Int32 right, Int32 top, Vec4 color)
{
	Int32 textWidth = (Int32)GetTextPixelWidth(text, glyphData);
	Int32 left = right - textWidth;
	Int32 baseLineY = top + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyphData, left, baseLineY, color);
}

#define TooltipWidth 300
#define TooltipPadding 3
#define TooltipTopPadding 5

static Int32
func GetTooltipHeight(Int32 lineN)
{
	Int32 height = TooltipTopPadding + lineN * (TextHeightInPixels) + TooltipPadding;
	return height;
}

static void
func DrawBitmapStringTooltip(Bitmap *bitmap, String string, GlyphData *glyphData, Int32 top, Int32 left)
{
	Int32 lineN = GetNumberOfLines(string);
    
	Assert(glyphData != 0);

	IntRect rect = {};
	rect.left = left;
	rect.top = top;
    
	rect.right = left + TooltipWidth;
	Int32 height = GetTooltipHeight(lineN);
	rect.bottom = top + height;
    
	Vec4 tooltipColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, tooltipColor);
    
	Vec4 tooltipBorderColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRectOutline(bitmap, rect, tooltipBorderColor);
    
	Vec4 titleColor = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 normalColor = MakeColor(1.0f, 1.0f, 1.0f);
	Int32 baseLineY = top + TooltipTopPadding + TextPixelsAboveBaseLine;
	Int32 textLeft = left + TooltipPadding;
    
	Real32 textX = (Real32)textLeft;
	Int32 lineIndex = 0;
    
	for(Int32 i = 0; i < string.usedSize; ++i)
	{
		UInt8 c = string.buffer[i];
		Assert(c != 0);
        
		if(c == '\n')
		{
			lineIndex++;
			baseLineY += TextHeightInPixels;
			textX = (Real32)textLeft;
		}
		else
		{
			Glyph *glyph = &glyphData->glyphs[c];
            
			Real32 right = textX + glyph->advanceX;
			UInt8 nextC = string.buffer[i + 1];
			if(nextC > 0)
			{
				right += glyphData->kerningTable[c][nextC];
			}
            
			Vec4 color = (lineIndex == 0) ? titleColor : normalColor;
			DrawBitmapGlyph(bitmap, glyph, Int32(textX), Int32(baseLineY), color);
			textX = right;
		}
        
		Assert(textX <= rect.right);
	}
}

static void
func DrawBitmapStringTooltipBottom(Bitmap *bitmap, String string, GlyphData *glyphData, Int32 bottom, Int32 left)
{
	Int32 lineN = GetNumberOfLines(string);
	Int32 top = bottom - GetTooltipHeight(lineN);
	DrawBitmapStringTooltip(bitmap, string, glyphData, top, left);
}

static void
func DrawBitmapStringTooltipBottomRight(Bitmap *bitmap, String string, GlyphData *glyphData, Int32 bottom, Int32 right)
{
	Int32 lineN = GetNumberOfLines(string);
	Int32 top = bottom - GetTooltipHeight(lineN);
	Int32 left = right - TooltipWidth;
	DrawBitmapStringTooltip(bitmap, string, glyphData, top, left);
}

static void
func DrawBitmapTooltip(Bitmap *bitmap, Int8 **lines, Int32 lineN, GlyphData *glyphData, Int32 top, Int32 left)
{
	Assert(glyphData != 0);
    
	IntRect rect = {};
	rect.left = left;
	rect.top = top;
	rect.right = left + TooltipWidth;
	Int32 height = GetTooltipHeight(lineN);
	rect.bottom = top + height;
    
	Vec4 tooltipColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, tooltipColor);
    
	Vec4 tooltipBorderColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRectOutline(bitmap, rect, tooltipBorderColor);
    
	Vec4 titleColor = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 normalColor = MakeColor(1.0f, 1.0f, 1.0f);
	Int32 baseLineY = top + TooltipTopPadding + TextPixelsAboveBaseLine;
	Int32 textLeft = left + TooltipPadding;
	for(Int32 i = 0; i < lineN; i++)
	{
		Vec4 color = (i == 0) ? titleColor : normalColor;
		Int8* line = lines[i];
		DrawBitmapTextLine(bitmap, line, glyphData, textLeft, baseLineY, color);
		baseLineY += TextHeightInPixels;
	}
}

static void
func DrawBitmapTooltipBottom(Bitmap *bitmap, Int8 **lines, Int32 lineN, GlyphData *glyphData, Int32 bottom, Int32 left)
{
	Int32 top = bottom - GetTooltipHeight(lineN);
	DrawBitmapTooltip(bitmap, lines, lineN, glyphData, top, left);
}

static void
func DrawBitmapTooltipBottomRight(Bitmap *bitmap, Int8 **lines, Int32 lineN, GlyphData *glyphData, Int32 bottom, Int32 right)
{
	Int32 top = bottom - GetTooltipHeight(lineN);
	Int32 left = right - TooltipWidth;
	DrawBitmapTooltip(bitmap, lines, lineN, glyphData, top, left);
}