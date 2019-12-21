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
	V2 position;
};

enum EntityGroupId
{
	NeutralGroupId,
	OrangeGroupId,
	PurpleGroupId
};

static V4
func GetEntityGroupColor(EntityGroupId group_id)
{
	V4 color = {};
	switch(group_id)
	{
		case OrangeGroupId:
		{
			color = MakeColor(1.0f, 0.5f, 0.0f);
			break;
		}
		case PurpleGroupId:
		{
			color = MakeColor(1.0f, 0.0f, 1.0f);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	return color;
}

struct MapEntity
{
	EntityGroupId group_id;
	V2 spawn_position;
};

struct Map
{
	I32 tile_row_n;
	I32 tile_col_n;
	TileId *tile_types;

	I32 item_n;
	MapItem *items;

	I32 entity_n;
	MapEntity *entities;
};

#define MapTileSide 10.0f

static R32
func GetMapWidth(Map *map)
{
	R32 width = map->tile_col_n * MapTileSide;
	return width;
}

static R32
func GetMapHeight(Map *map)
{
	R32 height = map->tile_row_n * MapTileSide;
	return height;
}

static IV2
func MakeTile(I32 row, I32 col)
{
	IV2 tile = MakeIntPoint(row, col);
	return tile;
}

static IV2
func MakeGrid(I32 row, I32 col)
{
	IV2 grid = MakeIntPoint(row, col);
	return grid;
}

static B32
func IsValidTile(Map *map, IV2 index)
{
	B32 result = (IsIntBetween(index.row, 0, map->tile_row_n - 1) &&
					 IsIntBetween(index.col, 0, map->tile_col_n - 1));
	return result;
}

static B32
func IsValidGridIndex(Map *map, IV2 index)
{
	B32 result = (IsIntBetween(index.row, 0, map->tile_row_n) &&
					 IsIntBetween(index.col, 0, map->tile_col_n));
	return result;
}

static TileId
func GetTileType(Map *map, IV2 tile)
{
	Assert(IsValidTile(map, tile));
	TileId tile_type = map->tile_types[tile.row * map->tile_col_n + tile.col];
	return tile_type;
}

static B32
func IsTileType(Map *map, IV2 tile, TileId type)
{
	TileId actual_type = GetTileType(map, tile);
	B32 is_type = (actual_type == type);
	return is_type;
}

static void
func SetTileType(Map *map, IV2 tile, TileId type)
{
	Assert(IsValidTile(map, tile));
	map->tile_types[tile.row * map->tile_col_n + tile.col] = type;
}

static IV2
func GetRandomTile(Map *map)
{
	IV2 tile = {};
	tile.row = IntRandom(0, map->tile_row_n - 1);
	tile.col = IntRandom(0, map->tile_col_n - 1);
	Assert(IsValidTile(map, tile));
	return tile;
}

#define InvalidTileIndex MakeTile(-1, -1)
static V2
func GetTileCenter(Map *map, IV2 index)
{
	Assert(IsValidTile(map, index));
	R32 x = (R32)index.col * MapTileSide + MapTileSide * 0.5f;
	R32 y = (R32)index.row * MapTileSide + MapTileSide * 0.5f;
	V2 position = MakePoint(x, y);
	return position;
}

static V2
func GetRandomTileCenter(Map *map)
{
	IV2 tile = GetRandomTile(map);
	V2 center = GetTileCenter(map, tile);
	return center;
}

static B32
func IsPassableTile(Map *map, IV2 tile)
{
	TileId tile_type = GetTileType(map, tile);
	bool is_passable = (tile_type != NoTileId);
	return is_passable;
}

static V2
func GetGridPosition(Map *map, IV2 index)
{
	R32 x = (R32)index.col * MapTileSide;
	R32 y = (R32)index.row * MapTileSide;
	V2 position = MakePoint(x, y);
	return position;
}

static IV2
func GetContainingTile(Map *map, V2 point)
{
	I32 row = Floor(point.y / MapTileSide);
	I32 col = Floor(point.x / MapTileSide);
	IV2 index = MakeTile(row, col);
	return index;
}

static V4
func GetTileColor(Map* map, I32 row, I32 col)
{
	Assert(IsIntBetween(row, 0, map->tile_row_n - 1));
	Assert(IsIntBetween(col, 0, map->tile_col_n - 1));

	I32 tile_index = row * map->tile_col_n + col;

	V4 black_color = MakeColor(0.0f, 0.0f, 0.0f);
	V4 cave_color  = MakeColor(0.2f, 0.05f, 0.0f);

	V4 color = {};
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
func GetTileRect(Map *map, IV2 tile)
{
	Assert(IsValidTile(map, tile));
	V2 tile_center = GetTileCenter(map, tile);
	Rect tile_rect = MakeSquareRect(tile_center, MapTileSide);
	return tile_rect;
}

static void
func HighlightTile(Canvas *canvas, Map *map, IV2 tile, V4 color)
{
	Assert(IsValidTile(map, tile));
	Rect rect = GetTileRect(map, tile);
	DrawRect(canvas, rect, color);
}

static B32 TilesAreAdjacent(IV2 tile1, IV2 tile2)
{
	I32 row_abs = IntAbs(tile1.row - tile2.row);
	I32 col_abs = IntAbs(tile1.col - tile2.col);
	B32 are_adjacent = ((row_abs == 0 && col_abs == 1) || (row_abs == 1 && col_abs == 0));
	return are_adjacent;
}

#define MapItemRadius 0.2f

static void
func DrawMapItem(Canvas *canvas, MapItem *item)
{
	V4 item_color = MakeColor(0.5f, 0.0f, 0.5f);
	DrawCircle(canvas, item->position, MapItemRadius, item_color);
}

static void
func DrawMapWithoutItems(Canvas *canvas, Map *map)
{
	Camera *camera = canvas->camera;
	R32 camera_left   = CameraLeftSide(camera);
	R32 camera_right  = CameraRightSide(camera);
	R32 camera_top    = CameraTopSide(camera);
	R32 camera_bottom = CameraBottomSide(camera);

	R32 map_left   = 0.0f;
	R32 map_right  = GetMapWidth(map);
	R32 map_top    = 0.0f;
	R32 map_bottom = GetMapHeight(map);

	for(I32 row = 0; row < map->tile_row_n; row++)
	{
		R32 tile_top    = map_top + MapTileSide * row;
		R32 tile_bottom = tile_top + MapTileSide;
		if(tile_bottom < camera_top || tile_top > camera_bottom)
		{
			continue;
		}

		for(I32 col = 0; col < map->tile_col_n; col++)
		{
			R32 tile_left  = map_left + MapTileSide * col;
			R32 tile_right = tile_left + MapTileSide;
			if(tile_right < camera_left || tile_left > camera_right)
			{
				continue;
			}

			V4 color = GetTileColor(map, row, col);
			DrawRectLRTB(canvas, tile_left, tile_right, tile_top, tile_bottom, color);
		}
	}
}

#define MapVersion 3

static Map
func ReadMapFromFile(I8 *file_path, MemArena *arena)
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

	I8 *base = arena->base_address;

	I8 *position = base;
	I32 version = *(I32 *)position;
	Assert(version == MapVersion);

	position += sizeof(I32);

	Map *map = (Map *)position;
	map->tile_types = (TileId *)GetAbsoluteAddress(map->tile_types, base);
	map->items = (MapItem *)GetAbsoluteAddress(map->items, base);
	map->entities = (MapEntity *)GetAbsoluteAddress(map->entities, base);

	position += sizeof(Map);
	position += map->tile_row_n * map->tile_col_n * sizeof(TileId);
	position += map->item_n * sizeof(MapItem);
	position += map->entity_n * sizeof(MapEntity);

	I8 *arena_top = GetArenaTop(arena);
	Assert(position == arena_top);

	result = CloseHandle(file);
	Assert(result);

	return *map;
}