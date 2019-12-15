#pragma once

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Item.hpp"
#include "Memory.hpp"
#include "Type.hpp"

enum TileId
{
	NoTileId,
	CaveTileId
};

struct MapItem
{
	ItemId item_id;
	Vec2 position;
};

struct Map
{
	Int32 tile_row_n;
	Int32 tile_col_n;
	TileId *tile_types;

	Int32 item_n;
	MapItem *items;
};

#define MapTileSide 10.0f

static Real32
func GetMapWidth(Map *map)
{
	Real32 width = map->tile_col_n * MapTileSide;
	return width;
}

static Real32
func GetMapHeight(Map *map)
{
	Real32 height = map->tile_row_n * MapTileSide;
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
	Bool32 result = (IsIntBetween(index.row, 0, map->tile_row_n - 1) &&
					 IsIntBetween(index.col, 0, map->tile_col_n - 1));
	return result;
}

static Bool32
func IsValidGridIndex(Map *map, IntVec2 index)
{
	Bool32 result = (IsIntBetween(index.row, 0, map->tile_row_n) &&
					 IsIntBetween(index.col, 0, map->tile_col_n));
	return result;
}

static TileId
func GetTileType(Map *map, IntVec2 tile)
{
	Assert(IsValidTile(map, tile));
	TileId tile_type = map->tile_types[tile.row * map->tile_col_n + tile.col];
	return tile_type;
}

static Bool32
func IsTileType(Map *map, IntVec2 tile, TileId type)
{
	TileId actual_type = GetTileType(map, tile);
	Bool32 is_type = (actual_type == type);
	return is_type;
}

static void
func SetTileType(Map *map, IntVec2 tile, TileId type)
{
	Assert(IsValidTile(map, tile));
	map->tile_types[tile.row * map->tile_col_n + tile.col] = type;
}

static IntVec2
func GetRandomTile(Map *map)
{
	IntVec2 tile = {};
	tile.row = IntRandom(0, map->tile_row_n - 1);
	tile.col = IntRandom(0, map->tile_col_n - 1);
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
	TileId tile_type = GetTileType(map, tile);
	bool is_passable = (tile_type != NoTileId);
	return is_passable;
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
	Assert(IsIntBetween(row, 0, map->tile_row_n - 1));
	Assert(IsIntBetween(col, 0, map->tile_col_n - 1));

	Int32 tile_index = row * map->tile_col_n + col;

	Vec4 black_color = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 cave_color  = MakeColor(0.2f, 0.05f, 0.0f);

	Vec4 color = {};
	TileId tile_type = map->tile_types[tile_index];
	switch(tile_type)
	{
		case NoTileId:
		{
			color = black_color;
			break;
		}
		case CaveTileId:
		{
			color = cave_color;
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
	Vec2 tile_center = GetTileCenter(map, tile);
	Rect tile_rect = MakeSquareRect(tile_center, MapTileSide);
	return tile_rect;
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
	Int32 row_abs = IntAbs(tile1.row - tile2.row);
	Int32 col_abs = IntAbs(tile1.col - tile2.col);
	Bool32 are_adjacent = ((row_abs == 0 && col_abs == 1) || (row_abs == 1 && col_abs == 0));
	return are_adjacent;
}

#define MapItemRadius 0.2f

static void
func DrawMapItem(Canvas *canvas, MapItem *item)
{
	Vec4 item_color = MakeColor(0.5f, 0.0f, 0.5f);
	DrawCircle(canvas, item->position, MapItemRadius, item_color);
}

static void
func DrawMapItems(Canvas *canvas, Map *map)
{
	for(Int32 i = 0; i < map->item_n; i++)
	{
		DrawMapItem(canvas, &map->items[i]);
	}
}

static void
func DrawMapWithoutItems(Canvas *canvas, Map *map)
{
	Camera *camera = canvas->camera;
	Real32 camera_left   = CameraLeftSide(camera);
	Real32 camera_right  = CameraRightSide(camera);
	Real32 camera_top    = CameraTopSide(camera);
	Real32 camera_bottom = CameraBottomSide(camera);

	Real32 map_left   = 0.0f;
	Real32 map_right  = GetMapWidth(map);
	Real32 map_top    = 0.0f;
	Real32 map_bottom = GetMapHeight(map);

	for(Int32 row = 0; row < map->tile_row_n; row++)
	{
		Real32 tile_top    = map_top + MapTileSide * row;
		Real32 tile_bottom = tile_top + MapTileSide;
		if(tile_bottom < camera_top || tile_top > camera_bottom)
		{
			continue;
		}

		for(Int32 col = 0; col < map->tile_col_n; col++)
		{
			Real32 tile_left  = map_left + MapTileSide * col;
			Real32 tile_right = tile_left + MapTileSide;
			if(tile_right < camera_left || tile_left > camera_right)
			{
				continue;
			}

			Vec4 color = GetTileColor(map, row, col);
			DrawRectLRTB(canvas, tile_left, tile_right, tile_top, tile_bottom, color);
		}
	}
}

static void
func DrawMapWithItems(Canvas *canvas, Map *map)
{
	DrawMapWithoutItems(canvas, map);
	DrawMapItems(canvas, map);
}

#define MapVersion 1

static Map
func ReadMapFromFile(Int8 *file_path, MemArena *arena)
{
	HANDLE file = CreateFileA(file_path, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	Assert(file != INVALID_HANDLE_VALUE);

	arena->used_size = 0;
	DWORD file_size = GetFileSize(file, 0);
	Assert(file_size <= arena->max_size);

	DWORD read_size = 0;
	bool result = ReadFile(file, arena->base_address, file_size, &read_size, 0);
	Assert(result);
	Assert(file_size == read_size);
	arena->used_size += file_size;

	Int8 *base = arena->base_address;

	Int8 *position = base;
	Int32 version = *(Int32 *)position;
	Assert(version == MapVersion);

	position += sizeof(Int32);

	Map *map = (Map *)position;
	map->tile_types = (TileId *)GetAbsoluteAddress(map->tile_types, base);
	map->items = (MapItem *)GetAbsoluteAddress(map->items, base);

	position += sizeof(Map);
	position += map->tile_row_n * map->tile_col_n * sizeof(TileId);
	position += map->item_n * sizeof(MapItem);

	Int8 *arena_top = GetArenaTop(arena);
	Assert(position == arena_top);

	result = CloseHandle(file);
	Assert(result);

	return *map;
}