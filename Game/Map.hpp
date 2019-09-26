#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Memory.hpp"
#include "Type.hpp"

enum TileId
{
	NoTileId,
	CaveTileId
};

struct Map
{
	Int32 tileRowN;
	Int32 tileColN;
	TileId *tileTypes;
};

#define MapTileSide 10.0f

static Real32
func GetMapWidth(Map *map)
{
	Real32 width = map->tileColN * MapTileSide;
	return width;
}

static Real32
func GetMapHeight(Map *map)
{
	Real32 height = map->tileRowN * MapTileSide;
	return height;
}

static IntVec2
func MakeTile(Int32 row, Int32 col)
{
	IntVec2 tile = MakeIntPoint(row, col);
	return tile;
}

static IntVec2
func MakeGrid(Int32 row, Int32 col)
{
	IntVec2 grid = MakeIntPoint(row, col);
	return grid;
}

static Bool32
func IsValidTile(Map *map, IntVec2 index)
{
	Bool32 result = (IsIntBetween(index.row, 0, map->tileRowN - 1) &&
					 IsIntBetween(index.col, 0, map->tileColN - 1));
	return result;
}

static Bool32
func IsValidGridIndex(Map *map, IntVec2 index)
{
	Bool32 result = (IsIntBetween(index.row, 0, map->tileRowN) &&
					 IsIntBetween(index.col, 0, map->tileColN));
	return result;
}

static TileId
func GetTileType(Map *map, IntVec2 tile)
{
	Assert(IsValidTile(map, tile));
	TileId tileType = map->tileTypes[tile.row * map->tileColN + tile.col];
	return tileType;
}

static Bool32
func IsTileType(Map *map, IntVec2 tile, TileId type)
{
	TileId actualType = GetTileType(map, tile);
	Bool32 isType = (actualType == type);
	return isType;
}

static void
func SetTileType(Map *map, IntVec2 tile, TileId type)
{
	Assert(IsValidTile(map, tile));
	map->tileTypes[tile.row * map->tileColN + tile.col] = type;
}

static IntVec2
func GetRandomTile(Map *map)
{
	IntVec2 tile = {};
	tile.row = IntRandom(0, map->tileRowN - 1);
	tile.col = IntRandom(0, map->tileColN - 1);
	Assert(IsValidTile(map, tile));
	return tile;
}

#define InvalidTileIndex MakeTile(-1, -1)
static Vec2
func GetTileCenter(Map *map, IntVec2 index)
{
	Assert(IsValidTile(map, index));
	Real32 x = (Real32)index.col * MapTileSide + MapTileSide * 0.5f;
	Real32 y = (Real32)index.row * MapTileSide + MapTileSide * 0.5f;
	Vec2 position = MakePoint(x, y);
	return position;
}

static Vec2
func GetRandomTileCenter(Map *map)
{
	IntVec2 tile = GetRandomTile(map);
	Vec2 center = GetTileCenter(map, tile);
	return center;
}

static Bool32
func IsPassableTile(Map *map, IntVec2 tile)
{
	TileId tileType = GetTileType(map, tile);
	bool isPassable = (tileType != NoTileId);
	return isPassable;
}

static Vec2
func GetGridPosition(Map *map, IntVec2 index)
{
	Real32 x = (Real32)index.col * MapTileSide;
	Real32 y = (Real32)index.row * MapTileSide;
	Vec2 position = MakePoint(x, y);
	return position;
}

static IntVec2
func GetContainingTile(Map *map, Vec2 point)
{
	Int32 row = Floor(point.y / MapTileSide);
	Int32 col = Floor(point.x / MapTileSide);
	IntVec2 index = MakeTile(row, col);
	return index;
}

static Vec4
func GetTileColor(Map* map, Int32 row, Int32 col)
{
	Assert(IsIntBetween(row, 0, map->tileRowN - 1));
	Assert(IsIntBetween(col, 0, map->tileColN - 1));

	Int32 tileIndex = row * map->tileColN + col;

	Vec4 blackColor = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 caveColor  = MakeColor(0.2f, 0.05f, 0.0f);

	Vec4 color = {};
	TileId tileType = map->tileTypes[tileIndex];
	switch(tileType)
	{
		case NoTileId:
		{
			color = blackColor;
			break;
		}
		case CaveTileId:
		{
			color = caveColor;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	return color;
}

static Rect
func GetTileRect(Map *map, IntVec2 tile)
{
	Assert(IsValidTile(map, tile));
	Vec2 tileCenter = GetTileCenter(map, tile);
	Rect tileRect = MakeSquareRect(tileCenter, MapTileSide);
	return tileRect;
}

static void
func HighlightTile(Canvas *canvas, Map *map, IntVec2 tile, Vec4 color)
{
	Assert(IsValidTile(map, tile));
	Rect rect = GetTileRect(map, tile);
	DrawRect(canvas, rect, color);
}

static Bool32 TilesAreAdjacent(IntVec2 tile1, IntVec2 tile2)
{
	Int32 rowAbs = IntAbs(tile1.row - tile2.row);
	Int32 colAbs = IntAbs(tile1.col - tile2.col);
	Bool32 areAdjacent = ((rowAbs == 0 && colAbs == 1) || (rowAbs == 1 && colAbs == 0));
	return areAdjacent;
}

static void
func DrawMap(Canvas *canvas, Map* map)
{
	Camera *camera = canvas->camera;
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
		Real32 tileTop    = mapTop + MapTileSide * row;
		Real32 tileBottom = tileTop + MapTileSide;
		if(tileBottom < cameraTop || tileTop > cameraBottom)
		{
			continue;
		}

		for(Int32 col = 0; col < map->tileColN; col++)
		{
			Real32 tileLeft  = mapLeft + MapTileSide * col;
			Real32 tileRight = tileLeft + MapTileSide;
			if(tileRight < cameraLeft || tileLeft > cameraRight)
			{
				continue;
			}

			Vec4 color = GetTileColor(map, row, col);
			DrawRectLRTB(canvas, tileLeft, tileRight, tileTop, tileBottom, color);
		}
	}
}

static Map
func ReadMapFromFile(Int8 *filePath, MemArena *arena)
{
	HANDLE file = CreateFileA(filePath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	Assert(file != INVALID_HANDLE_VALUE);

	arena->usedSize = 0;
	DWORD fileSize = GetFileSize(file, 0);
	Assert(fileSize <= arena->maxSize);

	DWORD readSize = 0;
	bool result = ReadFile(file, arena->baseAddress, fileSize, &readSize, 0);
	Assert(result);
	Assert(fileSize == readSize);

	arena->usedSize += fileSize;

	Int8 *base = arena->baseAddress;
	Map *map = (Map *)base;
	map->tileTypes = (TileId *)GetAbsoluteAddress(map->tileTypes, base);

	result = CloseHandle(file);
	Assert(result);

	return *map;
}