#pragma once

#include "Item.hpp"
#include "Map.hpp"
#include "UserInput.hpp"

#define GameArenaSize (1 * MegaByte)

struct Entity
{
	Vec2 position;
	Vec2 velocity;
};

struct Game
{
	Int8 arena_memory[GameArenaSize];
	MemArena arena;
	Map map;

	Inventory inventory;
	Bool32 show_inventory;

	Inventory trade_inventory;
	Bool32 show_trade_window;

	Entity player;

	Real32 *item_spawn_cooldowns;

	Bool32 quest_finished;
};

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

	Int8 *map_file = "Data/Map.data";
	game->map = ReadMapFromFile(map_file, &game->arena); 
	game->item_spawn_cooldowns = ArenaAllocArray(&game->arena, Real32, game->map.item_n);

	for(Int32 i = 0; i < game->map.item_n; i++)
	{
		game->item_spawn_cooldowns[i] = 0.0f;
	}

	canvas->glyph_data = GetGlobalGlyphData();

	InitInventory(&game->inventory, &game->arena, 3, 5);
	game->show_inventory = false;

	InitInventory(&game->trade_inventory, &game->arena, 3, 5);
	game->show_trade_window = false;

	game->quest_finished = false;
}

#define EntityRadius 0.5f
#define EntitySide (2.0f * EntityRadius)

static Rect
func GetEntityRect(Entity *entity)
{
	Rect rect = MakeSquareRect(entity->position, EntitySide);
	return rect;
}

static Vec2
func GetEntityLeft(Entity *entity)
{
	Vec2 left = entity->position + MakeVector(-EntityRadius, 0.0f);
	return left;
}

static Vec2
func GetEntityRight(Entity *entity)
{
	Vec2 right = entity->position + MakeVector(+EntityRadius, 0.0f);
	return right;
}

static Vec2
func GetEntityTop(Entity *entity)
{
	Vec2 top = entity->position + MakeVector(0.0f, -EntityRadius);
	return top;
}

static Vec2
func GetEntityBottom(Entity *entity)
{
	Vec2 bottom = entity->position + MakeVector(0.0f, +EntityRadius);
	return bottom;
}

static void
func UpdateEntityMovement(Entity *entity, Map *map, Real32 seconds)
{
	Vec2 move_vector = seconds * entity->velocity;
	Vec2 old_position = entity->position;
	Vec2 new_position = entity->position + move_vector;

	IntVec2 top_tile = GetContainingTile(map, GetEntityTop(entity));
	top_tile.row--;

	IntVec2 bottom_tile = GetContainingTile(map, GetEntityBottom(entity));
	bottom_tile.row++;

	IntVec2 left_tile = GetContainingTile(map, GetEntityLeft(entity));
	left_tile.col--;

	IntVec2 right_tile = GetContainingTile(map, GetEntityRight(entity));
	right_tile.col++;

	for(Int32 row = top_tile.row; row <= bottom_tile.row; row++)
	{
		for(Int32 col = left_tile.col; col <= right_tile.col; col++)
		{
			IntVec2 tile = MakeTile(row, col);
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

	Real32 map_width  = GetMapWidth(map);
	Real32 map_height = GetMapHeight (map);
	new_position.x = Clip(new_position.x, EntityRadius, map_width - EntityRadius);
	new_position.y = Clip(new_position.y, EntityRadius, map_height - EntityRadius);

	entity->position = new_position;
}

static void
func DrawInteractionDialogWithText(Canvas *canvas, Int8 *text)
{
	Vec4 color = MakeColor(0.8f, 0.8f, 0.4f);
	Vec4 outline_color = MakeColor(1.0f, 1.0f, 0.5f);

	Int32 border = 30;
	Bitmap *bitmap = &canvas->bitmap;
	IntRect rect = {};
	rect.bottom = bitmap->height - border;
	rect.left = border;
	rect.right = bitmap->width - border;
	rect.top = rect.bottom - 200;
	DrawBitmapRect(bitmap, rect, color);
	DrawBitmapRectOutline(bitmap, rect, outline_color);

	Vec4 text_color = MakeColor(0.0f, 0.0f, 0.0f);
	Int32 text_left = rect.left + 10;
	Int32 text_top = rect.top + 10;

	DrawBitmapTextLineTopLeft(bitmap, text, canvas->glyph_data, text_left, text_top, text_color);
}

#define InventorySlotSide 50
#define InventorySlotPadding 2

static IntRect
func GetInventorySlotRect(Inventory *inventory, IntVec2 slot)
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
func DrawInventorySlot(Canvas *canvas, Inventory *inventory, IntVec2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	ItemId item_id = GetInventoryItemId(inventory, slot);
	SlotId slot_id = GetInventorySlotId(inventory, slot);
	IntRect slot_rect = GetInventorySlotRect(inventory, slot);

	Vec4 slot_background_color = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 item_background_color = MakeColor(0.1f, 0.1f, 0.1f);
	Vec4 cooldown_color = MakeColor(0.2f, 0.0f, 0.0f);
	Vec4 item_name_color = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 slot_name_color = MakeColor(0.3f, 0.3f, 0.3f);

	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;

	if(item_id == NoItemId)
	{
		DrawBitmapRect(bitmap, slot_rect, slot_background_color);
		if(slot_id != AnySlotId)
		{
			Int8 *slot_name = GetSlotName(slot_id);
			Assert(slot_name != 0);
			DrawBitmapTextLineCentered(bitmap, slot_name, glyph_data, slot_rect, slot_name_color);
		}
	}
	else
	{
		DrawBitmapRect(bitmap, slot_rect, item_background_color);
		Int8 *name = GetItemSlotName(item_id);
		DrawBitmapTextLineCentered(bitmap, name, glyph_data, slot_rect, item_name_color);
	}
}

static void
func DrawInventorySlotOutline(Canvas *canvas, Inventory *inventory, IntVec2 slot, Vec4 color)
{
	Bitmap *bitmap = &canvas->bitmap;
	IntRect rect = GetInventorySlotRect(inventory, slot);
	DrawBitmapRectOutline(bitmap, rect, color);
}

#define InventorySlotPadding 2

static Int32
func GetInventoryWidth(Inventory *inventory)
{
	Int32 width = InventorySlotPadding + inventory->col_n * (InventorySlotSide + InventorySlotPadding);
	return width;
}

static Int32
func GetInventoryHeight(Inventory *inventory)
{
	Int32 height = InventorySlotPadding + inventory->row_n * (InventorySlotSide + InventorySlotPadding);
	return height;
}

static void
func MoveInventoryToRight(Inventory *inventory, Int32 right)
{
	Int32 width = GetInventoryWidth(inventory);
	inventory->left = right - width;
}

static void
func MoveInventoryToBottom(Inventory *inventory, Int32 bottom)
{
	Int32 height = GetInventoryHeight(inventory);
	inventory->top = bottom - height;
}

static void
func MoveInventoryToBottomRight(Inventory *inventory, Int32 bottom, Int32 right)
{
	MoveInventoryToBottom(inventory, bottom);
	MoveInventoryToRight(inventory, right);
}

static void
func MoveInventoryToBottomLeft(Inventory *inventory, Int32 bottom, Int32 left)
{
	MoveInventoryToBottom(inventory, bottom);
	inventory->left = left;
}

#define UIBoxSide 40
#define UIBoxPadding 5

static IntRect
func GetInventoryRect(Inventory *inventory)
{
	Int32 width = GetInventoryWidth(inventory);
	Int32 height = GetInventoryHeight(inventory);

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

	Vec4 background_color = MakeColor(0.5f, 0.5f, 0.5f);
	Vec4 hover_outline_color = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 invalid_outline_color = MakeColor(1.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, background_color);

	for(Int32 row = 0; row < inventory->row_n; row++)
	{
		for(Int32 col = 0; col < inventory->col_n; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
			SlotId slot_id = GetInventorySlotId(inventory, slot);
			ItemId item_id = GetInventoryItemId(inventory, slot);
			DrawInventorySlot(canvas, inventory, slot);
		}
	}
}

static InventoryItem
func GetInventoryItemAtPosition(Inventory *inventory, IntVec2 position)
{
	InventoryItem item = {};
	for(Int32 row = 0; row < inventory->row_n; row++)
	{
		for(Int32 col = 0; col < inventory->col_n; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
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
	for(Int32 row = 0; row < trade_inventory->row_n; row++)
	{
		for(Int32 col = 0; col < trade_inventory->col_n; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
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
func GameUpdate(Game *game, Canvas *canvas, Real32 seconds, UserInput *user_input)
{
	Bitmap *bitmap = &canvas->bitmap;
	Vec4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, background_color);

	Entity *player = &game->player;

	Real32 player_move_speed = 10.0f;
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
	for(Int32 i = 0; i < map->item_n; i++)
	{
		game->item_spawn_cooldowns[i] -= seconds;
		if(game->item_spawn_cooldowns[i] <= 0.0f)
		{		
			DrawMapItem(canvas, &map->items[i]);

			game->item_spawn_cooldowns[i] = 0.0f;
		}
	}

	Int32 hover_item_index = 0;
	MapItem *hover_item = 0;
	for(Int32 i = 0; i < map->item_n; i++)
	{
		MapItem *item = &map->items[i];
		if(game->item_spawn_cooldowns[i] == 0.0f)
		{
			Real32 distance = Distance(item->position, player->position);
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
		Vec4 hover_item_color = MakeColor(1.0f, 0.2f, 1.0f);
		DrawCircle(canvas, hover_item->position, MapItemRadius, hover_item_color);

		if(WasKeyReleased(user_input, 'E'))
		{
			AddItemToInventory(&game->inventory, CrystalItemId);
			Assert(game->item_spawn_cooldowns[hover_item_index] == 0.0f);
			game->item_spawn_cooldowns[hover_item_index] = 30.0f;
		}
	}
	   
	Vec4 player_color = MakeColor(1.0f, 1.0f, 0.0f);
	Rect player_rect = GetEntityRect(player);
	DrawRect(canvas, player_rect, player_color);

	Vec4 npc_color = MakeColor(1.0f, 0.0f, 1.0f);
	Vec4 npc_highlight_color = MakeColor(0.5f, 0.0f, 0.5f);
	Vec4 npc_border_color = MakeColor(1.0f, 1.0f, 0.0f);
	Entity npc = {};
	npc.position = MakePoint(50.0f, 25.0f);

	Real32 player_npc_distance = Distance(player->position, npc.position);
	Real32 interaction_distance = 3.0f;
	Rect npc_rect = GetEntityRect(&npc);

	DrawRect(canvas, npc_rect, npc_color);
	if(player_npc_distance <= interaction_distance)
	{
		DrawRectOutline(canvas, npc_rect, npc_border_color);
		Int8 *text = 0;
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

	Inventory *trade_inventory = &game->trade_inventory;
	Inventory *inventory = &game->inventory;
	if(game->show_trade_window)
	{
		Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
		Int32 left = UIBoxPadding;
		MoveInventoryToBottomLeft(trade_inventory, bottom, left);

		DrawInventory(canvas, trade_inventory);
		game->show_inventory = true;

		InventoryItem hover_item = GetInventoryItemAtPosition(trade_inventory, user_input->mouse_pixel_position);
		if(hover_item.inventory != 0 && hover_item.item_id != NoItemId)
		{
			Vec4 color = MakeColor(1.0f, 1.0f, 0.0f);
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
			Int32 crystal_count = 0;
			Int32 item_count = 0;
			for(Int32 row = 0; row < trade_inventory->row_n; row++)
			{
				for(Int32 col = 0; col < trade_inventory->col_n; col++)
				{
					IntVec2 slot = MakeIntPoint(row, col);
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

			Bool32 are_items_expected = (crystal_count == 10 && item_count == 10);
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
		Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
		Int32 right  = (bitmap->width - 1) - UIBoxPadding;
		MoveInventoryToBottomRight(inventory, bottom, right);

		DrawInventory(canvas, inventory);

		InventoryItem hover_item = GetInventoryItemAtPosition(inventory, user_input->mouse_pixel_position);
		if(hover_item.inventory != 0 && hover_item.item_id != NoItemId)
		{
			Vec4 color = MakeColor(1.0f, 1.0f, 0.0f);
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