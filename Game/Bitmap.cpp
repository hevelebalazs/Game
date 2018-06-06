#include "Bitmap.hpp"
#include "Debug.hpp"
#include "Geometry.hpp"
#include "Math.hpp"

void ResizeBitmap(Bitmap* bitmap, int width, int height)
{
	if (bitmap->memory)
		delete bitmap->memory;

	bitmap->width = width;
	bitmap->height = height;

	BITMAPINFOHEADER* header = &bitmap->info.bmiHeader;
	header->biSize = sizeof(*header);
	header->biWidth = bitmap->width;
	header->biHeight = -bitmap->height;
	header->biPlanes = 1;
	header->biBitCount = 32;
	header->biCompression = BI_RGB;

	int bytesPerPixel = 4;
	int bitmapMemorySize = (bitmap->width * bitmap->height) * bytesPerPixel;

	bitmap->memory = (void*)(new char[bitmapMemorySize]);
}

Color GetRandomColor() {
	Color randomColor = {
		RandomBetween(0.0f, 1.0f),
		RandomBetween(0.0f, 1.0f),
		RandomBetween(0.0f, 1.0f)
	};

	return randomColor;
}

Color GetColor(float red, float green, float blue)
{
	Color color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = 1.0f;
	return color;
}

Color GetAlphaColor(float red, float green, float blue, float alpha)
{
	Color color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	return color;
}

unsigned int GetColorCode(Color color)
{
	unsigned char alpha = (unsigned char)(color.alpha * 255);
	unsigned char red   = (unsigned char)(color.red * 255);
	unsigned char green = (unsigned char)(color.green * 255);
	unsigned char blue  = (unsigned char)(color.blue * 255);

	unsigned int colorCode = (alpha << 24) + (red << 16) + (green << 8) + (blue);

	return colorCode;
}

Color GetColorFromColorCode(unsigned int colorCode)
{
	Color color = {};
	color.alpha = (float)((colorCode & 0xff000000) >> 24) * (1.0f / 255.0f);
	color.red   = (float)((colorCode & 0x00ff0000) >> 16) * (1.0f / 255.0f);
	color.green = (float)((colorCode & 0x0000ff00) >>  8) * (1.0f / 255.0f);
	color.blue  = (float)((colorCode & 0x000000ff) >>  0) * (1.0f / 255.0f);
	return color;
}

Color AddColors(Color color1, Color color2)
{
	Color result = {};
	result.red   = (color1.red   + color2.red);
	result.green = (color1.green + color2.green);
	result.blue  = (color1.blue  + color2.blue);
	return result;
}

Color InterpolateColors(Color color1, float ratio, Color color2)
{
	Color result = {};
	result.red   = Lerp(color1.red,   ratio, color2.red);
	result.green = Lerp(color1.green, ratio, color2.green);
	result.blue  = Lerp(color1.blue,  ratio, color2.blue);
	return result;
}

static unsigned int* GetBitmapPixelAddress(Bitmap* bitmap, int row, int col)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);
	unsigned int* pixelAddress = (unsigned int*)bitmap->memory + row * bitmap->width + col;
	return pixelAddress;
}

static unsigned int GetClosestBitmapColorCode(Bitmap* bitmap, float heightRatio, float widthRatio)
{
	Assert(IsBetween(heightRatio, 0.0f, 1.0f));
	Assert(IsBetween(widthRatio,  0.0f, 1.0f));
	int row = Floor(heightRatio * bitmap->height);
	int col = Floor(widthRatio * bitmap->width);
	row = ClipInt(row, 0, bitmap->height - 1);
	col = ClipInt(col, 0, bitmap->width - 1);
	unsigned int* address = GetBitmapPixelAddress(bitmap, row, col);
	unsigned int colorCode = *address;
	return colorCode;
}

static void PaintBitmapPixel(Bitmap* bitmap, int row, int col, unsigned int colorCode)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);

	unsigned int* pixelAddress = GetBitmapPixelAddress(bitmap, row, col);
	*pixelAddress = colorCode;
}

struct BresenhamData {
	int row1, col1;
	int row2, col2;
	int rowAbs, colAbs;
	int rowAdd, colAdd;
	int error1, error2;
};

static BresenhamData InitBresenham(int row1, int col1, int row2, int col2)
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
		data.colAdd = -1;

	data.rowAdd = 1;
	if (data.row1 > data.row2)
		data.rowAdd = -1;

	data.error1 = 0;
	if (data.colAbs > data.rowAbs)
		data.error1 = data.colAbs / 2;
	else
		data.error1 = -data.rowAbs / 2;

	data.error2 = 0;
	return data;
}

static void AdvanceBresenham(BresenhamData* data)
{
	data->error2 = data->error1;
	if (data->error2 > -data->colAbs) {
		data->error1 -= data->rowAbs;
		data->col1 += data->colAdd;
	}
	if (data->error2 < data->rowAbs) {
		data->error1 += data->colAbs;
		data->row1 += data->rowAdd;
	}
}

static void DrawBitmapBresenhamLine(Bitmap* bitmap, int row1, int col1, int row2, int col2, Color color)
{
	unsigned int colorCode = GetColorCode(color);
	BresenhamData data = InitBresenham(row1, col1, row2, col2);
	while (1) {
		PaintBitmapPixel(bitmap, data.row1, data.col1, colorCode);

		if (data.row1 == data.row2 && data.col1 == data.col2)
			break;

		AdvanceBresenham(&data);
	}
}

void FloodfillBitmap(Bitmap* bitmap, int row, int col, Color color, MemArena* tmpArena)
{
	unsigned int paintColorCode = GetColorCode(color);
	unsigned int baseColorCode  = *GetBitmapPixelAddress(bitmap, row, col);
	Assert(paintColorCode != baseColorCode);
	int positionN = 0;
	int* positions = ArenaPushArray(tmpArena, int, 0);
	ArenaPush(tmpArena, int, row);
	ArenaPush(tmpArena, int, col);
	positionN++;

	// NOTE: Initial position has to be inserted twice, one for horizontal, one for vertical fill.
	ArenaPush(tmpArena, int, row);
	ArenaPush(tmpArena, int, col);
	positionN++;

	bool fillHorizontally = true;
	int directionSwitchPosition = 1;

	PaintBitmapPixel(bitmap, row, col, paintColorCode);

	for (int i = 0; i < positionN; ++i) {
		if (i == directionSwitchPosition) {
			fillHorizontally = !fillHorizontally;
			directionSwitchPosition = positionN;
		}
		int row = positions[2 * i];
		int col = positions[2 * i + 1];

		unsigned int* pixelStart = GetBitmapPixelAddress(bitmap, row, col);
		unsigned int* pixel = 0;

		if (fillHorizontally) {
			pixel = pixelStart;
			for (int left = col - 1; left >= 0; --left) {
				pixel--;
				if (*pixel != baseColorCode) 
					break;
				*pixel = paintColorCode;
				ArenaPush(tmpArena, int, row);
				ArenaPush(tmpArena, int, left);
				positionN++;
			}

			pixel = pixelStart;
			for (int right = col + 1; right < bitmap->width; ++right) {
				pixel++;
				if (*pixel != baseColorCode)
					break;
				*pixel = paintColorCode;
				ArenaPush(tmpArena, int, row);
				ArenaPush(tmpArena, int, right);
				positionN++;
			}
		} else {
			pixel = pixelStart;
			for (int top = row - 1; top >= 0; --top) {
				pixel -= bitmap->width;
				if (*pixel != baseColorCode)
					break;
				*pixel = paintColorCode;
				ArenaPush(tmpArena, int, top);
				ArenaPush(tmpArena, int, col);
				positionN++;
			}

			pixel = pixelStart;
			for (int bottom = row + 1; bottom < bitmap->height; ++bottom) {
				pixel += bitmap->width;
				if (*pixel != baseColorCode)
					break;
				*pixel = paintColorCode;
				ArenaPush(tmpArena, int, bottom);
				ArenaPush(tmpArena, int, col);
				positionN++;
			}
		}
	}
	ArenaPopTo(tmpArena, positions);
}

void FillBitmapWithColor(Bitmap* bitmap, Color color)
{
	unsigned int colorCode = GetColorCode(color);
	unsigned int* pixel = (unsigned int*)bitmap->memory;
	for (int row = 0; row < bitmap->height; ++row) {
		for (int col = 0; col < bitmap->width; ++col) {
			*pixel = colorCode;
			pixel++;
		}
	}
}

void CopyScaledRotatedBitmap(Bitmap* fromBitmap, Bitmap* toBitmap, int toCenterRow, int toCenterCol, float width, float height, float rotationAngle)
{
	Point center = {};
	center.x = (float)toCenterCol;
	center.y = (float)toCenterRow;

	float inverseHeight = Invert(height);
	float inverseWidth  = Invert(width);

	Point heightUnitVector = RotationVector(rotationAngle);
	Point widthUnitVector = TurnVectorToRight(heightUnitVector);

	Point inverseHeightVector = heightUnitVector;
	inverseHeightVector.x *= inverseHeight;
	inverseHeightVector.y *= inverseHeight;

	Point inverseWidthVector = widthUnitVector;
	inverseWidthVector.x *= inverseWidth;
	inverseWidthVector.y *= inverseWidth;

	float halfHeight = (float)height * 0.5f;
	float halfWidth  = (float)width * 0.5f;
	Point frontCenter = PointSum(center, PointProd(+halfHeight, heightUnitVector));
	Point frontLeft   = PointSum(frontCenter, PointProd(-halfWidth, widthUnitVector));
	Point frontRight  = PointSum(frontCenter, PointProd(+halfWidth, widthUnitVector));

	Point backCenter  = PointSum(center, PointProd(-halfHeight, heightUnitVector));
	Point backLeft    = PointSum(backCenter, PointProd(-halfWidth, widthUnitVector));
	Point backRight   = PointSum(backCenter, PointProd(+halfWidth, widthUnitVector));

	int leftBoundary   = (int)Min4(frontLeft.x, frontRight.x, backLeft.x, backRight.x);
	int rightBoundary  = (int)Max4(frontLeft.x, frontRight.x, backLeft.x, backRight.x);
	int topBoundary    = (int)Min4(frontLeft.y, frontRight.y, backLeft.y, backRight.y);
	int bottomBoundary = (int)Max4(frontLeft.y, frontRight.y, backLeft.y, backRight.y);

	int left   = IntMax2(leftBoundary, 0);
	int right  = IntMin2(rightBoundary, toBitmap->width - 1);
	int top    = IntMax2(topBoundary, 0);
	int bottom = IntMin2(bottomBoundary, toBitmap->height - 1);

	for (int toRow = top; toRow <= bottom; ++toRow) {
		for (int toCol = left; toCol <= right; ++toCol) {
			Point position = {};
			position.x = (float)toCol;
			position.y = (float)toRow;
			Point positionVector = PointDiff(position, center);
			float heightCoordinate = DotProduct(positionVector, inverseHeightVector);
			float widthCoordinate = DotProduct(positionVector, inverseWidthVector);

			float heightRatio = 1.0f - (0.5f + (heightCoordinate));
			float widthRatio  = 1.0f - (0.5f + (widthCoordinate));
			if (!IsBetween(heightRatio, 0.0f, 1.0f))
				continue;
			if (!IsBetween(widthRatio, 0.0f, 1.0f))
				continue;

			Color fillColor = GetColor(1.0f, 0.0f, 1.0f);
			unsigned int fromColorCode = GetClosestBitmapColorCode(fromBitmap, heightRatio, widthRatio);
			Color fromColor = GetColorFromColorCode(fromColorCode);
			if (fromColor.alpha != 0.0f) {
				unsigned int* pixelAddress = GetBitmapPixelAddress(toBitmap, toRow, toCol);
				*pixelAddress = fromColorCode;
			}
		}
	}
}

void CopyStretchedBitmap(Bitmap* fromBitmap, Bitmap* toBitmap, int toLeft, int toRight, int toTop, int toBottom)
{
	float toWidth  = (float)(toRight - toLeft);
	float toHeight = (float)(toBottom - toTop);

	int left   = ClipInt(toLeft,   0, toBitmap->width - 1);
	int right  = ClipInt(toRight,  0, toBitmap->width - 1);
	int top    = ClipInt(toTop,    0, toBitmap->height - 1);
	int bottom = ClipInt(toBottom, 0, toBitmap->height - 1);

	for (int toRow = top; toRow <= bottom; ++toRow) {
		for (int toCol = left; toCol <= right; ++toCol) {
			float fromWidthRatio  = (float)(toCol - toLeft) / toWidth;
			float fromHeightRatio = (float)(toRow - toTop)  / toHeight;
			unsigned int fromColorCode = GetClosestBitmapColorCode(fromBitmap, fromHeightRatio, fromWidthRatio);
			unsigned int* toPixelAddress = GetBitmapPixelAddress(toBitmap, toRow, toCol);
			*toPixelAddress = fromColorCode;
		}
	}
}

void DrawBitmapPolyOutline(Bitmap* bitmap, int polyN, int* polyColRow, Color color)
{
	for (int i = 0; i < polyN; ++i) {
		int next = 0;
		if (i < polyN - 1)
			next = i + 1;
		else
			next = 0;
		int row1 = polyColRow[2 * i];
		int col1 = polyColRow[2 * i + 1];
		int row2 = polyColRow[2 * next];
		int col2 = polyColRow[2 * next + 1];
		DrawBitmapBresenhamLine(bitmap, row1, col1, row2, col2, color);
	}
}