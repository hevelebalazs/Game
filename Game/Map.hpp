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

struct TileIndex
{
	Int32 row;
	Int32 col;
};

struct GridIndex
{
	Int32 row;
	Int32 col;
};

struct Map
{
	Real32 tileSide;
	Int32 tileRowN;
	Int32 tileColN;
	Real32* terrain;
	Bool8* isTree;
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

static TileIndex func MakeTileIndex(Int32 row, Int32 col)
{
	TileIndex index = {};
	index.row = row;
	index.col = col;
	return index;
}

static GridIndex func MakeGridIndex(Int32 row, Int32 col)
{
	GridIndex index = {};
	index.row = row;
	index.col = col;
	return index;
}

static Bool32 func IsValidTileIndex(Map* map, TileIndex index)
{
	Bool32 result = (IsIntBetween(index.row, 0, map->tileRowN - 1) &&
					 IsIntBetween(index.col, 0, map->tileColN - 1));
	return result;
}

static Bool32 func TileHasTree(Map* map, TileIndex index)
{
	Assert(IsValidTileIndex(map, index));
	Bool32 hasTree = map->isTree[index.row * map->tileColN + index.col];
	return hasTree;
}

static Bool32 func IsPassableTile(Map* map, TileIndex index)
{
	Bool32 isPassable = !TileHasTree(map, index);
	return isPassable;
}

static Vec2 func GetTileCenter(Map* map, TileIndex index)
{
	Assert(IsValidTileIndex(map, index));
	Real32 x = (Real32)index.col * map->tileSide + map->tileSide * 0.5f;
	Real32 y = (Real32)index.row * map->tileSide + map->tileSide * 0.5f;
	Vec2 position = MakePoint(x, y);
	return position;
}

static Vec2 func GetGridPosition(Map* map, GridIndex index)
{
	Real32 x = (Real32)index.col * map->tileSide;
	Real32 y = (Real32)index.row * map->tileSide;
	Vec2 position = MakePoint(x, y);
	return position;
}

static TileIndex func GetContainingTileIndex(Map* map, Vec2 point)
{
	Int32 row = Floor(point.y / map->tileSide);
	Int32 col = Floor(point.x / map->tileSide);
	TileIndex index = MakeTileIndex(row, col);
	return index;
}

#define MinTreeSide 5
#define MaxTreeSide 8
struct TreeShape
{
	Bool8 isTree[MaxTreeSide][MaxTreeSide];
};

static TreeShape func GenerateTreeShape()
{
	TreeShape shape = {};

	Int32 treeWidth  = IntRandom(MinTreeSide, MaxTreeSide);
	Int32 treeHeight = IntRandom(MinTreeSide, MaxTreeSide);

	for(Int32 row = 0; row < MaxTreeSide; row++)
	{
		for(Int32 col = 0; col < MaxTreeSide; col++)
		{
			shape.isTree[row][col] = (row < treeHeight && col < treeWidth);
		}
	}

	Int32 maxVerticalSlice   = treeHeight / 2 - 1;
	Int32 maxHorizontalSlice = treeWidth / 2 - 1;

	Int32 sliceN = IntRandom(3, 7);
	for(Int32 i = 0; i < sliceN; i++)
	{
		Int32 minRow = 0;
		Int32 maxRow = treeHeight - 1;
		Int32 minCol = 0;
		Int32 maxCol = treeWidth - 1;
		
		Bool32 sliceTopOrBottom = (IntRandom(0, 1) == 0);
		if(sliceTopOrBottom)
		{
			Bool32 sliceTop = (IntRandom(0, 1) == 0);
			if(sliceTop)
			{
				maxRow = minRow;
			}
			else
			{
				minRow = maxRow;
			}

			Bool32 sliceLeft = (IntRandom(0, 1) == 0);
			if(sliceLeft)
			{
				maxCol = minCol + IntRandom(0, maxHorizontalSlice - 1);
			}
			else
			{
				minCol = maxCol - IntRandom(0, maxHorizontalSlice - 1);
			}
		}
		else
		{
			Bool32 sliceLeft = (IntRandom(0, 1) == 0);
			if(sliceLeft)
			{
				maxCol = minCol;
			}
			else
			{
				minCol = maxCol;
			}

			Bool32 sliceTop = (IntRandom(0, 1) == 0);
			if(sliceTop)
			{
				maxRow = minRow + IntRandom(0, maxVerticalSlice - 1);
			}
			else
			{
				minRow = maxRow - IntRandom(0, maxVerticalSlice - 1);
			}
		}

		Assert(minRow <= maxRow);
		Assert(IsIntBetween(minRow, 0, MaxTreeSide - 1));
		Assert(IsIntBetween(maxRow, 0, MaxTreeSide - 1));

		Assert(minCol <= maxCol);
		Assert(IsIntBetween(minCol, 0, MaxTreeSide - 1));
		Assert(IsIntBetween(maxCol, 0, MaxTreeSide - 1));

		for(Int32 row = minRow; row <= maxRow; row++)
		{
			for(Int32 col = minCol; col <= maxCol; col++)
			{
				shape.isTree[row][col] = false;
			}
		}
	}

	return shape;
}

static void func ApplyTreeShape(Map* map, TreeShape* shape, Int32 leftCol, Int32 topRow)
{
	Int32 bottomRow = ClipInt(topRow + MaxTreeSide - 1, 0, map->tileRowN - 1);
	Int32 rightCol  = ClipInt(leftCol + MaxTreeSide - 1, 0, map->tileColN - 1);
	for(Int32 row = topRow; row <= bottomRow; row++)
	{
		for(Int32 col = leftCol; col <= rightCol; col++)
		{
			if(shape->isTree[row - topRow][col - leftCol])
			{
				Int32 tileIndex = map->tileColN * row + col;
				map->isTree[tileIndex] = true;
			}
		}
	}
}

static Map func GenerateForestMap(MemArena* arena)
{
	Map map = {};
	map.tileSide = 3.0f;
	map.tileRowN = 512;
	map.tileColN = 512;
	Int32 tileN = (map.tileRowN * map.tileColN);
	map.isTree = ArenaPushArray(arena, Bool8, tileN);
	map.terrain = ArenaPushArray(arena, Real32, tileN);

	Int8* arenaTop = GetArenaTop(arena);
	RandomValueTable randomValueTable = CreateRandomValueTable(65536, arena);
	InitRandomValueTable(&randomValueTable);

	for(Int32 i = 0; i < tileN; i++)
	{
		map.terrain[i] = 0.0f;
		map.isTree[i] = false;
	}

	Real32 gridLevelSwitchRatio = 0.67f;

	Real32 valueRatio = 1.0f;
	for(Int32 level = MinGridLevel; level <= MaxGridLevel; level++)
	{
		Int32 tileIndex = 0;
		for(Int32 row = 0; row < map.tileRowN; row++)
		{
			for(Int32 col = 0; col < map.tileColN; col++)
			{
				Real32 xRatio = ((Real32)col / (Real32)map.tileColN);
				Real32 yRatio = ((Real32)row / (Real32)map.tileRowN);

				Real32 value = GetPointValue(&randomValueTable, level, xRatio, yRatio);

				map.terrain[tileIndex] += value * valueRatio;
				tileIndex++;
			}
		}

		valueRatio *= gridLevelSwitchRatio;
	}

	Real32 minValue = map.terrain[0];
	Real32 maxValue = map.terrain[0];
	for(Int32 i = 0; i < tileN; i++)
	{
		minValue = Min2(minValue, map.terrain[i]);
		maxValue = Max2(maxValue, map.terrain[i]);
	}

	Assert(minValue < maxValue);

	for(Int32 i = 0; i < tileN; i++)
	{
		Assert(IsBetween(map.terrain[i], minValue, maxValue));
		map.terrain[i] = (map.terrain[i] - minValue) / (maxValue - minValue);
		map.terrain[i] = SmoothRatio(map.terrain[i]);
	}

	Int32* lowestTreeRow = ArenaPushArray(arena, Int32, map.tileColN);
	for(Int32 i = 0; i < map.tileColN; i++)
	{
		lowestTreeRow[i] = 0;
	}

	Int32 treeSide = 5;
	Int32 minTreeDistance = 8;
	Int32 maxTreeDistance = 12;
	Int32 finishedColN = 0;
	while(finishedColN < map.tileColN)
	{
		Int32 leftCol  = IntRandom(0, map.tileColN - 1);
		Int32 rightCol = ClipInt(leftCol + treeSide, 0, map.tileColN - 1);

		Int32 minRow = 0;
		Int32 leftCheckCol  = ClipInt(leftCol  - minTreeDistance, 0, map.tileColN - 1);
		Int32 rightCheckCol = ClipInt(rightCol + minTreeDistance, 0, map.tileColN - 1);
		for(Int32 col = leftCheckCol; col <= rightCheckCol; col++)
		{
			minRow = IntMax2(minRow, lowestTreeRow[col]);
		}

		Int32 topRow = minRow + IntRandom(minTreeDistance, maxTreeDistance);
		topRow = ClipInt(topRow, 0, map.tileRowN - 1);
		if(topRow == map.tileRowN - 1)
		{
			for(Int32 col = leftCol; col <= rightCol; col++)
			{
				if(lowestTreeRow[col] < map.tileRowN)
				{
					finishedColN++;
				}
				lowestTreeRow[col] = map.tileRowN - 1;
			}
			continue;
		}

		Int32 bottomRow = ClipInt(topRow + treeSide, 0, map.tileRowN - 1);
		for(Int32 col = leftCol; col <= rightCol; col++)
		{
			Assert(lowestTreeRow[col] < bottomRow);
			if(bottomRow == map.tileRowN - 1)
			{
				finishedColN++;
			}
			lowestTreeRow[col] = bottomRow;
		}

		TreeShape shape = GenerateTreeShape();
		ApplyTreeShape(&map, &shape, leftCol, topRow);
	}

	ArenaPopTo(arena, arenaTop);

	return map;
}

static Vec4 func GetTileColor(Map* map, Int32 row, Int32 col)
{
	Assert(IsIntBetween(row, 0, map->tileRowN - 1));
	Assert(IsIntBetween(col, 0, map->tileColN - 1));

	Int32 tileIndex = row * map->tileColN + col;
	
	Vec4 treeColor		 = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 waterLowColor   = MakeColor(0.0f, 0.0f, 0.1f);
	Vec4 waterHighColor  = MakeColor(0.0f, 0.0f, 1.0f);
	Vec4 groundLowColor  = MakeColor(0.0f, 1.0f, 0.0f);
	Vec4 groundHighColor = MakeColor(0.0f, 0.1f, 0.0f);

	Real32 waterLevel = 0.05f;

	Vec4 color = {};
	if(map->isTree[tileIndex])
	{
		color = treeColor;
	}
	else
	{
		Real32 height = map->terrain[tileIndex];
		Assert(IsBetween(height, 0.0f, 1.0f));

		if(IsBetween(height, 0.0f, waterLevel))
		{
			color = InterpolateColors(waterLowColor, height / waterLevel, waterHighColor); 
		}
		else
		{
			color = InterpolateColors(groundLowColor, (height - waterLevel) / (1.0f - waterLevel), groundHighColor);
		}
	}

	return color;
}

static GridIndex func GetTopLeftGridIndex(TileIndex tileIndex)
{
	GridIndex gridIndex = MakeGridIndex(tileIndex.row, tileIndex.col);
	return gridIndex;
}

static TileIndex func GetTopRightTileIndex(GridIndex gridIndex)
{
	TileIndex tileIndex = MakeTileIndex(gridIndex.row - 1, gridIndex.col);
	return tileIndex;
}

static TileIndex func GetBottomRightTileIndex(GridIndex gridIndex)
{
	TileIndex tileIndex = MakeTileIndex(gridIndex.row, gridIndex.col);
	return tileIndex;
}

static void func DrawTreeOutline(Canvas* canvas, Map* map, TileIndex tile)
{
	Assert(IsValidTileIndex(map, tile));
	Assert(TileHasTree(map, tile));

	Int32 topRow = tile.row;
	while(topRow > 0)
	{
		TileIndex tileTop = MakeTileIndex(topRow - 1, tile.col);
		Assert(IsValidTileIndex(map, tileTop));
		if(!TileHasTree(map, tileTop))
		{
			break;
		}
		topRow--;
	}

	Int32 topLeftCol = tile.col;
	while(topLeftCol > 0)
	{
		if(topRow > 0)
		{
			TileIndex tileLeftTop = MakeTileIndex(topRow - 1, topLeftCol - 1);
			if(TileHasTree(map, tileLeftTop))
			{
				break;
			}
		}

		TileIndex tileLeft = MakeTileIndex(topRow, topLeftCol - 1);
		Assert(IsValidTileIndex(map, tileLeft));
		if(!TileHasTree(map, tileLeft))
		{
			break;
		}
		topLeftCol--;
	}


	TileIndex topLeftTile = MakeTileIndex(topRow, topLeftCol);
	GridIndex gridIndex = GetTopLeftGridIndex(topLeftTile);
	Vec2 startPoint = GetGridPosition(map, gridIndex);

	while(1)
	{
		TileIndex topRightTile = GetTopRightTileIndex(gridIndex);
		TileIndex bottomRightTile = GetBottomRightTileIndex(gridIndex);
		if(!TileHasTree(map, topRightTile) && TileHasTree(map, bottomRightTile))
		{
			gridIndex.col++;
		}
		else
		{
			break;
		}
	}

	Vec2 endPoint = GetGridPosition(map, gridIndex);

	Vec4 color = MakeColor(1.0f, 0.0f, 1.0f);
	Bresenham(canvas, startPoint, endPoint, color);
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

			Vec4 color = GetTileColor(map, row, col);
			DrawRectLRTB(canvas, tileLeft, tileRight, tileTop, tileBottom, color);
		}
	}
}