﻿#pragma once

#include "Item.hpp"
#include "Map.hpp"
#include "UserInput.hpp"

#define GameArenaSize (1 * MegaByte)

struct SubTile
{
	IV2 tile_index;
	IV2 sub_index;
};

#define SubTileSideN 8
#define SubTileSide ((MapTileSide) / ((R32)(SubTileSideN)))

static SubTile
func MakeSubTile(I32 row, I32 col, I32 sub_row, I32 sub_col)
{
	SubTile sub_tile = {};
	sub_tile.tile_index = MakeIntPoint(row, col);
	sub_tile.sub_index = MakeIntPoint(sub_row, sub_col);
	return sub_tile;
}

static B32
func IsValidSubTile(Map *map, SubTile sub_tile)
{
	B32 is_valid_tile_index = IsValidTile(map, sub_tile.tile_index);

	B32 is_valid_subtile_row = IsIntBetween(sub_tile.sub_index.row, 0, SubTileSideN);
	B32 is_valid_subtile_col = IsIntBetween(sub_tile.sub_index.col, 0, SubTileSideN);
	B32 is_valid_subtile_index = (is_valid_subtile_row && is_valid_subtile_col);

	B32 is_valid = (is_valid_tile_index && is_valid_subtile_index);
	return is_valid;
}

static SubTile
func OffsetSubTile(SubTile tile, IV2 offset)
{
	SubTile result = tile;
	result.sub_index.row += offset.row;
	result.sub_index.col += offset.col;

	result.tile_index.row += (result.sub_index.row / SubTileSideN);
	result.sub_index.row %= SubTileSideN;
	if(result.sub_index.row < 0)
	{
		result.tile_index.row--;
		result.sub_index.row += SubTileSideN;
	}

	result.tile_index.col += (result.sub_index.col / SubTileSideN);
	result.sub_index.col %= SubTileSideN;
	if(result.sub_index.col < 0)
	{
		result.tile_index.col--;
		result.sub_index.col += SubTileSideN;
	}

	Assert(IsIntBetween(result.sub_index.row, 0, SubTileSideN - 1));
	Assert(IsIntBetween(result.sub_index.col, 0, SubTileSideN - 1));
	return result;
}

static V2
func GetSubTileCenter(Map *map, SubTile sub_tile)
{
	Assert(IsValidSubTile(map, sub_tile));

	V2 tile_center = GetTileCenter(map, sub_tile.tile_index);
	V2 tile_top_left = tile_center - 0.5f * MakePoint(MapTileSide, MapTileSide);

	V2 sub_tile_center = {};
	sub_tile_center.x = tile_top_left.x + (sub_tile.sub_index.col * SubTileSide) + (0.5f * SubTileSide);
	sub_tile_center.y = tile_top_left.y + (sub_tile.sub_index.row * SubTileSide) + (0.5f * SubTileSide);
	return sub_tile_center;
}

static Rect
func GetSubTileRect(Map *map, SubTile sub_tile)
{
	Assert(IsValidSubTile(map, sub_tile));

	V2 center = GetSubTileCenter(map, sub_tile);
	Rect rect = MakeSquareRect(center, SubTileSide);
	return rect;
}

static void
func DrawSubTile(Canvas *canvas, Map *map, SubTile sub_tile, V4 color)
{
	Rect rect = GetSubTileRect(map, sub_tile);
	DrawRect(canvas, rect, color);
}

static void
func DrawSubTileOutline(Canvas *canvas, Map *map, SubTile sub_tile, V4 color)
{
	Rect rect = GetSubTileRect(map, sub_tile);
	DrawRectOutline(canvas, rect, color);
}

struct Entity
{
	V2 position;
	V2 velocity;

	I32 health_points;
	I32 max_health_points;

	V2 start_position;
	R32 resurrect_time;

	Entity *target;

	R32 recharge_time;
	EntityGroupId group_id;
};

static B32
func SubTilesAreEqual(SubTile a, SubTile b)
{
	B32 tile_index_equal = (a.tile_index.row == b.tile_index.row && a.tile_index.col == b.tile_index.col);
	B32 sub_index_equal = (a.sub_index.row == b.sub_index.row && a.sub_index.col == b.sub_index.col);
	B32 equal = (tile_index_equal && sub_index_equal);
	return equal;
}

static SubTile
func GetContainingSubTile(Map *map, V2 point)
{
	IV2 tile_index = GetContainingTile(map, point);
	Rect tile_rect = GetTileRect(map, tile_index);
	I32 row = Floor((point.y - tile_rect.top) / SubTileSide);
	I32 col = Floor((point.x - tile_rect.left) / SubTileSide);
	Assert(IsIntBetween(row, 0, SubTileSideN - 1));
	Assert(IsIntBetween(col, 0, SubTileSideN - 1));
	
	SubTile sub_tile;
	sub_tile.tile_index = tile_index;
	sub_tile.sub_index = MakeIntPoint(row, col);
	return sub_tile;
}

#define MaxEntityN 1024

struct Game
{
	I8 arena_memory[GameArenaSize];
	MemArena arena;
	Map map;

	Inventory inventory;
	B32 show_inventory;

	Inventory trade_inventory;
	B32 show_trade_window;

	Entity entities[MaxEntityN];
	I32 entity_n;

	Entity *player;

	R32 *item_spawn_cooldowns;
};

static B32
func SubTileIsOccupied(Game *game, SubTile sub_tile)
{
	Map *map = &game->map;
	Assert(IsValidSubTile(map, sub_tile));

	B32 is_occupied = false;
	for(I32 i = 0; i < game->entity_n; i++)
	{
		Entity *entity = &game->entities[i];
		SubTile entity_sub_tile = GetContainingSubTile(map, entity->position);
		if(SubTilesAreEqual(entity_sub_tile, sub_tile))
		{
			is_occupied = true;
			break;
		}
	}

	return is_occupied;
}

static Entity *
func AddEntity(Game *game, Entity entity)
{
	Assert(game->entity_n < MaxEntityN);
	Entity *result = &game->entities[game->entity_n];
	game->entity_n++;
	*result = entity;
	return result;
}

static Entity *
func AddPlayer(Game *game, Entity entity)
{
	Assert(game->player == 0);
	Entity *result = AddEntity(game, entity);
	game->player = result;
	return result;
}

static void
func InitNpc(Entity *npc, EntityGroupId group_id, V2 position)
{
	I32 health_points = 0;
	switch(group_id)
	{
		case OrangeGroupId:
		{
			health_points = 100;
			break;
		}
		case PurpleGroupId:
		{
			health_points = 10;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	npc->group_id = group_id;
	npc->position = position;
	npc->start_position = position;
	npc->velocity = MakePoint(0.0f, 0.0f);
	npc->max_health_points = health_points;
	npc->health_points = health_points;
}

static EntityGroupId
func GetRandomGroupId()
{
	EntityGroupId id = NeutralGroupId;
	I32 random = IntRandom(0, 1);
	if(random == 0)
	{
		id = OrangeGroupId;
	}
	else if(random == 1)
	{
		id = PurpleGroupId;
	}
	else
	{
		DebugBreak();
	}

	return id;
}

static void
func GameInit(Game *game, Canvas *canvas)
{
	game->arena = CreateMemArena(game->arena_memory, GameArenaSize);

	Camera *camera = canvas->camera;
	camera->unit_in_pixels = 30;
	camera->center = MakePoint(0.0, 0.0);

	Entity player = {};
	player.position = MakePoint(0.5f * MapTileSide, 0.5f * MapTileSide);
	player.velocity = MakeVector(0.0f, 0.0f);
	player.max_health_points = 20;
	player.health_points = player.max_health_points;
	player.group_id = OrangeGroupId;
	AddPlayer(game, player);

	I8 *map_file = "Data/Map.data";
	game->map = ReadMapFromFile(map_file, &game->arena); 
	game->item_spawn_cooldowns = ArenaAllocArray(&game->arena, R32, game->map.item_n);

	for(I32 i = 0; i < game->map.item_n; i++)
	{
		game->item_spawn_cooldowns[i] = 0.0f;
	}

	canvas->glyph_data = GetGlobalGlyphData();

	InitInventory(&game->inventory, &game->arena, 3, 5);
	game->show_inventory = false;

	InitInventory(&game->trade_inventory, &game->arena, 3, 5);
	game->show_trade_window = false;

	Map *map = &game->map;
	for(I32 i = 0; i < map->entity_n; i++)
	{
		MapEntity *map_entity = &map->entities[i];
		Entity npc = {};
		InitNpc(&npc, map_entity->group_id, map_entity->spawn_position);
		AddEntity(game, npc);
	}
}

#define EntityRadius 0.5f
#define EntitySide (2.0f * EntityRadius)

static Rect
func GetEntityRect(Entity *entity)
{
	Rect rect = MakeSquareRect(entity->position, EntitySide);
	return rect;
}

static V2
func GetEntityLeft(Entity *entity)
{
	V2 left = entity->position + MakeVector(-EntityRadius, 0.0f);
	return left;
}

static V2
func GetEntityRight(Entity *entity)
{
	V2 right = entity->position + MakeVector(+EntityRadius, 0.0f);
	return right;
}

static V2
func GetEntityTop(Entity *entity)
{
	V2 top = entity->position + MakeVector(0.0f, -EntityRadius);
	return top;
}

static V2
func GetEntityBottom(Entity *entity)
{
	V2 bottom = entity->position + MakeVector(0.0f, +EntityRadius);
	return bottom;
}

static B32
func IsDead(Entity *entity)
{
	B32 is_dead = (entity->health_points == 0);
	return is_dead;
}

static B32
func IsAlive(Entity *entity)
{
	B32 is_alive = (entity->health_points > 0);
	return is_alive;
}

static V2
func GetUpdatedEntityPosition(Game *game, Entity *entity, R32 seconds)
{
	V2 old_position = entity->position;
	V2 new_position = old_position;

	if(IsAlive(entity))
	{
		Map *map = &game->map;

		V2 move_vector = seconds * entity->velocity;
		new_position = entity->position + move_vector;

		IV2 top_tile = GetContainingTile(map, GetEntityTop(entity));
		top_tile.row--;

		IV2 bottom_tile = GetContainingTile(map, GetEntityBottom(entity));
		bottom_tile.row++;

		IV2 left_tile = GetContainingTile(map, GetEntityLeft(entity));
		left_tile.col--;

		IV2 right_tile = GetContainingTile(map, GetEntityRight(entity));
		right_tile.col++;

		for(I32 row = top_tile.row; row <= bottom_tile.row; row++)
		{
			for(I32 col = left_tile.col; col <= right_tile.col; col++)
			{
				IV2 tile = MakeTile(row, col);
				if(IsValidTile(map, tile) && IsTileType(map, tile, NoTileId))
				{
					Poly16 collision_poly = {};
					Rect tile_rect = GetTileRect(map, tile);
					Rect collision_rect = GetExtendedRect(tile_rect, EntityRadius);

					if(IsBetween(old_position.x, collision_rect.left, collision_rect.right))
					{
						if(old_position.y <= collision_rect.top && new_position.y > collision_rect.top)
						{
							new_position.y = collision_rect.top;
						}

						if(old_position.y >= collision_rect.bottom && new_position.y < collision_rect.bottom)
						{
							new_position.y = collision_rect.bottom;
						}
					}

					if(IsBetween(old_position.y, collision_rect.top, collision_rect.bottom))
					{
						if(old_position.x <= collision_rect.left && new_position.x > collision_rect.left)
						{
							new_position.x = collision_rect.left;
						}

						if(old_position.x >= collision_rect.right && new_position.x < collision_rect.right)
						{
							new_position.x = collision_rect.right;
						}
					}
				}
			}
		}

		R32 map_width  = GetMapWidth(map);
		R32 map_height = GetMapHeight (map);
		new_position.x = Clip(new_position.x, EntityRadius, map_width - EntityRadius);
		new_position.y = Clip(new_position.y, EntityRadius, map_height - EntityRadius);
	}

	return new_position;
}

static void
func UpdateEntityMovementWithSubTileCollision(Game *game, Entity *entity, R32 seconds)
{
	Map *map = &game->map;
	V2 new_position = GetUpdatedEntityPosition(game, entity, seconds);

	SubTile old_sub_tile = GetContainingSubTile(map, entity->position);
	SubTile new_sub_tile = GetContainingSubTile(map, new_position);

	B32 can_move = false;
	if(SubTilesAreEqual(old_sub_tile, new_sub_tile))
	{
		can_move = true;
	}
	else
	{
		can_move = (!SubTileIsOccupied(game, new_sub_tile));
	}

	if(can_move)
	{
		entity->position = new_position;
	}
}

static void
func UpdateEntityMovementWithoutSubTileCollision(Game *game, Entity *entity, R32 seconds)
{
	V2 new_position = GetUpdatedEntityPosition(game, entity, seconds);
	entity->position = new_position;
}

static void
func DrawInteractionDialogWithText(Canvas *canvas, I8 *text)
{
	V4 color = MakeColor(0.8f, 0.8f, 0.4f);
	V4 outline_color = MakeColor(1.0f, 1.0f, 0.5f);

	I32 border = 30;
	Bitmap *bitmap = &canvas->bitmap;
	IntRect rect = {};
	rect.bottom = bitmap->height - border;
	rect.left = border;
	rect.right = bitmap->width - border;
	rect.top = rect.bottom - 200;
	DrawBitmapRect(bitmap, rect, color);
	DrawBitmapRectOutline(bitmap, rect, outline_color);

	V4 text_color = MakeColor(0.0f, 0.0f, 0.0f);
	I32 text_left = rect.left + 10;
	I32 text_top = rect.top + 10;

	DrawBitmapTextLineTopLeft(bitmap, text, canvas->glyph_data, text_left, text_top, text_color);
}

#define InventorySlotSide 50
#define InventorySlotPadding 2

static IntRect
func GetInventorySlotRect(Inventory *inventory, IV2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	IntRect rect = {};
	rect.left   = (inventory->left + InventorySlotPadding) + slot.col * (InventorySlotSide + InventorySlotPadding);
	rect.right  = rect.left + InventorySlotSide;
	rect.top    = (inventory->top + InventorySlotPadding) + slot.row * (InventorySlotSide + InventorySlotPadding);
	rect.bottom = rect.top + InventorySlotSide;
	return rect;
}

static void
func DrawInventorySlot(Canvas *canvas, Inventory *inventory, IV2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	ItemId item_id = GetInventoryItemId(inventory, slot);
	SlotId slot_id = GetInventorySlotId(inventory, slot);
	IntRect slot_rect = GetInventorySlotRect(inventory, slot);

	V4 slot_background_color = MakeColor(0.0f, 0.0f, 0.0f);
	V4 item_background_color = MakeColor(0.1f, 0.1f, 0.1f);
	V4 cooldown_color = MakeColor(0.2f, 0.0f, 0.0f);
	V4 item_name_color = MakeColor(1.0f, 1.0f, 1.0f);
	V4 slot_name_color = MakeColor(0.3f, 0.3f, 0.3f);

	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;

	if(item_id == NoItemId)
	{
		DrawBitmapRect(bitmap, slot_rect, slot_background_color);
		if(slot_id != AnySlotId)
		{
			I8 *slot_name = GetSlotName(slot_id);
			Assert(slot_name != 0);
			DrawBitmapTextLineCentered(bitmap, slot_name, glyph_data, slot_rect, slot_name_color);
		}
	}
	else
	{
		DrawBitmapRect(bitmap, slot_rect, item_background_color);
		I8 *name = GetItemSlotName(item_id);
		DrawBitmapTextLineCentered(bitmap, name, glyph_data, slot_rect, item_name_color);
	}
}

static void
func DrawInventorySlotOutline(Canvas *canvas, Inventory *inventory, IV2 slot, V4 color)
{
	Bitmap *bitmap = &canvas->bitmap;
	IntRect rect = GetInventorySlotRect(inventory, slot);
	DrawBitmapRectOutline(bitmap, rect, color);
}

#define InventorySlotPadding 2

static I32
func GetInventoryWidth(Inventory *inventory)
{
	I32 width = InventorySlotPadding + inventory->col_n * (InventorySlotSide + InventorySlotPadding);
	return width;
}

static I32
func GetInventoryHeight(Inventory *inventory)
{
	I32 height = InventorySlotPadding + inventory->row_n * (InventorySlotSide + InventorySlotPadding);
	return height;
}

static void
func MoveInventoryToRight(Inventory *inventory, I32 right)
{
	I32 width = GetInventoryWidth(inventory);
	inventory->left = right - width;
}

static void
func MoveInventoryToBottom(Inventory *inventory, I32 bottom)
{
	I32 height = GetInventoryHeight(inventory);
	inventory->top = bottom - height;
}

static void
func MoveInventoryToBottomRight(Inventory *inventory, I32 bottom, I32 right)
{
	MoveInventoryToBottom(inventory, bottom);
	MoveInventoryToRight(inventory, right);
}

static void
func MoveInventoryToBottomLeft(Inventory *inventory, I32 bottom, I32 left)
{
	MoveInventoryToBottom(inventory, bottom);
	inventory->left = left;
}

#define UIBoxSide 40
#define UIBoxPadding 5

static IntRect
func GetInventoryRect(Inventory *inventory)
{
	I32 width = GetInventoryWidth(inventory);
	I32 height = GetInventoryHeight(inventory);

	IntRect rect = {};
	rect.left   = inventory->left;
	rect.right  = rect.left + width;
	rect.top    = inventory->top;
	rect.bottom = rect.top + height;

	return rect;
}

static void
func DrawInventory(Canvas *canvas, Inventory *inventory)
{
	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;
	Assert(glyph_data != 0);

	IntRect rect = GetInventoryRect(inventory);

	V4 background_color = MakeColor(0.5f, 0.5f, 0.5f);
	V4 hover_outline_color = MakeColor(1.0f, 1.0f, 0.0f);
	V4 invalid_outline_color = MakeColor(1.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, background_color);

	for(I32 row = 0; row < inventory->row_n; row++)
	{
		for(I32 col = 0; col < inventory->col_n; col++)
		{
			IV2 slot = MakeIntPoint(row, col);
			SlotId slot_id = GetInventorySlotId(inventory, slot);
			ItemId item_id = GetInventoryItemId(inventory, slot);
			DrawInventorySlot(canvas, inventory, slot);
		}
	}
}

static InventoryItem
func GetInventoryItemAtPosition(Inventory *inventory, IV2 position)
{
	InventoryItem item = {};
	for(I32 row = 0; row < inventory->row_n; row++)
	{
		for(I32 col = 0; col < inventory->col_n; col++)
		{
			IV2 slot = MakeIntPoint(row, col);
			ItemId item_id = GetInventoryItemId(inventory, slot);
			IntRect rect = GetInventorySlotRect(inventory, slot);
			if(IsPointInIntRect(position, rect))
			{
				item = GetInventoryItem(inventory, slot);
			}
		}
	}

	return item;
}

static void
func StopTrading(Game *game)
{
	Assert(game->show_trade_window);
	Assert(game->show_inventory);

	Inventory *inventory = &game->inventory;
	Inventory *trade_inventory = &game->trade_inventory;
	for(I32 row = 0; row < trade_inventory->row_n; row++)
	{
		for(I32 col = 0; col < trade_inventory->col_n; col++)
		{
			IV2 slot = MakeIntPoint(row, col);
			InventoryItem item = GetInventoryItem(trade_inventory, slot);
			Assert(item.inventory == trade_inventory);
			if(item.item_id != NoItemId)
			{
				MoveItemToInventory(item, inventory);
			}
		}
	}

	game->show_trade_window = false;
	game->show_inventory = false;
}

static void
func DrawEntity(Canvas *canvas, Entity *entity, V4 color)
{
	Rect rect = GetEntityRect(entity);
	DrawRect(canvas, rect, color);

	if(entity->max_health_points > 0)
	{
		Assert(IsIntBetween(entity->health_points, 0, entity->max_health_points));
		R32 ratio = (R32)entity->health_points / (R32)entity->max_health_points;

		R32 unit_in_pixels = canvas->camera->unit_in_pixels;
		R32 bar_width = 50.0f / unit_in_pixels;
		R32 bar_height = 10.0f / unit_in_pixels;

		Rect bar_rect = {};
		bar_rect.left   = entity->position.x - bar_width * 0.5f;
		bar_rect.right  = entity->position.x + bar_width * 0.5f;
		bar_rect.bottom = rect.top - 10.0f / unit_in_pixels;
		bar_rect.top    = bar_rect.bottom - bar_height;

		V4 bar_background_color = MakeColor(0.5f, 0.5f, 0.5f);
		V4 bar_outline_color = MakeColor(0.0f, 0.0f, 0.0f);

		DrawRect(canvas, bar_rect, bar_background_color);

		V4 filled_bar_color = MakeColor(1.0f, 0.0f, 0.0f);
		Rect filled_bar_rect = bar_rect;
		filled_bar_rect.right = bar_rect.left + (bar_rect.right - bar_rect.left) * ratio;
		DrawRect(canvas, filled_bar_rect, filled_bar_color);

		DrawRectOutline(canvas, bar_rect, bar_outline_color);
	}
}

static void
func HighlightEntity(Canvas *canvas, Entity *entity, V4 color)
{
	Rect rect = GetEntityRect(entity);
	DrawRectOutline(canvas, rect, color);
}

static R32
func ClipUpToZero(R32 value)
{
	R32 result = (value >= 0.0f) ? value : 0.0f;
	return result;
}

static I32
func ClipIntUpToZero(I32 value)
{
	I32 result = (value >= 0) ? value : 0;
	return result;
}

static void
func DoDamage(Entity *target, I32 damage)
{
	/*
	Assert(target->health_points > 0);
	target->health_points = ClipIntUpToZero(target->health_points - damage);
	if(target->health_points == 0)
	{
		target->resurrect_time = 30.0f;
	}
	*/
}

#define MaxAttackDistance 15.0f
#define MaxMeleeDistance 3.0f

static B32
func IsEnemyOf(Entity *a, Entity *b)
{
	B32 is_enemy = (a->group_id != NeutralGroupId && b->group_id != NeutralGroupId && b->group_id != a->group_id);
	return is_enemy;
}

static void
func UpdateNpcTarget(Game *game, Entity *npc)
{
	Assert(npc->health_points > 0);

	Entity *target = npc->target;
	if(target && target->health_points == 0)
	{
		target = 0;
	}

	R32 closest_distance = MaxAttackDistance;
	if(!target)
	{
		for(I32 i = 0; i < game->entity_n; i++)
		{
			Entity *entity = &game->entities[i];
			if(entity != npc)
			{
				B32 is_alive = (entity->health_points > 0);
				B32 is_enemy = IsEnemyOf(npc, entity);
				if(is_alive && is_enemy)
				{
					R32 distance = Distance(npc->position, entity->position);
					if(distance <= closest_distance)
					{
						target = entity;
						closest_distance = distance;
					}
				}
			}
		}
	}

	npc->target = target;
}

static V2
func GetNpcMoveDirection(Game *game, Entity *npc)
{
	V2 direction = {};
	
	Entity *target = npc->target;
	if(target)
	{
		Map *map = &game->map;

		SubTile base_sub_tile = GetContainingSubTile(map, npc->position);

		SubTile top_left_sub_tile = OffsetSubTile(base_sub_tile, MakeIntPoint(-2, -2));

		IV2 visited_offsets[5 * 5] = {};
		I32 visited_n = 0;
		B32 is_visited[5][5] = {};

		IV2 came_from_offset[5][5] = {};

		is_visited[2][2] = true;
		came_from_offset[2][2] = MakeIntPoint(2, 2);
		visited_offsets[visited_n] = MakeIntPoint(2, 2);
		visited_n++;

		V2 target_position = target->position;

		IV2 closest_visited_offset = MakeIntPoint(2, 2);
		R32 closest_distance = Distance(GetSubTileCenter(map, base_sub_tile), target_position);

		for(I32 i = 0; i < visited_n; i++)
		{
			IV2 start_offset = visited_offsets[i];
			IV2 to_visit[4] = 
			{
				start_offset + MakeIntPoint(-1, 0),
				start_offset + MakeIntPoint(+1, 0),
				start_offset + MakeIntPoint(0, -1),
				start_offset + MakeIntPoint(0, +1)
			};

			for(I32 j = 0; j < 4; j++)
			{
				IV2 offset = to_visit[j];
				if(!IsIntBetween(offset.row, 0, 4) || !IsIntBetween(offset.col, 0, 4))
				{
					continue;
				}

				SubTile sub_tile = OffsetSubTile(top_left_sub_tile, offset);
				TileId tile_id = GetTileType(map, sub_tile.tile_index);
				if(tile_id == NoTileId)
				{
					continue;
				}

				if(SubTileIsOccupied(game, sub_tile))
				{
					continue;
				}

				if(is_visited[offset.row][offset.col])
				{
					continue;
				}

				R32 distance = Distance(GetSubTileCenter(map, sub_tile), target_position);
				if(distance < closest_distance)
				{
					closest_distance = distance;
					closest_visited_offset = offset;
				}

				is_visited[offset.row][offset.col] = true;
				came_from_offset[offset.row][offset.col] = start_offset;
				visited_offsets[visited_n] = offset;
				visited_n++;
			}
		}

		SubTile closest_visited_sub_tile = OffsetSubTile(top_left_sub_tile, closest_visited_offset);

		IV2 target_offset = closest_visited_offset;

		IV2 offset = closest_visited_offset;
		while(1)
		{
			Assert(IsIntBetween(offset.row, 0, 4));
			Assert(IsIntBetween(offset.col, 0, 4));
			Assert(is_visited[offset.row][offset.col]);
			IV2 from_offset = came_from_offset[offset.row][offset.col];
			if(offset.row == from_offset.row && offset.col == from_offset.col)
			{
				break;
			}

			target_offset = offset;
			offset = from_offset;
		}

		SubTile target_sub_tile = OffsetSubTile(top_left_sub_tile, target_offset);
		V2 center = GetSubTileCenter(map, target_sub_tile);
		direction = NormalVector(center - npc->position);
	}

	return direction;
}

static void
func UpdateNpc(Game *game, Entity *npc, R32 seconds)
{
	if(npc->health_points > 0)
	{
		npc->recharge_time = ClipUpToZero(npc->recharge_time - seconds);
		npc->velocity = MakeVector(0.0f, 0.0f);

		UpdateNpcTarget(game, npc);

		Entity *target = npc->target;
		if(target)
		{
			R32 distance = Distance(npc->position, target->position);
			if(distance > MaxMeleeDistance)
			{
				V2 direction = GetNpcMoveDirection(game, npc);
				R32 speed = 10.0f;
				npc->velocity = speed * direction;
			}
			else
			{
				if(npc->recharge_time == 0.0f)
				{
					I32 damage = 0;
					switch(npc->group_id)
					{
						case OrangeGroupId:
						{
							damage = 2;
							break;
						}
						case PurpleGroupId:
						{
							damage = 2;
							break;
						}
						default:
						{
							DebugBreak();
						}
					}

					DoDamage(target, damage);
					npc->recharge_time = 3.0f;
				}
			}
		}
	}
	else
	{
		Assert(npc->health_points == 0);
		npc->velocity = MakeVector(0.0f, 0.0f);

		npc->resurrect_time -= seconds;
		if(npc->resurrect_time <= 0.0f)
		{
			npc->position = npc->start_position;
			npc->health_points = npc->max_health_points;
			npc->target = 0;
		}
	}

	UpdateEntityMovementWithSubTileCollision(game, npc, seconds);
}

static void
func GameUpdate(Game *game, Canvas *canvas, R32 seconds, UserInput *user_input)
{
	Bitmap *bitmap = &canvas->bitmap;
	V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, background_color);

	Entity *player = game->player;
	Assert(player != 0);

	R32 player_move_speed = 10.0f;
	player->velocity.x = 0.0f;
	if(IsKeyDown(user_input, 'A'))
	{
		player->velocity.x -= player_move_speed;
	}
	if(IsKeyDown(user_input, 'D'))
	{
		player->velocity.x += player_move_speed;
	}

	player->velocity.y = 0.0f;
	if(IsKeyDown(user_input, 'W'))
	{
		player->velocity.y -= player_move_speed;
	}
	if(IsKeyDown(user_input, 'S'))
	{
		player->velocity.y += player_move_speed;
	}

	if(WasKeyReleased(user_input, 'I'))
	{
		if(game->show_inventory)
		{
			if(game->show_trade_window)
			{
				StopTrading(game);
			}
			game->show_inventory = false;
		}
		else
		{
			game->show_inventory = true;
		}
	}

	Map *map = &game->map;
	UpdateEntityMovementWithoutSubTileCollision(game, player, seconds);
	canvas->camera->center = player->position;

	DrawMapWithoutItems(canvas, map);
	for(I32 i = 0; i < map->item_n; i++)
	{
		game->item_spawn_cooldowns[i] -= seconds;
		if(game->item_spawn_cooldowns[i] <= 0.0f)
		{		
			DrawMapItem(canvas, &map->items[i]);

			game->item_spawn_cooldowns[i] = 0.0f;
		}
	}

	I32 hover_item_index = 0;
	MapItem *hover_item = 0;
	for(I32 i = 0; i < map->item_n; i++)
	{
		MapItem *item = &map->items[i];
		if(game->item_spawn_cooldowns[i] == 0.0f)
		{
			R32 distance = Distance(item->position, player->position);
			if(distance < 2.0f)
			{
				hover_item_index = i;
				hover_item = item;
				break;
			}
		}
	}

	if(hover_item)
	{
		V4 hover_item_color = MakeColor(1.0f, 0.2f, 1.0f);
		DrawCircle(canvas, hover_item->position, MapItemRadius, hover_item_color);

		if(WasKeyReleased(user_input, 'E'))
		{
			AddItemToInventory(&game->inventory, CrystalItemId);
			Assert(game->item_spawn_cooldowns[hover_item_index] == 0.0f);
			game->item_spawn_cooldowns[hover_item_index] = 30.0f;
		}
	}

	if(WasKeyPressed(user_input, VK_TAB))
	{
		R32 max_target_distance = 30.0f;

		Entity *new_target = player->target;
		R32 new_target_distance = max_target_distance;
		for(I32 i = 0; i < game->entity_n; i++)
		{
			Entity *entity = &game->entities[i];
			if(entity != player && entity != player->target)
			{
				if(IsAlive(entity) && IsEnemyOf(player, entity))
				{
					R32 distance = Distance(entity->position, player->position);
					if(distance < new_target_distance)
					{
						new_target = entity;
						new_target_distance = distance;
					}
				}
			}
		}

		player->target = new_target;
	}

	if(player->target && IsDead(player->target))
	{
		player->target = 0;
	}

	player->recharge_time = ClipUpToZero(player->recharge_time - seconds);

	if(WasKeyPressed(user_input, '1'))
	{
		if(IsAlive(player) && player->target && IsAlive(player->target))
		{
			Assert(IsEnemyOf(player, player->target));
			if(player->recharge_time == 0.0f)
			{
				R32 distance_from_target = Distance(player->position, player->target->position);
				DoDamage(player->target, 3);
				player->recharge_time = 1.0f;
			}
		}
	}

	for(I32 i = 0; i < game->entity_n; i++)
	{
		Entity *entity = &game->entities[i];
		V4 color = GetEntityGroupColor(entity->group_id);
		DrawEntity(canvas, entity, color);

		if(entity == player->target)
		{
			V4 highlight_color = MakeColor(1.0f, 1.0f, 0.0f);
			HighlightEntity(canvas, entity, highlight_color);
		}

		if(entity != game->player)
		{
			UpdateNpc(game, entity, seconds);
		}
	}

	if(player->recharge_time > 0.0f)
	{
		R32 recharge_from = 1.0f;
		R32 r = (player->recharge_time / recharge_from);
		Assert(IsBetween(r, 0.0f, 1.0f));

		Bitmap *bitmap = &canvas->bitmap;

		I32 bar_height = 10;
		IntRect bar_rect = {};
		bar_rect.left = 0;
		bar_rect.right = bitmap->width - 1;
		bar_rect.bottom = bitmap->height - 1;
		bar_rect.top = bar_rect.bottom - bar_height;

		V4 background_color = MakeColor(0.3f, 0.3f, 0.3f);
		DrawBitmapRect(bitmap, bar_rect, background_color);

		IntRect filled_rect = bar_rect;
		filled_rect.right = (I32)Lerp((R32)bar_rect.left, r, (R32)bar_rect.right);
		V4 filled_color = MakeColor(0.8f, 0.8f, 0.8f);
		DrawBitmapRect(bitmap, filled_rect, filled_color);
	}

	Inventory *trade_inventory = &game->trade_inventory;
	Inventory *inventory = &game->inventory;
	if(game->show_trade_window)
	{
		I32 bottom = (bitmap->height - 1) - UIBoxPadding;
		I32 left = UIBoxPadding;
		MoveInventoryToBottomLeft(trade_inventory, bottom, left);

		DrawInventory(canvas, trade_inventory);
		game->show_inventory = true;

		InventoryItem hover_item = GetInventoryItemAtPosition(trade_inventory, user_input->mouse_pixel_position);
		if(hover_item.inventory != 0 && hover_item.item_id != NoItemId)
		{
			V4 color = MakeColor(1.0f, 1.0f, 0.0f);
			DrawInventorySlotOutline(canvas, trade_inventory, hover_item.slot, color);

			if(game->show_trade_window)
			{
				if(WasKeyReleased(user_input, VK_RBUTTON))
				{
					MoveItemToInventory(hover_item, inventory);
				}
			}
		}
	}

	if(game->show_inventory)
	{
		I32 bottom = (bitmap->height - 1) - UIBoxPadding;
		I32 right  = (bitmap->width - 1) - UIBoxPadding;
		MoveInventoryToBottomRight(inventory, bottom, right);

		DrawInventory(canvas, inventory);

		InventoryItem hover_item = GetInventoryItemAtPosition(inventory, user_input->mouse_pixel_position);
		if(hover_item.inventory != 0 && hover_item.item_id != NoItemId)
		{
			V4 color = MakeColor(1.0f, 1.0f, 0.0f);
			DrawInventorySlotOutline(canvas, inventory, hover_item.slot, color);

			if(game->show_trade_window)
			{
				if(WasKeyReleased(user_input, VK_RBUTTON))
				{
					MoveItemToInventory(hover_item, trade_inventory);
				}
			}
		}
	}
}