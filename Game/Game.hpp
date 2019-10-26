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
	Int8 arenaMemory[GameArenaSize];
	MemArena arena;
	Map map;

	Inventory inventory;
	Bool32 showInventory;

	Inventory tradeInventory;
	Bool32 showTradeWindow;

	Entity player;

	Real32 *itemSpawnCooldowns;
};

static void
func GameInit(Game *game, Canvas *canvas)
{
	game->arena = CreateMemArena(game->arenaMemory, GameArenaSize);

	Camera *camera = canvas->camera;
	camera->unitInPixels = 30;
	camera->center = MakePoint(0.0, 0.0);

	Entity *player = &game->player;
	player->position = MakePoint(0.5f * MapTileSide, 0.5f * MapTileSide);
	player->velocity = MakeVector(0.0f, 0.0f);

	Int8 *mapFile = "Data/Map.data";
	game->map = ReadMapFromFile(mapFile, &game->arena); 
	game->itemSpawnCooldowns = ArenaAllocArray(&game->arena, Real32, game->map.itemN);

	for(Int32 i = 0; i < game->map.itemN; i++)
	{
		game->itemSpawnCooldowns[i] = 0.0f;
	}

	canvas->glyphData = GetGlobalGlyphData();

	InitInventory(&game->inventory, &game->arena, 3, 5);
	game->showInventory = false;

	InitInventory(&game->tradeInventory, &game->arena, 3, 5);
	game->showTradeWindow = false;
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
	Vec2 moveVector = seconds * entity->velocity;
	Vec2 oldPosition = entity->position;
	Vec2 newPosition = entity->position + moveVector;

	IntVec2 topTile = GetContainingTile(map, GetEntityTop(entity));
	topTile.row--;

	IntVec2 bottomTile = GetContainingTile(map, GetEntityBottom(entity));
	bottomTile.row++;

	IntVec2 leftTile = GetContainingTile(map, GetEntityLeft(entity));
	leftTile.col--;

	IntVec2 rightTile = GetContainingTile(map, GetEntityRight(entity));
	rightTile.col++;

	for(Int32 row = topTile.row; row <= bottomTile.row; row++)
	{
		for(Int32 col = leftTile.col; col <= rightTile.col; col++)
		{
			IntVec2 tile = MakeTile(row, col);
			if(IsValidTile(map, tile) && IsTileType(map, tile, NoTileId))
			{
				Poly16 collisionPoly = {};
				Rect tileRect = GetTileRect(map, tile);
				Rect collisionRect = GetExtendedRect(tileRect, EntityRadius);

				if(IsBetween(oldPosition.x, collisionRect.left, collisionRect.right))
				{
					if(oldPosition.y <= collisionRect.top && newPosition.y > collisionRect.top)
					{
						newPosition.y = collisionRect.top;
					}

					if(oldPosition.y >= collisionRect.bottom && newPosition.y < collisionRect.bottom)
					{
						newPosition.y = collisionRect.bottom;
					}
				}

				if(IsBetween(oldPosition.y, collisionRect.top, collisionRect.bottom))
				{
					if(oldPosition.x <= collisionRect.left && newPosition.x > collisionRect.left)
					{
						newPosition.x = collisionRect.left;
					}

					if(oldPosition.x >= collisionRect.right && newPosition.x < collisionRect.right)
					{
						newPosition.x = collisionRect.right;
					}
				}
			}
		}
	}

	Real32 mapWidth  = GetMapWidth(map);
	Real32 mapHeight = GetMapHeight (map);
	newPosition.x = Clip(newPosition.x, EntityRadius, mapWidth  - EntityRadius);
	newPosition.y = Clip(newPosition.y, EntityRadius, mapHeight - EntityRadius);

	entity->position = newPosition;
}

static void
func DrawInteractionDialog(Canvas *canvas)
{
	Vec4 color = MakeColor(0.8f, 0.8f, 0.4f);
	Vec4 outlineColor = MakeColor(1.0f, 1.0f, 0.5f);

	Int32 border = 30;
	Bitmap *bitmap = &canvas->bitmap;
	Int32 bottom = bitmap->height - border;
	Int32 left = border;
	Int32 right = bitmap->width - border;
	Int32 top = bottom - 200;
	DrawBitmapRect(bitmap, left, right, top, bottom, color);
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);

	Vec4 textColor = MakeColor(0.0f, 0.0f, 0.0f);
	Int32 textLeft = left + 10;
	Int32 textTop = top + 10;

	Int8 *text = "Can you bring me 10 crystals?";
	DrawBitmapTextLineTopLeft(bitmap, text, canvas->glyphData, textLeft, textTop, textColor);
}

#define InventorySlotSide 50

static void
func DrawInventorySlot(Canvas *canvas, SlotId slotId, ItemId itemId, Int32 top, Int32 left)
{
	Int32 bottom = top + InventorySlotSide;
	Int32 right = left + InventorySlotSide;

	Vec4 slotBackgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 itemBackgroundColor = MakeColor(0.1f, 0.1f, 0.1f);
	Vec4 cooldownColor = MakeColor(0.2f, 0.0f, 0.0f);
	Vec4 itemNameColor = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 slotNameColor = MakeColor(0.3f, 0.3f, 0.3f);

	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyphData = canvas->glyphData;

	if(itemId == NoItemId)
	{
		DrawBitmapRect(bitmap, left, right, top, bottom, slotBackgroundColor);
		if(slotId != AnySlotId)
		{
			Int8 *slotName = GetSlotName(slotId);
			Assert(slotName != 0);
			DrawBitmapTextLineCentered(bitmap, slotName, glyphData, left, right, top, bottom, slotNameColor);
		}
	}
	else
	{
		DrawBitmapRect(bitmap, left, right, top, bottom, itemBackgroundColor);
		Int8 *name = GetItemSlotName(itemId);
		DrawBitmapTextLineCentered(bitmap, name, glyphData, left, right, top, bottom, itemNameColor);
	}
}

static void
func DrawInventorySlotOutline(Canvas *canvas, Int32 top, Int32 left, Vec4 color)
{
	Bitmap *bitmap = &canvas->bitmap;
	Int32 bottom = top + InventorySlotSide;
	Int32 right = left + InventorySlotSide;
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, color);
}

#define InventorySlotPadding 2

static Int32
func GetInventoryWidth(Inventory *inventory)
{
	Int32 width = InventorySlotPadding + inventory->colN * (InventorySlotSide + InventorySlotPadding);
	return width;
}

static Int32
func GetInventoryHeight(Inventory *inventory)
{
	Int32 height = InventorySlotPadding + inventory->rowN * (InventorySlotSide + InventorySlotPadding);
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

static void
func DrawInventory(Canvas *canvas, Inventory *inventory)
{
	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	Int32 slotPadding = 2;

	Int32 width = GetInventoryWidth(inventory);
	Int32 height = GetInventoryHeight(inventory);

	Int32 left   = inventory->left;
	Int32 right  = left + width;
	Int32 top    = inventory->top;
	Int32 bottom = top + height;

	Vec4 backgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
	Vec4 hoverOutlineColor = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 invalidOutlineColor = MakeColor(1.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);

	Int32 slotTop = top + slotPadding;
	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		Int32 slotLeft = left + slotPadding;
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			SlotId slotId = GetInventorySlotId(inventory, row, col);
			ItemId itemId = GetInventoryItemId(inventory, row, col);

			DrawInventorySlot(canvas, slotId, itemId, slotTop, slotLeft);

			slotLeft += (InventorySlotSide + slotPadding);
		}

		slotTop += (InventorySlotSide + slotPadding);
	}
}

static void
func GameUpdate(Game *game, Canvas *canvas, Real32 seconds, UserInput *userInput)
{
	Bitmap *bitmap = &canvas->bitmap;
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	Entity *player = &game->player;

	Real32 playerMoveSpeed = 10.0f;
	player->velocity.x = 0.0f;
	if(IsKeyDown(userInput, 'A'))
	{
		player->velocity.x -= playerMoveSpeed;
	}
	if(IsKeyDown(userInput, 'D'))
	{
		player->velocity.x += playerMoveSpeed;
	}

	player->velocity.y = 0.0f;
	if(IsKeyDown(userInput, 'W'))
	{
		player->velocity.y -= playerMoveSpeed;
	}
	if(IsKeyDown(userInput, 'S'))
	{
		player->velocity.y += playerMoveSpeed;
	}

	if(WasKeyReleased(userInput, 'I'))
	{
		if(game->showInventory)
		{
			game->showInventory = false;
			game->showTradeWindow = false;
		}
		else
		{
			game->showInventory = true;
		}
	}

	Map *map = &game->map;
	UpdateEntityMovement(player, map, seconds);
	canvas->camera->center = player->position;


	DrawMapWithoutItems(canvas, map);
	for(Int32 i = 0; i < map->itemN; i++)
	{
		game->itemSpawnCooldowns[i] -= seconds;
		if(game->itemSpawnCooldowns[i] <= 0.0f)
		{		
			DrawMapItem(canvas, &map->items[i]);

			game->itemSpawnCooldowns[i] = 0.0f;
		}
	}

	Int32 hoverItemIndex = 0;
	MapItem *hoverItem = 0;
	for(Int32 i = 0; i < map->itemN; i++)
	{
		MapItem *item = &map->items[i];
		if(game->itemSpawnCooldowns[i] == 0.0f)
		{
			Real32 distance = Distance(item->position, player->position);
			if(distance < 2.0f)
			{
				hoverItemIndex = i;
				hoverItem = item;
				break;
			}
		}
	}

	if(hoverItem)
	{
		Vec4 hoverItemColor = MakeColor(1.0f, 0.2f, 1.0f);
		DrawCircle(canvas, hoverItem->position, MapItemRadius, hoverItemColor);

		if(WasKeyReleased(userInput, 'E'))
		{
			AddItemToInventory(&game->inventory, CrystalItemId);
			Assert(game->itemSpawnCooldowns[hoverItemIndex] == 0.0f);
			game->itemSpawnCooldowns[hoverItemIndex] = 30.0f;
		}
	}
	   
	Vec4 playerColor = MakeColor(1.0f, 1.0f, 0.0f);
	Rect playerRect = GetEntityRect(player);
	DrawRect(canvas, playerRect, playerColor);

	Vec4 npcColor = MakeColor(1.0f, 0.0f, 1.0f);
	Vec4 npcHighlightColor = MakeColor(0.5f, 0.0f, 0.5f);
	Vec4 npcBorderColor = MakeColor(1.0f, 1.0f, 0.0f);
	Entity npc = {};
	npc.position = MakePoint(50.0f, 25.0f);

	Real32 playerNPCDistance = Distance(player->position, npc.position);
	Real32 interactionDistance = 3.0f;
	Rect npcRect = GetEntityRect(&npc);

	DrawRect(canvas, npcRect, npcColor);
	if(playerNPCDistance <= interactionDistance)
	{
		DrawRectOutline(canvas, npcRect, npcBorderColor);
		DrawInteractionDialog(canvas);

		if(WasKeyReleased(userInput, 'E'))
		{
			game->showTradeWindow = !game->showTradeWindow;
			game->showInventory = game->showTradeWindow;
		}
	}
	else
	{
		if(game->showTradeWindow)
		{
			game->showTradeWindow = false;
			game->showInventory = false;
		}
	}

	if(game->showTradeWindow)
	{
		Inventory *inventory = &game->tradeInventory;

		Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
		Int32 left = UIBoxPadding;
		MoveInventoryToBottomLeft(inventory, bottom, left);

		DrawInventory(canvas, inventory);
		game->showInventory = true;
	}

	if(game->showInventory)
	{
		Inventory *inventory = &game->inventory;

		Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
		Int32 right  = (bitmap->width - 1) - UIBoxPadding;
		MoveInventoryToBottomRight(inventory, bottom, right);

		DrawInventory(canvas, inventory);
	}
}