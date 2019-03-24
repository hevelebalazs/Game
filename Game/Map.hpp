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

static IntVec2 func MakeTileIndex(Int32 row, Int32 col)
{
	IntVec2 index = MakeIntPoint(row, col);
	return index;
}

static IntVec2 func MakeGridIndex(Int32 row, Int32 col)
{
	IntVec2 index = MakeIntPoint(row, col);
	return index;
}

static Bool32 func IsValidTileIndex(Map* map, IntVec2 index)
{
	Bool32 result = (IsIntBetween(index.row, 0, map->tileRowN - 1) &&
					 IsIntBetween(index.col, 0, map->tileColN - 1));
	return result;
}

static Bool32 func IsValidGridIndex(Map* map, IntVec2 index)
{
	Bool32 result = (IsIntBetween(index.row, 0, map->tileRowN) &&
					 IsIntBetween(index.col, 0, map->tileColN));
	return result;
}

static Bool32 func TileHasTree(Map* map, IntVec2 index)
{
	Assert(IsValidTileIndex(map, index));
	Bool32 hasTree = map->isTree[index.row * map->tileColN + index.col];
	return hasTree;
}

static Bool32 func IsPassableTile(Map* map, IntVec2 index)
{
	Bool32 isPassable = !TileHasTree(map, index);
	return isPassable;
}

static Vec2 func GetTileCenter(Map* map, IntVec2 index)
{
	Assert(IsValidTileIndex(map, index));
	Real32 x = (Real32)index.col * map->tileSide + map->tileSide * 0.5f;
	Real32 y = (Real32)index.row * map->tileSide + map->tileSide * 0.5f;
	Vec2 position = MakePoint(x, y);
	return position;
}

static Vec2 func GetGridPosition(Map* map, IntVec2 index)
{
	Real32 x = (Real32)index.col * map->tileSide;
	Real32 y = (Real32)index.row * map->tileSide;
	Vec2 position = MakePoint(x, y);
	return position;
}

static IntVec2 func GetContainingTileIndex(Map* map, Vec2 point)
{
	Int32 row = Floor(point.y / map->tileSide);
	Int32 col = Floor(point.x / map->tileSide);
	IntVec2 index = MakeTileIndex(row, col);
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

static IntVec2 func GetTopLeftGridIndex(IntVec2 tileIndex)
{
	IntVec2 gridIndex = MakeGridIndex(tileIndex.row, tileIndex.col);
	return gridIndex;
}

static IntVec2 func GetTopLeftTileIndex(IntVec2 gridIndex)
{
	IntVec2 tileIndex = MakeTileIndex(gridIndex.row - 1, gridIndex.col - 1);
	return tileIndex;
}

static IntVec2 func GetTopRightTileIndex(IntVec2 gridIndex)
{
	IntVec2 tileIndex = MakeTileIndex(gridIndex.row - 1, gridIndex.col);
	return tileIndex;
}

static IntVec2 func GetBottomLeftTileIndex(IntVec2 gridIndex)
{
	IntVec2 tileIndex = MakeTileIndex(gridIndex.row, gridIndex.col - 1);
	return tileIndex;
}

static IntVec2 func GetBottomRightTileIndex(IntVec2 gridIndex)
{
	IntVec2 tileIndex = MakeTileIndex(gridIndex.row, gridIndex.col);
	return tileIndex;
}

static IntVec2 func GetTreeTopLeftTileIndex(Map* map, IntVec2 tile)
{
	Assert(IsValidTileIndex(map, tile));
	Assert(TileHasTree(map, tile));

	Int32 topRow = tile.row;
	while(topRow > 0)
	{
		IntVec2 tileTop = MakeTileIndex(topRow - 1, tile.col);
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
			IntVec2 tileLeftTop = MakeTileIndex(topRow - 1, topLeftCol - 1);
			if(TileHasTree(map, tileLeftTop))
			{
				break;
			}
		}

		IntVec2 tileLeft = MakeTileIndex(topRow, topLeftCol - 1);
		Assert(IsValidTileIndex(map, tileLeft));
		if(!TileHasTree(map, tileLeft))
		{
			break;
		}
		topLeftCol--;
	}


	IntVec2 topLeftTile = MakeTileIndex(topRow, topLeftCol);
	return topLeftTile;
}

enum GridDirection
{
	GridLeft,
	GridRight,
	GridUp,
	GridDown
};

static Poly16 func GetExtendedTreeOutline(Map* map, IntVec2 tile, Real32 radius)
{
	Poly16 poly = {};
	poly.pointN = 0;

	IntVec2 topLeftTile = GetTreeTopLeftTileIndex(map, tile);
	IntVec2 startGridIndex = GetTopLeftGridIndex(topLeftTile);
	IntVec2 gridIndex = startGridIndex;

	GridDirection direction = GridRight;

	while (1)
	{
		Vec2 startPoint = GetGridPosition(map, gridIndex);
		Vec2 pointOffset = {};
		if(direction == GridRight)
		{
			while(1)
			{
				IntVec2 nextGridIndex = MakeGridIndex(gridIndex.row, gridIndex.col + 1);

				IntVec2 topTile = GetTopRightTileIndex(gridIndex);
				IntVec2 bottomTile = GetBottomRightTileIndex(gridIndex);
				Bool32 topTileOk = (!IsValidTileIndex(map, topTile) || !TileHasTree(map, topTile));
				Bool32 bottomTileOk = (IsValidTileIndex(map, topTile) && TileHasTree(map, bottomTile));
				if(topTileOk && bottomTileOk)
				{
					gridIndex = nextGridIndex;
				}
				else
				{
					if(bottomTileOk)
					{
						pointOffset = MakeVector(-radius, -radius);
						direction = GridUp;
					}
					else
					{
						pointOffset = MakeVector(+radius, -radius);
						direction = GridDown;
					}
					break;
				}
			}
		}
		else if(direction == GridLeft)
		{
			while(1)
			{
				IntVec2 nextGridIndex = MakeGridIndex(gridIndex.row, gridIndex.col - 1);

				IntVec2 topTile = GetTopLeftTileIndex(gridIndex);
				IntVec2 bottomTile = GetBottomLeftTileIndex(gridIndex);
				Bool32 topTileOk = (IsValidTileIndex(map, topTile) && TileHasTree(map, topTile));
				Bool32 bottomTileOk = (!IsValidTileIndex(map, bottomTile) || !TileHasTree(map, bottomTile));
				if(topTileOk && bottomTileOk)
				{
					gridIndex = nextGridIndex;
				}
				else
				{
					if(topTileOk)
					{
						pointOffset = MakeVector(+radius, +radius);
						direction = GridDown;
					}
					else
					{
						pointOffset = MakeVector(-radius, +radius);
						direction = GridUp;
					}
					break;
				}
			}
		}
		else if(direction == GridUp)
		{
			while(1)
			{
				IntVec2 nextGridIndex = MakeGridIndex(gridIndex.row - 1, gridIndex.col);

				IntVec2 leftTile = GetTopLeftTileIndex(gridIndex);
				IntVec2 rightTile = GetTopRightTileIndex(gridIndex);
				Bool32 leftTileOk = (!IsValidTileIndex(map, leftTile) || !TileHasTree(map, leftTile));
				Bool32 rightTileOk = (IsValidTileIndex(map, rightTile) && TileHasTree(map, rightTile));
				if(leftTileOk && rightTileOk)
				{
					gridIndex = nextGridIndex;
				}
				else
				{
					if(rightTileOk)
					{
						pointOffset = MakeVector(-radius, +radius);
						direction = GridLeft;
					}
					else
					{
						pointOffset = MakeVector(-radius, -radius);
						direction = GridRight;
					}
					break;
				}
			}
		}
		else if(direction == GridDown)
		{
			while(1)
			{
				IntVec2 nextGridIndex = MakeGridIndex(gridIndex.row + 1, gridIndex.col);

				IntVec2 leftTile = GetBottomLeftTileIndex(gridIndex);
				IntVec2 rightTile = GetBottomRightTileIndex(gridIndex);
				Bool32 leftTileOk = (IsValidTileIndex(map, leftTile) && TileHasTree(map, leftTile));
				Bool32 rightTileOk = (!IsValidTileIndex(map, rightTile) || !TileHasTree(map, rightTile));
				if(leftTileOk && rightTileOk)
				{
					gridIndex = nextGridIndex;
				}
				else
				{
					if(leftTileOk)
					{
						pointOffset = MakePoint(+radius, -radius);
						direction = GridRight;
					}
					else
					{
						pointOffset = MakePoint(+radius, +radius);
						direction = GridLeft;
					}
					break;
				}
			}
		}
		else
		{
			DebugBreak();
		}

		Vec2 point = GetGridPosition(map, gridIndex) + pointOffset;
		Poly16Add(&poly, point);

		if(gridIndex.row == startGridIndex.row && gridIndex.col == startGridIndex.col)
		{
			break;
		}
	}
	return poly;
}

static Poly16 func GetTreeOutline(Map* map, IntVec2 tile)
{
	Poly16 poly = GetExtendedTreeOutline(map, tile, 0.0f);
	return poly; 
}

static void func DrawTreeOutline(Canvas* canvas, Map* map, IntVec2 tile)
{
	Poly16 poly = GetTreeOutline(map, tile);
	Vec4 color = MakeColor(1.0f, 0.0f, 1.0f);
	DrawPolyOutline(canvas, poly.points, poly.pointN, color);
}

static void func DrawExtendedTreeOutline(Canvas* canvas, Map* map, IntVec2 tile, Real32 radius)
{
	Poly16 poly = GetExtendedTreeOutline(map, tile, radius);
	Vec4 color = MakeColor(1.0f, 0.0f, 1.0f);
	DrawPolyOutline(canvas, poly.points, poly.pointN, color);
}

static Rect func GetTileRect(Canvas* canvas, Map* map, IntVec2 tileIndex)
{
	Assert(IsValidTileIndex(map, tileIndex));
	Vec2 tileCenter = GetTileCenter(map, tileIndex);
	Rect tileRect = MakeSquareRect(tileCenter, map->tileSide);
	return tileRect;
}

static void func HighlightTile(Canvas* canvas, Map* map, IntVec2 tileIndex, Vec4 color)
{
	Assert(IsValidTileIndex(map, tileIndex));
	Rect rect = GetTileRect(canvas, map, tileIndex);
	DrawRect(canvas, rect, color);
}

static IntVec2 func GetNextTileInDirection(Map* map, IntVec2 tile, GridDirection direction)
{
	IntVec2 nextTile = tile;
	switch(direction)
	{
		case GridLeft:
		{
			nextTile.col--;
			break;
		}
		case GridRight:
		{
			nextTile.col++;
			break;
		}
		case GridUp:
		{
			nextTile.row--;
			break;
		}
		case GridDown:
		{
			nextTile.row++;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return nextTile;
}

static Bool32 TilesAreAdjacent(IntVec2 tile1, IntVec2 tile2)
{
	Int32 rowAbs = IntAbs(tile1.row - tile2.row);
	Int32 colAbs = IntAbs(tile1.col - tile2.col);
	Bool32 areAdjacent = ((rowAbs == 0 && colAbs == 1) || (rowAbs == 1 && colAbs == 0));
	return areAdjacent;
}

static GridDirection func GetLeftDirection(GridDirection direction)
{
	GridDirection leftDirection = direction;
	switch(direction)
	{
		case GridLeft:
		{
			leftDirection = GridDown;
			break;
		}
		case GridRight:
		{
			leftDirection = GridUp;
			break;
		}
		case GridUp:
		{
			leftDirection = GridLeft;
			break;
		}
		case GridDown:
		{
			leftDirection = GridRight;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return leftDirection;
}

static GridDirection func GetRightDirection(GridDirection direction)
{
	GridDirection rightDirection = direction;
	switch(direction)
	{
		case GridLeft:
		{
			rightDirection = GridUp;
			break;
		}
		case GridRight:
		{
			rightDirection = GridDown;
			break;
		}
		case GridUp:
		{
			rightDirection = GridRight;
			break;
		}
		case GridDown:
		{
			rightDirection = GridLeft;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return rightDirection;
}

struct NearTreePosition
{
	IntVec2 tile;
	IntVec2 treeTile;
	GridDirection direction;
};

static NearTreePosition func GetLeftNearTreePosition(Map* map, IntVec2 startTile, IntVec2 endTile)
{
	Assert(IsValidTileIndex(map, startTile));
	Assert(IsValidTileIndex(map, endTile));
	Assert(startTile != endTile);

	NearTreePosition position = {};
	position.tile = startTile;
	Assert(!TileHasTree(map, position.tile));

	position.treeTile = startTile;
	position.treeTile.row += IntSign(endTile.row - startTile.row);
	position.treeTile.col += IntSign(endTile.col - startTile.col);
	Assert(TileHasTree(map, position.treeTile));

	position.direction = GridRight;
	if(position.tile.row < position.treeTile.row && position.tile.col <= position.treeTile.col)
	{
		position.direction = GridRight;
	}
	else if(position.tile.col > position.treeTile.col && position.tile.row <= position.treeTile.row)
	{
		position.direction = GridDown;
	}
	else if(position.tile.row > position.treeTile.row && position.tile.col >= position.treeTile.col)
	{
		position.direction = GridLeft;
	}
	else if(position.tile.col < position.treeTile.col && position.tile.row >= position.treeTile.row)
	{
		position.direction = GridUp;
	}
	else
	{
		DebugBreak();
	}

	return position;

}

static NearTreePosition func GetRightNearTreePosition(Map* map, IntVec2 startTile, IntVec2 endTile)
{
	Assert(IsValidTileIndex(map, startTile));
	Assert(IsValidTileIndex(map, endTile));
	Assert(startTile != endTile);

	NearTreePosition position = {};

	position.tile = startTile;
	Assert(!TileHasTree(map, position.tile));

	position.treeTile = startTile;
	position.treeTile.row += IntSign(endTile.row - startTile.row);
	position.treeTile.col += IntSign(endTile.col - startTile.col);
	Assert(TileHasTree(map, position.treeTile));

	position.direction = GridLeft;
	if(startTile.col < position.treeTile.col && startTile.row <= position.treeTile.row)
	{
		position.direction = GridDown;
	}
	else if(startTile.row > position.treeTile.row && startTile.col <= position.treeTile.col)
	{
		position.direction = GridRight;
	}
	else if(startTile.col > position.treeTile.col && startTile.row >= position.treeTile.row)
	{
		position.direction = GridUp;
	}
	else if(startTile.row < position.treeTile.row && startTile.col >= position.treeTile.col)
	{
		position.direction = GridLeft;
	}
	else
	{
		DebugBreak();
	}

	return position;
}

static NearTreePosition func GetNextLeftNearTreePosition(Map* map, NearTreePosition position)
{
	Assert(!TileHasTree(map, position.tile));
	Assert(TileHasTree(map, position.treeTile));

	IntVec2 nextTile = GetNextTileInDirection(map, position.tile, position.direction);
	IntVec2 nextTreeTile = GetNextTileInDirection(map, position.treeTile, position.direction);

	NearTreePosition nextPosition = position;
	if(IsValidTileIndex(map, nextTile))
	{
		if(TilesAreAdjacent(position.tile, position.treeTile))
		{
			Assert(IsValidTileIndex(map, nextTreeTile));
			if(TileHasTree(map, nextTile))
			{
				nextPosition.treeTile = nextTile;
				nextPosition.direction = GetLeftDirection(position.direction);
			}
			else if(TileHasTree(map, nextTreeTile))
			{
				nextPosition.tile = nextTile;
				nextPosition.treeTile = nextTreeTile;
			}
			else
			{
				nextPosition.tile = nextTile;
				nextPosition.direction = GetRightDirection(position.direction);
			}
		}
		else
		{
			if(TileHasTree(map, nextTile))
			{
				nextPosition.treeTile = nextTile;
				nextPosition.direction = GetLeftDirection(position.direction);
			}
			else
			{
				nextPosition.tile = nextTile;
			}
		}

		Assert(!TileHasTree(map, nextPosition.tile));
		Assert(TileHasTree(map, nextPosition.treeTile));
	}
	else
	{
		nextPosition.tile = nextTile;
		nextPosition.treeTile = nextTreeTile;
	}

	return nextPosition;
}

static NearTreePosition func GetNextRightNearTreePosition(Map* map, NearTreePosition position)
{
	Assert(!TileHasTree(map, position.tile));
	Assert(TileHasTree(map, position.treeTile));

	IntVec2 nextTile = GetNextTileInDirection(map, position.tile, position.direction);
	IntVec2 nextTreeTile = GetNextTileInDirection(map, position.treeTile, position.direction);

	NearTreePosition nextPosition = position;
	if(IsValidTileIndex(map, nextTile))
	{
		if(TilesAreAdjacent(position.tile, position.treeTile))
		{
			Assert(IsValidTileIndex(map, nextTreeTile));
			if(TileHasTree(map, nextTile))
			{
				nextPosition.treeTile = nextTile;
				nextPosition.direction = GetRightDirection(position.direction);
			}
			else if(TileHasTree(map, nextTreeTile))
			{
				nextPosition.tile = nextTile;
				nextPosition.treeTile = nextTreeTile;
			}
			else
			{
				nextPosition.tile = nextTile;
				nextPosition.direction = GetLeftDirection(position.direction);
			}
		}
		else
		{
			if(TileHasTree(map, nextTile))
			{
				nextPosition.treeTile = nextTile;
				nextPosition.direction = GetRightDirection(position.direction);
			}
			else
			{
				nextPosition.tile = nextTile;
			}
		}

		Assert(!TileHasTree(map, nextPosition.tile));
		Assert(TileHasTree(map, nextPosition.treeTile));
	}
	else
	{
		nextPosition.tile = nextTile;
		nextPosition.treeTile = nextTreeTile;
	}

	return nextPosition;
}

static void func DrawPathAroundTree(Canvas* canvas, Map* map, IntVec2 startTile, IntVec2 endTile, Vec4 color)
{
	NearTreePosition rightPosition = GetRightNearTreePosition(map, startTile, endTile);
	Bool32 canGoRight = true;
	Int32 rightDistance = 0;
	while(rightPosition.tile != endTile)
	{
		NearTreePosition nextPosition = GetNextRightNearTreePosition(map, rightPosition);
		if(!IsValidTileIndex(map, nextPosition.tile) || !IsValidTileIndex(map, nextPosition.treeTile))
		{
			canGoRight = false;
			break;
		}
		else
		{
			rightPosition = nextPosition;
		}

		rightDistance++;
		if(rightDistance >= 100)
		{
			DebugBreak();
		}
	}

	NearTreePosition leftPosition = GetLeftNearTreePosition(map, startTile, endTile);
	Bool32 canGoLeft = true;
	Int32 leftDistance = 0;
	while(leftPosition.tile != endTile)
	{
		NearTreePosition nextPosition = GetNextLeftNearTreePosition(map, leftPosition);
		if(!IsValidTileIndex(map, nextPosition.tile) || !IsValidTileIndex(map, nextPosition.treeTile))
		{
			canGoLeft = false;
			break;
		}
		else
		{
			leftPosition = nextPosition;
		}

		leftDistance++;
		if(leftDistance >= 100)
		{
			DebugBreak();
		}
	}

	Assert(canGoRight || canGoLeft);

	Bool32 goLeft = (canGoLeft && !canGoRight) || (canGoLeft && canGoRight && leftDistance < rightDistance);
	NearTreePosition position = {};
	if(goLeft)
	{
		position = GetLeftNearTreePosition(map, startTile, endTile);
	}
	else
	{
		position = GetRightNearTreePosition(map, startTile, endTile);
	}

	while(position.tile != endTile)
	{
		IntVec2 previousTile = position.tile;
		if(goLeft)
		{
			position = GetNextLeftNearTreePosition(map, position);
		}
		else
		{
			position = GetNextRightNearTreePosition(map, position);
		}

		Assert(IsValidTileIndex(map, position.tile));
		Assert(IsValidTileIndex(map, position.treeTile));

		if(previousTile != position.tile)
		{
			Vec2 point1 = GetTileCenter(map, previousTile);
			Vec2 point2 = GetTileCenter(map, position.tile);
			Vec4 color = MakeColor(1.0f, 1.0f, 0.0f);
			Bresenham(canvas, point1, point2, color);
		}
	}
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

// TODO: IsValidNearTreePosition function