#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Memory.hpp"
#include "Type.hpp"

struct RandomValueTable
{
	Real32* values;
	Int32 valueN;
};

struct Map
{
	Real32 tileSide;
	Int32 tileRowN;
	Int32 tileColN;
	Bitmap bitmap;
};

static func RandomValueTable CreateRandomValueTable(Int32 valueN, MemArena* arena)
{
	Assert(valueN > 0);

	RandomValueTable table = {};
	table.valueN = valueN;
	table.values = ArenaPushArray(arena, Real32, valueN);
	return table;
}

#define MinGridLevel 2
#define MaxGridLevel 8

static Real32 func GetMapWidth(Map* map)
{
	Real32 width = (map->tileSide * map->tileColN);
	return width;
}

static Real32 func GetMapHeight(Map* map)
{
	Real32 height = (map->tileSide * map->tileRowN);
	return height;
}

static Real32 func GetValueFromRandomTable(RandomValueTable* table, Int32 index)
{
	Real32 value = table->values[index % table->valueN];
	return value;
}

static Real32 func SmoothRatio(Real32 x)
{
	Real32 result = x * x * (3.0f - 2.0f * x);
	return result;
}

static Real32 func GetGridPointValue(RandomValueTable* table, Int32 gridLevel, Int32 row, Int32 col)
{
	Assert(IsIntBetween(gridLevel, MinGridLevel, MaxGridLevel));
	Int32 gridN = (1 << gridLevel);

	Assert(IsIntBetween(row, 0, gridN));
	Assert(IsIntBetween(col, 0, gridN));

	Int32 firstIndexOnLevel = 0;
	for(Int32 i = 0; i < gridLevel; i++)
	{
		firstIndexOnLevel += IntSquare((1 << i) + 1);
	}

	Int32 valueIndex = firstIndexOnLevel + (gridN + 1) * row + col;
	Real32 value = GetValueFromRandomTable(table, valueIndex);
	return value;
}

static Real32 func GetPointValue(RandomValueTable* table, Int32 gridLevel, Real32 x, Real32 y)
{
	Assert(IsBetween(x, 0.0f, 1.0f));
	Assert(IsBetween(y, 0.0f, 1.0f));
	Assert(IsIntBetween(gridLevel, MinGridLevel, MaxGridLevel));

	Int32 gridN = (1 << gridLevel);
	Int32 left   = Floor(x * gridN);
	Int32 right  = left + 1;
	Int32 top    = Floor(y * gridN);
	Int32 bottom = top + 1;

	Real32 ratioX = SmoothRatio(Fraction(x * gridN));
	Real32 ratioY = SmoothRatio(Fraction(y * gridN));

	Real32 topLeftValue     = GetGridPointValue(table, gridLevel, top, left);
	Real32 topRightValue    = GetGridPointValue(table, gridLevel, top, right);
	Real32 bottomLeftValue  = GetGridPointValue(table, gridLevel, bottom, left);
	Real32 bottomRightValue = GetGridPointValue(table, gridLevel, bottom, right);

	Real32 topValue    = Lerp(topLeftValue, ratioX, topRightValue);
	Real32 bottomValue = Lerp(bottomLeftValue, ratioX, bottomRightValue);

	Real32 value = Lerp(topValue, ratioY, bottomValue);

	return value;
}

static void func InitRandomValueTable(RandomValueTable* table)
{
	InitRandom();
	for(Int32 i = 0; i < table->valueN; i++)
	{
		table->values[i] = RandomBetween(-1.0f, 1.0f);
	}
}

static Map func GenerateForestMap(MemArena* arena)
{
	Map map = {};
	map.tileSide = 1.0f;
	map.tileRowN = 512;
	map.tileColN = 512;
	ResizeBitmap(&map.bitmap, map.tileRowN, map.tileColN);

	Int8* arenaTop = GetArenaTop(arena);
	RandomValueTable randomValueTable = CreateRandomValueTable(65536, arena);
	InitRandomValueTable(&randomValueTable);

	Real32* values = ArenaPushArray(arena, Real32, map.tileRowN * map.tileColN);
	Int32 valueIndex = 0;
	for(Int32 row = 0; row < map.tileRowN; row++)
	{
		for(Int32 col = 0; col < map.tileColN; col++)
		{
			values[valueIndex] = 0.0f;
			valueIndex++;
		}
	}

	Real32 gridLevelSwitchRatio = 0.67f;

	Real32 valueRatio = 1.0f;
	for(Int32 level = MinGridLevel; level <= MaxGridLevel; level++)
	{
		Int32 valueIndex = 0;
		for(Int32 row = 0; row < map.tileRowN; row++)
		{
			for(Int32 col = 0; col < map.tileColN; col++)
			{
				Real32 xRatio = ((Real32)col / (Real32)map.tileColN);
				Real32 yRatio = ((Real32)row / (Real32)map.tileRowN);

				Real32 value = GetPointValue(&randomValueTable, level, xRatio, yRatio);

				values[valueIndex] += value * valueRatio;
				valueIndex++;
			}
		}

		valueRatio *= gridLevelSwitchRatio;
	}

	Real32 minValue = values[0];
	Real32 maxValue = values[0];
	valueIndex = 0;
	for(Int32 row = 0; row < map.tileRowN; row++)
	{
		for(Int32 col = 0; col < map.tileColN; col++)
		{
			minValue = Min2(minValue, values[valueIndex]);
			maxValue = Max2(maxValue, values[valueIndex]);
			valueIndex++;
		}
	}

	Assert(minValue < maxValue);
	valueIndex = 0;
	for(Int32 row = 0; row < map.tileRowN; row++)
	{
		for(Int32 col = 0; col < map.tileColN; col++)
		{
			Assert(IsBetween(values[valueIndex], minValue, maxValue));
			values[valueIndex] = (values[valueIndex] - minValue) / (maxValue - minValue);
			values[valueIndex] = SmoothRatio(values[valueIndex]);
			valueIndex++;
		}
	}

	UInt32* pixel = map.bitmap.memory;
	Vec4 waterLowColor   = MakeColor(0.0f, 0.0f, 0.1f);
	Vec4 waterHighColor  = MakeColor(0.0f, 0.0f, 1.0f);
	Vec4 groundLowColor  = MakeColor(0.0f, 1.0f, 0.0f);
	Vec4 groundHighColor = MakeColor(0.0f, 0.1f, 0.0f);

	Real32 waterLevel = 0.05f;

	valueIndex = 0;
	for(Int32 row = 0; row < map.tileRowN; row++)
	{
		for(Int32 col = 0; col < map.tileColN; col++)
		{
			Real32 value = values[valueIndex];
			valueIndex++;
			Assert(IsBetween(value, 0.0f, 1.0f));

			Vec4 color = {};
			if(IsBetween(value, 0.0f, waterLevel))
			{
				color = InterpolateColors(waterLowColor, value / waterLevel, waterHighColor); 
			}
			else
			{
				color = InterpolateColors(groundLowColor, (value - waterLevel) / (1.0f - waterLevel), groundHighColor);
			}

			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	return map;
}

static void func DrawMap(Canvas* canvas, Map* map)
{
	Camera* camera = canvas->camera;
	Real32 cameraLeft   = CameraLeftSide(camera);
	Real32 cameraRight  = CameraRightSide(camera);
	Real32 cameraTop    = CameraTopSide(camera);
	Real32 cameraBottom = CameraBottomSide(camera);

	Real32 mapLeft   = 0.0f;
	Real32 mapRight  = GetMapWidth(map);
	Real32 mapTop    = 0.0f;
	Real32 mapBottom = GetMapHeight(map);

	for(Int32 row = 0; row < map->tileRowN; row++)
	{
		Real32 tileTop    = mapTop + map->tileSide * row;
		Real32 tileBottom = tileTop + map->tileSide;
		if(tileBottom < cameraTop || tileTop > cameraBottom)
		{
			continue;
		}

		for(Int32 col = 0; col < map->tileColN; col++)
		{
			Real32 tileLeft  = mapLeft + map->tileSide * col;
			Real32 tileRight = tileLeft + map->tileSide;
			if(tileRight < cameraLeft || tileLeft > cameraRight)
			{
				continue;
			}

			Vec4 color = GetBitmapPixelColor(&map->bitmap, row, col);
			DrawRectLRTB(canvas, tileLeft, tileRight, tileTop, tileBottom, color);
		}
	}
}

// TODO: Save only values, GetTileColor function!