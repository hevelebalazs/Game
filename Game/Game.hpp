#pragma once

#include "Item.hpp"
#include "Map.hpp"
#include "UserInput.hpp"

#define GameArenaSize (1 * MegaByte)

enum EntityGroupId
{
	NeutralGroupId,
	OrangeGroupId,
	PurpleGroupId
};

struct Entity
{
	Vec2 position;
	Vec2 velocity;

	Int32 healthPoints;
	Int32 maxHealthPoints;

	Entity *target;

	Real32 rechargeTime;
	EntityGroupId groupId;
};

#define MaxNpcN 1024

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

	Bool32 questFinished;

	Int32 npcN;
	Entity npcs[MaxNpcN];
};

static void
func AddNpc(Game *game, Entity *npc)
{
	Assert(game->npcN < MaxNpcN);
	game->npcs[game->npcN] = *npc;
	game->npcN++;
}

static void
func InitNpc(Entity *npc, EntityGroupId groupId, Real32 x, Real32 y)
{
	npc->groupId = groupId;
	npc->position = MakePoint(x, y);
	npc->velocity = MakePoint(0.0f, 0.0f);
	npc->maxHealthPoints = 100;
	npc->healthPoints = npc->maxHealthPoints;
}

static EntityGroupId
func GetRandomGroupId()
{
	EntityGroupId id = NeutralGroupId;
	Int32 random = IntRandom(0, 1);
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
	game->arena = CreateMemArena(game->arenaMemory, GameArenaSize);

	Camera *camera = canvas->camera;
	camera->unitInPixels = 30;
	camera->center = MakePoint(0.0, 0.0);

	Entity *player = &game->player;
	player->position = MakePoint(0.5f * MapTileSide, 0.5f * MapTileSide);
	player->velocity = MakeVector(0.0f, 0.0f);
	player->maxHealthPoints = 100;
	player->healthPoints = player->maxHealthPoints;

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

	game->questFinished = false;

	Map *map = &game->map;
	for(Int32 row = 0; row < map->tileRowN; row++)
	{
		for(Int32 col = 0; col < map->tileColN; col++)
		{
			IntVec2 tile = MakeIntPoint(row, col);
			TileId tileId = GetTileType(map, tile);
			if(tileId != NoTileId)
			{
				Int32 tileNpcN = IntRandom(0, 3);

				Rect tileRect = GetTileRect(map, tile);
				for(Int32 i = 0; i < tileNpcN; i++)
				{
					Real32 x = RandomBetween(tileRect.left, tileRect.right);
					Real32 y = RandomBetween(tileRect.top, tileRect.bottom);
					EntityGroupId id = GetRandomGroupId();

					Entity npc = {};
					InitNpc(&npc, id, x, y);
					AddNpc(game, &npc);
				}
			}
		}
	}

	for(Int32 i = 0; i < game->npcN - 1; i++)
	{
		Int32 j = IntRandom(i + 1, game->npcN - 1);
		Entity tmp = game->npcs[i];
		game->npcs[i] = game->npcs[j];
		game->npcs[j] = tmp;
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
func DrawInteractionDialogWithText(Canvas *canvas, Int8 *text)
{
	Vec4 color = MakeColor(0.8f, 0.8f, 0.4f);
	Vec4 outlineColor = MakeColor(1.0f, 1.0f, 0.5f);

	Int32 border = 30;
	Bitmap *bitmap = &canvas->bitmap;
	IntRect rect = {};
	rect.bottom = bitmap->height - border;
	rect.left = border;
	rect.right = bitmap->width - border;
	rect.top = rect.bottom - 200;
	DrawBitmapRect(bitmap, rect, color);
	DrawBitmapRectOutline(bitmap, rect, outlineColor);

	Vec4 textColor = MakeColor(0.0f, 0.0f, 0.0f);
	Int32 textLeft = rect.left + 10;
	Int32 textTop = rect.top + 10;

	DrawBitmapTextLineTopLeft(bitmap, text, canvas->glyphData, textLeft, textTop, textColor);
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
	ItemId itemId = GetInventoryItemId(inventory, slot);
	SlotId slotId = GetInventorySlotId(inventory, slot);
	IntRect slotRect = GetInventorySlotRect(inventory, slot);

	Vec4 slotBackgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 itemBackgroundColor = MakeColor(0.1f, 0.1f, 0.1f);
	Vec4 cooldownColor = MakeColor(0.2f, 0.0f, 0.0f);
	Vec4 itemNameColor = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 slotNameColor = MakeColor(0.3f, 0.3f, 0.3f);

	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyphData = canvas->glyphData;

	if(itemId == NoItemId)
	{
		DrawBitmapRect(bitmap, slotRect, slotBackgroundColor);
		if(slotId != AnySlotId)
		{
			Int8 *slotName = GetSlotName(slotId);
			Assert(slotName != 0);
			DrawBitmapTextLineCentered(bitmap, slotName, glyphData, slotRect, slotNameColor);
		}
	}
	else
	{
		DrawBitmapRect(bitmap, slotRect, itemBackgroundColor);
		Int8 *name = GetItemSlotName(itemId);
		DrawBitmapTextLineCentered(bitmap, name, glyphData, slotRect, itemNameColor);
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
	GlyphData *glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	IntRect rect = GetInventoryRect(inventory);

	Vec4 backgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
	Vec4 hoverOutlineColor = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 invalidOutlineColor = MakeColor(1.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, backgroundColor);

	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
			SlotId slotId = GetInventorySlotId(inventory, slot);
			ItemId itemId = GetInventoryItemId(inventory, slot);
			DrawInventorySlot(canvas, inventory, slot);
		}
	}
}

static InventoryItem
func GetInventoryItemAtPosition(Inventory *inventory, IntVec2 position)
{
	InventoryItem item = {};
	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
			ItemId itemId = GetInventoryItemId(inventory, slot);
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
	Assert(game->showTradeWindow);
	Assert(game->showInventory);

	Inventory *inventory = &game->inventory;
	Inventory *tradeInventory = &game->tradeInventory;
	for(Int32 row = 0; row < tradeInventory->rowN; row++)
	{
		for(Int32 col = 0; col < tradeInventory->colN; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
			InventoryItem item = GetInventoryItem(tradeInventory, slot);
			Assert(item.inventory == tradeInventory);
			if(item.itemId != NoItemId)
			{
				MoveItemToInventory(item, inventory);
			}
		}
	}

	game->showTradeWindow = false;
	game->showInventory = false;
}

static void
func DrawEntity(Canvas *canvas, Entity *entity, Vec4 color)
{
	Rect rect = GetEntityRect(entity);
	DrawRect(canvas, rect, color);

	if(entity->maxHealthPoints > 0)
	{
		Assert(IsIntBetween(entity->healthPoints, 0, entity->maxHealthPoints));
		Real32 ratio = (Real32)entity->healthPoints / (Real32)entity->maxHealthPoints;

		Real32 unitInPixels = canvas->camera->unitInPixels;
		Real32 barWidth = 50.0f / unitInPixels;
		Real32 barHeight = 10.0f / unitInPixels;

		Rect barRect = {};
		barRect.left   = entity->position.x - barWidth * 0.5f;
		barRect.right  = entity->position.x + barWidth * 0.5f;
		barRect.bottom = rect.top - 10.0f / unitInPixels;
		barRect.top    = barRect.bottom - barHeight;

		Vec4 barBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
		Vec4 barOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);

		DrawRect(canvas, barRect, barBackgroundColor);

		Vec4 filledBarColor = MakeColor(1.0f, 0.0f, 0.0f);
		Rect filledBarRect = barRect;
		filledBarRect.right = barRect.left + (barRect.right - barRect.left) * ratio;
		DrawRect(canvas, filledBarRect, filledBarColor);


		DrawRectOutline(canvas, barRect, barOutlineColor);
	}
}

static void
func HighlightEntity(Canvas *canvas, Entity *entity, Vec4 color)
{
	Rect rect = GetEntityRect(entity);
	DrawRectOutline(canvas, rect, color);
}

static Real32
func ClipUpToZero(Real32 value)
{
	Real32 result = (value >= 0.0f) ? value : 0.0f;
	return result;
}

static Int32
func ClipIntUpToZero(Int32 value)
{
	Int32 result = (value >= 0) ? value : 0;
	return result;
}

static void
func DoDamage(Entity *target, Int32 damage)
{
	target->healthPoints = ClipIntUpToZero(target->healthPoints - damage);
}

#define MaxAttackDistance 30.0f
#define MaxMeleeDistance 3.0f

static void
func UpdateNpcTarget(Game *game, Entity *npc)
{
	Assert(npc->healthPoints > 0);

	Entity *target = npc->target;
	if(target && target->healthPoints == 0)
	{
		target = 0;
	}

	if(!target)
	{
		for(Int32 i = 0; i < game->npcN; i++)
		{
			Entity *entity = &game->npcs[i];
			if(entity != npc)
			{
				Bool32 isAlive = (entity->healthPoints > 0);
				Bool32 isEnemy = (entity->groupId != NeutralGroupId && entity->groupId != npc->groupId);
				if(isAlive && isEnemy)
				{
					Real32 distance = Distance(npc->position, entity->position);
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
func UpdateNpc(Game *game, Entity *npc, Real32 seconds)
{
	if(npc->healthPoints > 0)
	{
		npc->rechargeTime = ClipUpToZero(npc->rechargeTime - seconds);
		npc->velocity = MakeVector(0.0f, 0.0f);

		UpdateNpcTarget(game, npc);

		Entity *target = npc->target;
		if(target)
		{
			Real32 distance = Distance(npc->position, target->position);
			if(distance > MaxMeleeDistance)
			{
				Vec2 direction = PointDirection(npc->position, target->position);
				Real32 speed = 10.0f;
				npc->velocity = speed * direction;
			}
			else
			{
				if(npc->rechargeTime == 0.0f)
				{
					DoDamage(target, 30);
					npc->rechargeTime = 3.0f;
				}
			}
		}
	}
	else
	{
		npc->velocity = MakeVector(0.0f, 0.0f);
	}

	npc->position += seconds * npc->velocity;
}

static Vec4
func GetEntityGroupColor(EntityGroupId groupId)
{
	Vec4 color = {};
	switch(groupId)
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
			if(game->showTradeWindow)
			{
				StopTrading(game);
			}
			game->showInventory = false;
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
	DrawEntity(canvas, player, playerColor);

	Vec4 npcColor = MakeColor(1.0f, 0.0f, 1.0f);
	Vec4 npcHighlightColor = MakeColor(0.5f, 0.0f, 0.5f);
	Vec4 npcBorderColor = MakeColor(1.0f, 1.0f, 0.0f);
	Entity npc = {};
	npc.position = MakePoint(50.0f, 25.0f);

	Real32 playerNPCDistance = Distance(player->position, npc.position);
	Real32 interactionDistance = 3.0f;
	Rect npcRect = GetEntityRect(&npc);

	DrawEntity(canvas, &npc, npcColor);
	if(playerNPCDistance <= interactionDistance)
	{
		HighlightEntity(canvas, &npc, npcBorderColor);
		Int8 *text = 0;
		if(game->questFinished)
		{
			text = "Thanks!";
		}
		else
		{
			text = "Can you bring me 10 crystals?";
		}
		DrawInteractionDialogWithText(canvas, text);

		if(WasKeyReleased(userInput, 'E'))
		{
			if(game->showTradeWindow)
			{
				StopTrading(game);
			}
			else
			{
				game->showTradeWindow = true;
				game->showInventory = true; 
			}
		}
	}
	else
	{
		if(game->showTradeWindow)
		{
			StopTrading(game);
		}
	}

	for(Int32 i = 0; i < game->npcN; i++)
	{
		Entity *npc = &game->npcs[i];
		Vec4 color = GetEntityGroupColor(npc->groupId);
		DrawEntity(canvas, npc, color);
		UpdateNpc(game, npc, seconds);
	}

	Inventory *tradeInventory = &game->tradeInventory;
	Inventory *inventory = &game->inventory;
	if(game->showTradeWindow)
	{
		Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
		Int32 left = UIBoxPadding;
		MoveInventoryToBottomLeft(tradeInventory, bottom, left);

		DrawInventory(canvas, tradeInventory);
		game->showInventory = true;

		InventoryItem hoverItem = GetInventoryItemAtPosition(tradeInventory, userInput->mousePixelPosition);
		if(hoverItem.inventory != 0 && hoverItem.itemId != NoItemId)
		{
			Vec4 color = MakeColor(1.0f, 1.0f, 0.0f);
			DrawInventorySlotOutline(canvas, tradeInventory, hoverItem.slot, color);

			if(game->showTradeWindow)
			{
				if(WasKeyReleased(userInput, VK_RBUTTON))
				{
					MoveItemToInventory(hoverItem, inventory);
				}
			}
		}

		if(!game->questFinished)
		{
			Int32 crystalCount = 0;
			Int32 itemCount = 0;
			for(Int32 row = 0; row < tradeInventory->rowN; row++)
			{
				for(Int32 col = 0; col < tradeInventory->colN; col++)
				{
					IntVec2 slot = MakeIntPoint(row, col);
					ItemId itemId = GetInventoryItemId(tradeInventory, slot);
					if(itemId != NoItemId)
					{
						itemCount++;
						if(itemId == CrystalItemId)
						{
							crystalCount++;
						}
					}
				}
			}

			Bool32 areItemsExpected = (crystalCount == 10 && itemCount == 10);
			if(areItemsExpected)
			{
				game->questFinished = true;
				ClearInventory(tradeInventory);
				StopTrading(game);
			}
		}
	}

	if(game->showInventory)
	{
		Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
		Int32 right  = (bitmap->width - 1) - UIBoxPadding;
		MoveInventoryToBottomRight(inventory, bottom, right);

		DrawInventory(canvas, inventory);

		InventoryItem hoverItem = GetInventoryItemAtPosition(inventory, userInput->mousePixelPosition);
		if(hoverItem.inventory != 0 && hoverItem.itemId != NoItemId)
		{
			Vec4 color = MakeColor(1.0f, 1.0f, 0.0f);
			DrawInventorySlotOutline(canvas, inventory, hoverItem.slot, color);

			if(game->showTradeWindow)
			{
				if(WasKeyReleased(userInput, VK_RBUTTON))
				{
					MoveItemToInventory(hoverItem, tradeInventory);
				}
			}
		}
	}
}