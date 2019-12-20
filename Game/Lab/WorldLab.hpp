#pragma once

#include <Windows.h>

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Geometry.hpp"
#include "../Map.hpp"
#include "../UserInput.hpp"

#define WorldLabArenaSize (1 * MegaByte)

enum WorldEditMode
{
	PlaceTileMode,
	PlaceItemMode,
	RemoveItemMode,
	PlaceEntityMode
};

struct WorldLabState
{
	Int8 arena_memory1[WorldLabArenaSize];
	Int8 arena_memory2[WorldLabArenaSize];
	MemArena arena1;
	MemArena arena2;
	MemArena *arena;
	MemArena *tmp_arena;

	Map map;

	WorldEditMode edit_mode;
};

static void
func DrawMapItems(Canvas *canvas, Map *map)
{
	for(Int32 i = 0; i < map->item_n; i++)
	{
		DrawMapItem(canvas, &map->items[i]);
	}
}

static void
func DrawMapEntities(Canvas *canvas, Map *map)
{
	Vec4 color = MakeColor(1.0f, 0.5f, 0.0f);
	Real32 radius = 0.5f;
	for(Int32 i = 0; i < map->entity_n; i++)
	{
		MapEntity *entity = &map->entities[i];
		DrawCircle(canvas, entity->spawn_position, radius, color);
	}
}

static void
func DrawMapWithItems(Canvas *canvas, Map *map)
{
	DrawMapWithoutItems(canvas, map);
	DrawMapItems(canvas, map);
	DrawMapEntities(canvas, map);
}

static void
func WorldLabSwapArenas(WorldLabState *lab_state)
{
	MemArena *arena = lab_state->arena;
	lab_state->arena = lab_state->tmp_arena;
	lab_state->tmp_arena = arena;

	Assert((lab_state->arena == &lab_state->arena1) && (lab_state->tmp_arena == &lab_state->arena2) ||
		   (lab_state->arena == &lab_state->arena2) && (lab_state->tmp_arena == &lab_state->arena1));
}

static void
func WorldLabInit(WorldLabState *lab_state, Canvas* canvas)
{
	lab_state->arena1 = CreateMemArena(lab_state->arena_memory1, WorldLabArenaSize);
	lab_state->arena2 = CreateMemArena(lab_state->arena_memory2, WorldLabArenaSize);
	lab_state->arena = &lab_state->arena1;
	lab_state->tmp_arena = &lab_state->arena2;

	lab_state->map = {};

	lab_state->edit_mode = PlaceTileMode;

	Camera *camera = canvas->camera;
	camera->unit_in_pixels = 100.0f;
	camera->center = MakePoint(0.0f, 0.0f);
}

static Bool32
func ArenaContainsMapData(MemArena *arena, Map *map)
{
	Bool32 contains_tile_types = ArenaContainsAddress(arena, map->tile_types);
	Bool32 contains_items = ArenaContainsAddress(arena, map->items);
	Bool32 contains_entities = ArenaContainsAddress(arena, map->entities);
	Bool32 contains_data = (contains_tile_types && contains_items && contains_entities);
	return contains_data;
}

static void
func CheckConsistency(WorldLabState *lab_state)
{
	Assert(ArenaContainsMapData(lab_state->arena, &lab_state->map));
}

static void
func HandlePlaceTileMode(WorldLabState *lab_state, Canvas *canvas, UserInput *user_input)
{
	MemArena *arena = lab_state->arena;
	MemArena *tmp_arena = lab_state->tmp_arena;
	Map *map = &lab_state->map;

	Camera *camera = canvas->camera;
	Vec2 mouse_position = PixelToUnit(camera, user_input->mouse_pixel_position);

	IntVec2 tile = {};
	tile.row = Floor(mouse_position.y / MapTileSide);
	tile.col = Floor(mouse_position.x / MapTileSide);

	if(WasKeyReleased(user_input, VK_LBUTTON))
	{
		ArenaReset(tmp_arena);

		if(map->tile_row_n == 0)
		{
			Assert(map->tile_col_n == 0);
			Assert(map->tile_types == 0);
			map->tile_row_n = 1;
			map->tile_col_n = 1;
			map->tile_types = ArenaAllocArray(tmp_arena, TileId, 1);

			IntVec2 new_tile = MakeIntPoint(0, 0);
			SetTileType(map, new_tile, CaveTileId);

			camera->center.y -= tile.row * MapTileSide;
			camera->center.x -= tile.col * MapTileSide;
		}
		else
		{
			Assert(map->tile_col_n > 0);
			Assert(map->tile_row_n > 0);

			Int32 new_row_n = map->tile_row_n;
			Int32 new_col_n = map->tile_col_n;

			if(tile.row < 0)
			{
				new_row_n += (-tile.row);
			}
			else if(tile.row >= map->tile_row_n)
			{
				new_row_n += (tile.row - map->tile_row_n + 1);
			}

			if(tile.col < 0)
			{
				new_col_n += (-tile.col);
			}
			else if(tile.col >= map->tile_col_n)
			{
				new_col_n += (tile.col - map->tile_col_n + 1);
			}

			TileId *new_tiles = ArenaAllocArray(tmp_arena, TileId, new_row_n * new_col_n);
			for(Int32 index = 0; index < new_row_n * new_col_n; index++)
			{
				new_tiles[index] = NoTileId;
			}

			for(Int32 row = 0; row < map->tile_row_n; row++)
			{
				for(Int32 col = 0; col < map->tile_col_n; col++)
				{
					Int32 old_index = map->tile_col_n * row + col;
					Int32 new_row = row;
					if(tile.row < 0)
					{
						new_row += (-tile.row);
					}

					Int32 new_col = col;
					if(tile.col < 0)
					{
						new_col += (-tile.col);
					}

					Int32 new_index = new_col_n * new_row + new_col;
					new_tiles[new_index] = map->tile_types[old_index];
				}
			}

			map->tile_row_n = new_row_n;
			map->tile_col_n = new_col_n;
			map->tile_types = new_tiles;

			if(tile.row < 0)
			{
				camera->center.y += (-tile.row) * MapTileSide;
			}
			if(tile.col < 0)
			{
				camera->center.x += (-tile.col) * MapTileSide;
			}

			tile.row = IntMax2(tile.row, 0);
			tile.col = IntMax2(tile.col, 0);
			SetTileType(map, tile, CaveTileId);

		}

		WorldLabSwapArenas(lab_state);

		MapItem *old_items = map->items;
		map->items = ArenaAllocArray(arena, MapItem, map->item_n);
		for(Int32 i = 0; i < map->item_n; i++)
		{
			map->items[i] = old_items[i];
		}
	}
	
	Rect tile_rect = {};
	tile_rect.left   = tile.col * MapTileSide;
	tile_rect.right  = tile_rect.left + MapTileSide;
	tile_rect.top    = tile.row * MapTileSide;
	tile_rect.bottom = tile_rect.top + MapTileSide;

	Vec4 tile_color = MakeColor(0.5f, 0.5f, 0.5f);
	DrawRect(canvas, tile_rect, tile_color);
}

static void
func HandlePlaceItemMode(WorldLabState *lab_state, Canvas *canvas, UserInput *user_input)
{
	Map *map = &lab_state->map;
	Camera *camera = canvas->camera;
	Vec2 mouse_position = PixelToUnit(camera, user_input->mouse_pixel_position);

	Real32 item_radius = 0.2f;
	Vec4 item_color = MakeColor(1.0f, 0.0f, 1.0f);

	DrawCircle(canvas, mouse_position, item_radius, item_color);

	if(WasKeyReleased(user_input, VK_LBUTTON))
	{
		Assert(ArenaContainsMapData(lab_state->arena, map));

		MemArena *tmp_arena = lab_state->tmp_arena;

		ArenaReset(tmp_arena);

		Int32 tile_n = map->tile_row_n * map->tile_col_n;
		TileId *new_tile_types = ArenaAllocArray(tmp_arena, TileId, tile_n);
		for(Int32 i = 0; i < tile_n; i++)
		{
			new_tile_types[i] = map->tile_types[i];
		}
		map->tile_types = new_tile_types;

		MapItem *new_items = ArenaAllocArray(tmp_arena, MapItem, map->item_n + 1);
		for(Int32 i = 0; i < map->item_n; i++)
		{
			new_items[i] = map->items[i];
		}
		MapItem *new_item = &new_items[map->item_n];
		new_item->item_id = NoItemId;
		new_item->position = mouse_position;
		map->item_n++;
		map->items = new_items;

		MapEntity *new_entities = ArenaAllocArray(tmp_arena, MapEntity, map->entity_n);
		for(Int32 i = 0; i < map->entity_n; i++)
		{
			new_entities[i] = map->entities[i];
		}
		map->entities = new_entities;

		Assert(ArenaContainsMapData(tmp_arena, map));

		WorldLabSwapArenas(lab_state);
		CheckConsistency(lab_state);
	}
}

static void
func HandleRemoveItemMode(WorldLabState *lab_state, Canvas *canvas, UserInput *user_input)
{
	Vec2 mouse_position = PixelToUnit(canvas->camera, user_input->mouse_pixel_position);

	Map *map = &lab_state->map;
	MapItem *hover_item = 0;
	Int32 hover_index = -1;
	for(Int32 i = 0; i < map->item_n; i++)
	{
		MapItem *item = &map->items[i];
		Real32 distance = Distance(item->position, mouse_position);
		if(distance <= MapItemRadius)
		{
			hover_item = item;
			hover_index = i;
			break;
		}
	}

	if(hover_item)
	{
		Assert(IsIntBetween(hover_index, 0, map->item_n - 1));

		Vec4 item_color = MakeColor(0.2f, 0.2f, 0.2f);
		DrawCircle(canvas, hover_item->position, MapItemRadius, item_color);
		if(WasKeyReleased(user_input, VK_LBUTTON))
		{
			MemArena *tmp_arena = lab_state->tmp_arena;
			
			ArenaReset(tmp_arena);

			Int32 tile_n = map->tile_row_n * map->tile_col_n;
			TileId *new_tile_types = ArenaAllocArray(tmp_arena, TileId, tile_n);
			for(Int32 i = 0; i < tile_n; i++)
			{
				new_tile_types[i] = map->tile_types[i];
			}
			map->tile_types = new_tile_types;

			Int32 item_index = 0;
			MapItem *new_items = ArenaAllocArray(tmp_arena, MapItem, map->item_n - 1);
			for(Int32 i = 0; i < map->item_n; i++)
			{
				if(i != hover_index)
				{
					new_items[item_index] = map->items[i];
					item_index++;
				}
			}
			map->item_n--;
			Assert(item_index == map->item_n);
			map->items = new_items;

			MapEntity *new_entities = ArenaAllocArray(tmp_arena, MapEntity, map->entity_n);
			for(Int32 i = 0; i < map->entity_n; i++)
			{
				new_entities[i] = map->entities[i];
			}
			map->entities = new_entities;

			Assert(ArenaContainsMapData(tmp_arena, map));

			WorldLabSwapArenas(lab_state);
			CheckConsistency(lab_state);
		}
	}
}

static void
func HandlePlaceEntityMode(WorldLabState *lab_state, Canvas *canvas, UserInput *user_input)
{
	Map *map = &lab_state->map;
	Camera *camera = canvas->camera;
	Vec2 mouse_position = PixelToUnit(camera, user_input->mouse_pixel_position);

	Real32 entity_radius = 0.5f;
	Vec4 entity_color = MakeColor(1.0f, 0.5f, 0.0f);;

	DrawCircle(canvas, mouse_position, entity_radius, entity_color);

	if(WasKeyReleased(user_input, VK_LBUTTON))
	{
		MemArena *tmp_arena = lab_state->tmp_arena;
		ArenaReset(tmp_arena);

		Int32 tile_n = map->tile_row_n * map->tile_col_n;
		TileId *new_tile_types = ArenaAllocArray(tmp_arena, TileId, tile_n);
		for(Int32 i = 0; i < tile_n; i++)
		{
			new_tile_types[i] = map->tile_types[i];
		}
		map->tile_types = new_tile_types;

		MapItem *new_items = ArenaAllocArray(tmp_arena, MapItem, map->item_n);
		for(Int32 i = 0; i < map->item_n; i++)
		{
			new_items[i] = map->items[i];
		}
		map->items = new_items;

		MapEntity *new_entities = ArenaAllocArray(tmp_arena, MapEntity, map->entity_n + 1);
		for(Int32 i = 0; i < map->entity_n; i++)
		{
			new_entities[i] = map->entities[i];
		}
		MapEntity *new_entity = &new_entities[map->entity_n];
		new_entity->group_id = OrangeGroupId;
		new_entity->spawn_position = mouse_position;
		map->entity_n++;
		map->entities = new_entities;

		Assert(ArenaContainsMapData(tmp_arena, map));

		WorldLabSwapArenas(lab_state);
		CheckConsistency(lab_state);
	}
}

static Int8 *map_file = "Data/Map.data";

static void
func WorldLabUpdate(WorldLabState *lab_state, Canvas *canvas, Real32 seconds, UserInput *user_input)
{
	Bitmap *bitmap = &canvas->bitmap;
	Vec4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, background_color);

	Real32 camera_move_speed = seconds * 50.0f;
	Camera *camera = canvas->camera;

	if(IsKeyDown(user_input, 'A'))
	{
		camera->center.x -= camera_move_speed;
	}
	if(IsKeyDown(user_input, 'D'))
	{
		camera->center.x += camera_move_speed;
	}

	if(IsKeyDown(user_input, 'W'))
	{
		camera->center.y -= camera_move_speed;
	}
	if(IsKeyDown(user_input, 'S'))
	{
		camera->center.y += camera_move_speed;
	}

	if(IsKeyDown(user_input, 'Q'))
	{
		camera->unit_in_pixels /= 1.01f;
	}
	if(IsKeyDown(user_input, 'E'))
	{
		camera->unit_in_pixels *= 1.01f;
	}

	if(WasKeyPressed(user_input, '1'))
	{
		lab_state->edit_mode = PlaceTileMode;
	}
	if(WasKeyPressed(user_input, '2'))
	{
		lab_state->edit_mode = PlaceItemMode;
	}
	if(WasKeyPressed(user_input, '3'))
	{
		lab_state->edit_mode = RemoveItemMode;
	}
	if(WasKeyPressed(user_input, '4'))
	{
		lab_state->edit_mode = PlaceEntityMode;
	}

	MemArena *arena = lab_state->arena;
	MemArena *tmp_arena = lab_state->tmp_arena;
	if(WasKeyReleased(user_input, 'M'))
	{
		CheckConsistency(lab_state);

		HANDLE file = CreateFileA(map_file, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		Assert(file != INVALID_HANDLE_VALUE);

		MemArena *file_arena = tmp_arena;
		ArenaReset(file_arena);
		Int32 version = MapVersion;
		ArenaPushVar(file_arena, version);

		Map *map = &lab_state->map;
		Map *pushed_map = (Map *)ArenaPushVar(file_arena, *map);
		Int32 tile_n = (map->tile_row_n * map->tile_col_n);

		TileId *tile_types = (TileId *)ArenaPushData(file_arena, tile_n * sizeof(TileId), map->tile_types);
		pushed_map->tile_types = (TileId *)GetRelativeAddress(tile_types, file_arena->base_address);

		MapItem *items = (MapItem *)ArenaPushData(file_arena, map->item_n * sizeof(MapItem), map->items);
		pushed_map->items = (MapItem *)GetRelativeAddress(items, file_arena->base_address);

		MapEntity *entities = (MapEntity *)ArenaPushData(file_arena, map->entity_n * sizeof(MapEntity), map->entities);
		pushed_map->entities = (MapEntity *)GetRelativeAddress(entities, file_arena->base_address);

		Int8 *data = file_arena->base_address;
		DWORD data_size = file_arena->used_size;
		DWORD written_data_size = 0;

		BOOL result = WriteFile(file, (LPCVOID)data, (DWORD)data_size, &written_data_size, 0);
		Assert(result);
		Assert(written_data_size == data_size);

		result = CloseHandle(file);
		Assert(result);
	}

	if(WasKeyReleased(user_input, 'L'))	
	{
		lab_state->map = ReadMapFromFile(map_file, arena);
	}

	Map *map = &lab_state->map;
	DrawMapWithItems(canvas, map);

	if(map->tile_row_n > 0)
	{
		Assert(map->tile_col_n > 0);
		Rect rect = {};
		rect.left = 0.0f;
		rect.right = GetMapWidth(map);
		rect.top = 0.0f;
		rect.bottom = GetMapHeight(map);

		Vec4 border_color = MakeColor(1.0f, 1.0f, 0.0);
		DrawRectOutline(canvas, rect, border_color);
	}
	
	switch(lab_state->edit_mode)
	{
		case PlaceTileMode:
		{
			HandlePlaceTileMode(lab_state, canvas, user_input);
			break;
		}
		case PlaceItemMode:
		{
			HandlePlaceItemMode(lab_state, canvas, user_input);
			break;
		}
		case RemoveItemMode:
		{
			HandleRemoveItemMode(lab_state, canvas, user_input);
			break;
		}
		case PlaceEntityMode:
		{
			HandlePlaceEntityMode(lab_state, canvas, user_input);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
}