#pragma once

#include "Item.hpp"
#include "Map.hpp"
#include "UserInput.hpp"

#define GameArenaSize (1 * MegaByte)

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

#define MaxNpcN 1024

struct Game
{
	I8 arena_memory[GameArenaSize];
	MemArena arena;
	Map map;

	Inventory inventory;
	B32 show_inventory;

	Inventory trade_inventory;
	B32 show_trade_window;

	Entity player;

	R32 *item_spawn_cooldowns;

	B32 quest_finished;

	I32 npc_n;
	Entity npcs[MaxNpcN];
};

static void
func AddNpc(Game *game, Entity *npc)
{
	Assert(game->npc_n < MaxNpcN);
	game->npcs[game->npc_n] = *npc;
	game->npc_n++;
}

static void
func InitNpc(Entity *npc, EntityGroupId group_id, V2 position)
{
	npc->group_id = group_id;
	npc->position = position;
	npc->start_position = position;
	npc->velocity = MakePoint(0.0f, 0.0f);
	npc->max_health_points = 100;
	npc->health_points = npc->max_health_points;
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

	Entity *player = &game->player;
	player->position = MakePoint(0.5f * MapTileSide, 0.5f * MapTileSide);
	player->velocity = MakeVector(0.0f, 0.0f);
	player->max_health_points = 100;
	player->health_points = player->max_health_points;

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

	game->quest_finished = false;

	Map *map = &game->map;
	for(I32 i = 0; i < map->entity_n; i++)
	{
		MapEntity *map_entity = &map->entities[i];
		Entity npc = {};
		InitNpc(&npc, map_entity->group_id, map_entity->spawn_position);
		AddNpc(game, &npc);
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

static void
func UpdateEntityMovement(Entity *entity, Map *map, R32 seconds)
{
	V2 move_vector = seconds * entity->velocity;
	V2 old_position = entity->position;
	V2 new_position = entity->position + move_vector;

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
	Assert(target->health_points > 0);
	target->health_points = ClipIntUpToZero(target->health_points - damage);
	if(target->health_points == 0)
	{
		target->resurrect_time = 5.0f;
	}
}

#define MaxAttackDistance 30.0f
#define MaxMeleeDistance 3.0f

static void
func UpdateNpcTarget(Game *game, Entity *npc)
{
	Assert(npc->health_points > 0);

	Entity *target = npc->target;
	if(target && target->health_points == 0)
	{
		target = 0;
	}

	if(!target)
	{
		for(I32 i = 0; i < game->npc_n; i++)
		{
			Entity *entity = &game->npcs[i];
			if(entity != npc)
			{
				B32 is_alive = (entity->health_points > 0);
				B32 is_enemy = (entity->group_id != NeutralGroupId && entity->group_id != npc->group_id);
				if(is_alive && is_enemy)
				{
					R32 distance = Distance(npc->position, entity->position);
					if(distance <= MaxAttackDistance)
					{
						target = entity;
						break;
					}
				}
			}
		}
	}

	npc->target = target;
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
				V2 direction = PointDirection(npc->position, target->position);
				R32 speed = 10.0f;
				npc->velocity = speed * direction;
			}
			else
			{
				if(npc->recharge_time == 0.0f)
				{
					DoDamage(target, 3);
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

	npc->position += seconds * npc->velocity;
}

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

static void
func GameUpdate(Game *game, Canvas *canvas, R32 seconds, UserInput *user_input)
{
	Bitmap *bitmap = &canvas->bitmap;
	V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, background_color);

	Entity *player = &game->player;

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
	UpdateEntityMovement(player, map, seconds);
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
	   
	V4 player_color = MakeColor(1.0f, 1.0f, 0.0f);
	DrawEntity(canvas, player, player_color);

	V4 npc_color = MakeColor(1.0f, 0.0f, 1.0f);
	V4 npc_highlight_color = MakeColor(0.5f, 0.0f, 0.5f);
	V4 npc_border_color = MakeColor(1.0f, 1.0f, 0.0f);
	Entity npc = {};
	npc.position = MakePoint(50.0f, 25.0f);

	R32 player_npc_distance = Distance(player->position, npc.position);
	R32 interaction_distance = 3.0f;
	Rect npc_rect = GetEntityRect(&npc);

	DrawEntity(canvas, &npc, npc_color);
	if(player_npc_distance <= interaction_distance)
	{
		HighlightEntity(canvas, &npc, npc_border_color);
		I8 *text = 0;
		if(game->quest_finished)
		{
			text = "Thanks!";
		}
		else
		{
			text = "Can you bring me 10 crystals?";
		}
		DrawInteractionDialogWithText(canvas, text);

		if(WasKeyReleased(user_input, 'E'))
		{
			if(game->show_trade_window)
			{
				StopTrading(game);
			}
			else
			{
				game->show_trade_window = true;
				game->show_inventory = true; 
			}
		}
	}
	else
	{
		if(game->show_trade_window)
		{
			StopTrading(game);
		}
	}

	for(I32 i = 0; i < game->npc_n; i++)
	{
		Entity *npc = &game->npcs[i];
		V4 color = GetEntityGroupColor(npc->group_id);
		DrawEntity(canvas, npc, color);
		UpdateNpc(game, npc, seconds);
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

		if(!game->quest_finished)
		{
			I32 crystal_count = 0;
			I32 item_count = 0;
			for(I32 row = 0; row < trade_inventory->row_n; row++)
			{
				for(I32 col = 0; col < trade_inventory->col_n; col++)
				{
					IV2 slot = MakeIntPoint(row, col);
					ItemId item_id = GetInventoryItemId(trade_inventory, slot);
					if(item_id != NoItemId)
					{
						item_count++;
						if(item_id == CrystalItemId)
						{
							crystal_count++;
						}
					}
				}
			}

			B32 are_items_expected = (crystal_count == 10 && item_count == 10);
			if(are_items_expected)
			{
				game->quest_finished = true;
				ClearInventory(trade_inventory);
				StopTrading(game);
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