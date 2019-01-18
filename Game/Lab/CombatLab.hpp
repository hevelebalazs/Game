#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Geometry.hpp"
#include "../Item.hpp"
#include "../String.hpp"

#define WhiteColor MakeColor(1.0f, 1.0f, 1.0f)
#define BlueColor  MakeColor(0.0f, 0.0f, 1.0f)
#define RedColor   MakeColor(1.0f, 0.0f, 0.0f)
#define GreenColor MakeColor(0.0f, 1.0f, 0.0f)

#define WhiteAbility1Radius 5.0f
#define WhiteAbility1TotalCastTime 0.5f
#define WhiteAbility1AngleRange (0.25f * PI)
#define WhiteAbility1Damage 20

#define WhiteAbility2TotalCastTime 0.3f
#define WhiteAbility2MoveSpeed 30.0f
#define WhiteAbility2ResourceCost 30
#define WhiteAbility2Damage 10

#define WhiteAbility3TotalCastTime 0.3f
#define WhiteAbility3Radius 5.0f
#define WhiteAbility3ResourceCost 60 
#define WhiteAbility3Damage 50

#define RedAbility1Radius 5.0f
#define RedAbility1TotalCastTime 0.8f
#define RedAbility1AngleRange (0.5f * PI)
#define RedAbility1Damage 10

#define RedAbility2Radius 5.0f
#define RedAbility2TotalCastTime 0.5f
#define RedAbility2AngleRange (0.25f * PI)
#define RedAbility2Damage 30

#define BlueAbility1Radius 3.0f
#define BlueAbility1TotalCastTime 1.0f
#define BlueAbility1Damage 10

#define BlueAbility2Radius 3.0f
#define BlueAbility2TotalCastTime 1.0f
#define BlueAbility2Damage 15
#define BlueAbility2SlowTime 3.0f
#define BlueAbility2ResourceCost 50

#define BlueAbility3Radius 2.0f
#define BlueAbility3TotalCastTime 1.0f
#define BlueAbility3Damage 30
#define BlueAbility3SlowTime 6.0f
#define BlueAbility3ResourceCost 100

#define MaxLevel 5

static I32 ExperienceNeededForNextLevel[] = 
{
	0,
	100,
	150,
	250,
	500
};

static I32 ConstitutionOnLevel[] =
{
	0,
	10,
	20,
	35,
	55,
	80
};

static I32 RedStrengthOnLevel[] =
{
	0,
	10,
	15,
	25,
	40,
	65
};

static I32 WhiteStrengthOnLevel[] =
{
	0,
	5,
	10,
	15,
	20,
	25,
	30
};

static I32 BlueIntellectOnLevel[] =
{
	0,
	10,
	15,
	25,
	40,
	65
};

static I32 WhiteDexterityOnLevel[] =
{
	0,
	5,
	10,
	15,
	20,
	25
};

static I32 BlueDexterityOnLevel[] =
{
	0,
	5,
	10,
	15,
	20,
	25
};

enum AbilityId
{
	NoAbilityId,
	WhiteAbility1Id,
	WhiteAbility2Id,
	WhiteAbility3Id,
	RedAbility1Id,
	RedAbility2Id,
	BlueAbility1Id,
	BlueAbility2Id,
	BlueAbility3Id
};

struct Ability
{
	I32 id;
	F32 castRatioLeft;

	V2 position;
	F32 angle;
};

enum EntityTypeId
{
	WhiteEntityId,
	RedEntityId,
	BlueEntityId
};

#define EntityNameSize 16
struct Entity
{
	EntityTypeId typeId;
	I32 level;

	V2 position;
	V2 velocity;
	F32 radius;

	I32 health;

	Ability ability;

	B32 isAttacking;
	F32 attackFromAngle;

	B32 hasSpecialResource;
	I32 maxSpecialResource;
	I32 specialResource;

	F32 slowTimeLeft;

	I32 strength;
	I32 intellect;
	I32 constitution;
	I32 dexterity;

	I8 name[EntityNameSize];
};

enum TextAlertId
{
	DamageTextAlertId,
	HealTextAlertId
};

struct TextAlert
{
	I32 id;
	V4 color;
	F32 centerX;
	F32 baseLineY;
	F32 timeRemaining;

	union
	{
		I32 damage;
		I32 heal;
	};
};

#define CombatLabEnemyN 20
#define MaxTextAlertN 100

#define CombatLogLineN 12
#define CombatLogLineSize 64

struct Inventory
{
	I32 top;
	I32 left;

	I32 rowN;
	I32 colN;
	I32* itemIds;
	I32* slotIds;
};

struct InventoryItem
{
	Inventory* inventory;
	I32 itemId;
	I32 slotId;
	I32 row;
	I32 col;
};

#define MaxDroppedItemN 32
#define DroppedItemDuration 10.0f

struct DroppedItem
{
	V2 position;
	I32 itemId;
	F32 timeLeft;
};

struct CombatLabState
{
	Camera camera;
	Canvas canvas;
	B32 running;

	Entity player;
	Entity enemies[CombatLabEnemyN];

	B32 followMouse;
	I32 useAbilityId;

	I32 experienceGainedThisLevel;

	F32 secondsFraction;

	B32 moveLeft;
	B32 moveRight;
	B32 moveUp;
	B32 moveDown;

	TextAlert textAlerts[MaxTextAlertN];
	I32 textAlertN;

	B32 showCharacterInfo;

	I8 combatLogLines[CombatLogLineN][CombatLogLineSize];

	DroppedItem droppedItems[MaxDroppedItemN];
	I32 droppedItemN;

	DroppedItem* hoverDroppedItem;

	I32 inventoryItemIds[5][8];
	I32 inventorySlotIds[5][8];
	Inventory inventory;

	I32 equipInventoryItemIds[1][8];
	I32 equipInventorySlotIds[1][8];
	Inventory equipInventory;
	
	InventoryItem hoverItem; 
	InventoryItem dragItem;
};
static CombatLabState gCombatLabState;

static void func CombatLabResize(CombatLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 20.0f;
}

static void func CombatLabBlit(Canvas* canvas, HDC context, RECT rect)
{
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	Bitmap* bitmap = &canvas->bitmap;
	BITMAPINFO bitmapInfo = GetBitmapInfo(bitmap);
	StretchDIBits(context,
				  0, 0, bitmap->width, bitmap->height,
				  0, 0, width, height,
				  bitmap->memory,
				  &bitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static void func GainSpecialResource(Entity* entity, I32 amount)
{
	Assert(entity->hasSpecialResource);
	Assert(amount >= 0);
	entity->specialResource = IntMin2(entity->specialResource + amount, entity->maxSpecialResource);
}

static F32 func GetAbilityCastTime(Entity* entity, I32 abilityId)
{
	Assert(entity->dexterity >= 0);
	F32 castTimeRatio = 100.0f / (100.0f + F32(entity->dexterity));
	F32 castTime = 0.0f;
	switch (abilityId)
	{
		case WhiteAbility1Id:
		{
			castTime = castTimeRatio * WhiteAbility1TotalCastTime;
			break;
		}
		case WhiteAbility2Id:
		{
			castTime = WhiteAbility2TotalCastTime;
			break;
		}
		case WhiteAbility3Id:
		{
			castTime = castTimeRatio * WhiteAbility3TotalCastTime;
			break;
		}
		case RedAbility1Id:
		{
			castTime = castTimeRatio * RedAbility1TotalCastTime;
			break;
		}
		case RedAbility2Id:
		{
			castTime = castTimeRatio * RedAbility2TotalCastTime;
			break;
		}
		case BlueAbility1Id:
		{
			castTime = castTimeRatio * BlueAbility1TotalCastTime;
			break;
		}
		case BlueAbility2Id:
		{
			castTime = castTimeRatio * BlueAbility2TotalCastTime;
			break;
		}
		case BlueAbility3Id:
		{
			castTime = castTimeRatio * BlueAbility3TotalCastTime;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	Assert(castTime > 0.0f);
	return castTime;
}

static I32 func GetAbilityDamage(Entity* entity, AbilityId abilityId)
{
	I32 damage = 0;
	switch (abilityId)
	{
		case WhiteAbility1Id:
		{
			damage = WhiteAbility1Damage + entity->strength;
			break;
		}
		case WhiteAbility2Id:
		{
			damage = WhiteAbility2Damage + entity->strength;
			break;
		}
		case WhiteAbility3Id:
		{
			damage = WhiteAbility3Damage + entity->strength;
			break;
		}
		case RedAbility1Id:
		{
			damage = RedAbility1Damage + entity->strength;
			break;
		}
		case RedAbility2Id:
		{
			damage = RedAbility2Damage + entity->strength;
			break;
		}
		case BlueAbility1Id:
		{
			damage = BlueAbility1Damage + entity->intellect;
			break;
		}
		case BlueAbility2Id:
		{
			damage = BlueAbility1Damage + entity->intellect;
			break;
		}
		case BlueAbility3Id:
		{
			damage = BlueAbility3Damage + entity->intellect;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return damage;
}

static void func AddCombatLogLine(CombatLabState* labState, I8* line)
{
	for (I32 i = 1; i < CombatLogLineN; i++)
	{
		StringCopy(labState->combatLogLines[i], labState->combatLogLines[i - 1], CombatLogLineSize);
	}
	StringCopy(line, labState->combatLogLines[CombatLogLineN - 1], CombatLogLineSize);
}

static I8 gCombatLogLine[CombatLogLineSize];
#define CombatLog(labState, line) {OneLineString(gCombatLogLine, CombatLogLineSize, line); AddCombatLogLine(labState, gCombatLogLine);}

static I32 func GetMaxHealth(Entity* entity)
{
	I32 maxHealth = 10 * entity->constitution;
	Assert(maxHealth > 0);
	return maxHealth;
}

static void func SetLevel(Entity* entity, I32 level)
{
	Assert(IsIntBetween(level, 1, MaxLevel));
	entity->level = level;

	if (entity->typeId == WhiteEntityId)
	{
		entity->constitution = ConstitutionOnLevel[level];
		entity->strength = WhiteStrengthOnLevel[level];
		entity->intellect = 0;
		entity->dexterity = WhiteDexterityOnLevel[level];
	}
	else if (entity->typeId == RedEntityId)
	{
		entity->constitution = ConstitutionOnLevel[level];
		entity->strength = RedStrengthOnLevel[level];
		entity->intellect = 0;
		entity->dexterity = 0;
	}
	else if (entity->typeId == BlueEntityId)
	{
		entity->constitution = ConstitutionOnLevel[level];
		entity->strength = 0;
		entity->intellect = BlueIntellectOnLevel[level];
		entity->dexterity = BlueDexterityOnLevel[level];
	}
	else
	{
		DebugBreak();
	}

	entity->health = GetMaxHealth(entity);
}

static void func GainExperience(CombatLabState* labState, I32 experience)
{
	Entity* player = &labState->player;
	if (player->level < MaxLevel)
	{
		labState->experienceGainedThisLevel += experience;

		CombatLog(labState, player->name + " Gains " + experience + " experience.");

		if (labState->experienceGainedThisLevel >= ExperienceNeededForNextLevel[player->level])
		{
			I32 level = player->level + 1;
			SetLevel(player, level);

			CombatLog(labState, player->name + " reaches level " + player->level + ".");

			labState->experienceGainedThisLevel -= ExperienceNeededForNextLevel[player->level];
		}
	}
}

static void func DropItem(CombatLabState* labState, Entity* entity, I32 itemId, V2 position)
{
	Assert(itemId != NoItemId);
	Assert(labState->droppedItemN + 1 < MaxDroppedItemN);
	DroppedItem* item = &labState->droppedItems[labState->droppedItemN];
	labState->droppedItemN++;
	item->itemId   = itemId;
	item->position = position;
	item->timeLeft = DroppedItemDuration;

	I8 itemName[32];
	GetItemName(itemId, itemName, 32);
	CombatLog(labState, entity->name + " drops " + itemName + ".");
}

static void func AddTextAlert(CombatLabState* labState, TextAlert textAlert)
{
	Assert(labState->textAlertN < MaxTextAlertN);
	labState->textAlerts[labState->textAlertN] = textAlert;
	labState->textAlertN++;
}

static V4 func GetEntityColor(Entity* entity)
{
	V4 color = {};
	switch (entity->typeId)
	{
		case WhiteEntityId:
		{
			color = WhiteColor;
			break;
		}
		case RedEntityId:
		{
			color = RedColor;
			break;
		}
		case BlueEntityId:
		{
			color = BlueColor;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return color;
}

static void func DoDamage(CombatLabState* labState, Entity* source, Entity* target, I32 damage)
{
	if (target->health > 0)
	{
		static I8 combatLogLine[CombatLogLineSize];

		target->health = IntMax2(target->health - damage, 0);

		TextAlert alert;
		alert.id            = DamageTextAlertId;
		alert.color         = GetEntityColor(source);
		alert.damage        = damage;
		alert.centerX       = target->position.x;
		alert.baseLineY     = target->position.y;
		alert.timeRemaining = 1.0f;
		AddTextAlert(labState, alert);

		CombatLog(labState, source->name + " damages " + target->name + " for " + damage + ".");

		if (target->health == 0)
		{
			Ability* ability = &target->ability;
			ability->castRatioLeft = 0.0f;

			CombatLog(labState, source->name + " kills " + target->name + ".");

			if (source == &labState->player)
			{
				GainExperience(labState, 20);
			}

			if (target->typeId == RedEntityId)
			{
				V2 dropPosition = target->position + MakeVector(0.0f, 2.0f);
				DropItem(labState, target, TestPotionId, dropPosition);
			}
		}
	}
}

static void func HealWithoutLog(CombatLabState* labState, Entity* entity, I32 healAmount)
{
	Assert(entity->health > 0);
	entity->health = IntMin2(entity->health + healAmount, GetMaxHealth(entity));

	TextAlert alert;
	alert.id            = HealTextAlertId;
	alert.color         = GreenColor;
	alert.heal          = healAmount;
	alert.centerX       = entity->position.x;
	alert.baseLineY     = entity->position.y;
	alert.timeRemaining = 1.0f;
	AddTextAlert(labState, alert);
}

static void func HealFromItem(CombatLabState* labState, I32 itemId, Entity* target, I32 healAmount)
{
	HealWithoutLog(labState, target, healAmount);

	I8 itemName[32];
	GetItemName(itemId, itemName, 32);
	CombatLog(labState, itemName + " heals " + target->name + " for " + healAmount + ".");
}

static void func SlowEntity(CombatLabState* labState, Entity* source, Entity* target, F32 seconds)
{
	target->slowTimeLeft += seconds;
	CombatLog(labState, source->name + " slows " + target->name + " for " + seconds + " sec.");
}

static void func InterruptAbility(Entity* entity)
{
	Ability* ability = &entity->ability;
	ability->id = NoAbilityId;
	ability->castRatioLeft = 0.0f;
}

static void func SetInventoryItemId(Inventory* inventory, I32 row, I32 col, I32 itemId)
{
	Assert(IsIntBetween(row, 0, inventory->rowN - 1));
	Assert(IsIntBetween(col, 0, inventory->colN - 1));
	Assert(ItemGoesIntoSlot(itemId, inventory->slotIds[inventory->colN * row + col]));
	inventory->itemIds[inventory->colN * row + col] = itemId;
}

static void func SetInventorySlotId(Inventory* inventory, I32 row, I32 col, I32 slotId)
{
	Assert(IsIntBetween(row, 0, inventory->rowN - 1));
	Assert(IsIntBetween(col, 0, inventory->colN - 1));
	Assert(IsValidSlotId(slotId));
	inventory->slotIds[inventory->colN * row + col] = slotId;
}

static void func CombatLabReset(CombatLabState* labState)
{
	InitRandom();

	labState->running = true;

	Entity* player = &labState->player;
	player->typeId = WhiteEntityId;
	SetLevel(player, 1);

	player->position = MakePoint(0.0f, 0.0f);
	player->radius = 1.0f;

	player->hasSpecialResource = true;
	player->specialResource = 0;
	player->maxSpecialResource = 100;

	player->slowTimeLeft = 0.0f;

	OneLineString(player->name, EntityNameSize, "Player");

	player->ability.id = WhiteAbility1Id;

	for (I32 i = 0; i < CombatLabEnemyN; ++i) 
	{
		Entity* enemy = &labState->enemies[i];
		if (i < CombatLabEnemyN / 2)
		{
			enemy->typeId = RedEntityId;
			enemy->ability.id = RedAbility1Id;
		}
		else
		{
			enemy->typeId = BlueEntityId;
			enemy->ability.id = BlueAbility1Id;
		}

		SetLevel(enemy, IntRandom(1, 3));
		enemy->position.x = player->position.x + RandomBetween(-50.0f, +50.0f);
		enemy->position.y = player->position.y + RandomBetween(-50.0f, +50.0f);
		enemy->radius = player->radius;

		enemy->health = GetMaxHealth(enemy);

		enemy->hasSpecialResource = false;
		enemy->isAttacking = false;

		if (enemy->level >= 2)
		{
			enemy->hasSpecialResource = true;
			enemy->maxSpecialResource = 100;
			enemy->specialResource = 0;
		}

		switch (enemy->typeId)
		{
			case RedEntityId:
			{
				OneLineString(enemy->name, EntityNameSize, "Red " + enemy->level);
				break;
			}
			case BlueEntityId:
			{
				OneLineString(enemy->name, EntityNameSize, "Blue " + enemy->level);
				break;
			}
			default:
			{
				DebugBreak();
			}
		}
	}

	labState->experienceGainedThisLevel = 0;

	labState->textAlertN = 0;

	Inventory* inventory = &labState->inventory;
	inventory->rowN = 5;
	inventory->colN = 8;
	inventory->itemIds = (I32*)labState->inventoryItemIds;
	inventory->slotIds = (I32*)labState->inventorySlotIds;

	SetInventoryItemId(inventory, 0, 0, TestItemId);
	SetInventoryItemId(inventory, 0, 1, TestHelmId);
	SetInventoryItemId(inventory, 0, 2, TestChestId);
	SetInventoryItemId(inventory, 0, 3, TestPantsId);
	SetInventoryItemId(inventory, 0, 4, TestBootsId);
	SetInventoryItemId(inventory, 0, 5, TestGlovesId);
	SetInventoryItemId(inventory, 1, 0, TestPotionId);

	Inventory* equipInventory = &labState->equipInventory;
	equipInventory->rowN = 1;
	equipInventory->colN = 8;
	equipInventory->itemIds = (I32*)labState->equipInventoryItemIds;
	equipInventory->slotIds = (I32*)labState->equipInventorySlotIds;

	SetInventorySlotId(equipInventory, 0, 0, HeadSlotId);
	SetInventorySlotId(equipInventory, 0, 1, ChestSlotId);
	SetInventorySlotId(equipInventory, 0, 2, ArmsSlotId);
	SetInventorySlotId(equipInventory, 0, 3, LegsSlotId);
	SetInventorySlotId(equipInventory, 0, 4, FeetSlotId);
	SetInventorySlotId(equipInventory, 0, 5, HandSlotId);
	SetInventorySlotId(equipInventory, 0, 6, HandSlotId);
	SetInventorySlotId(equipInventory, 0, 7, WaistSlotId);
}

static void func CombatLabResetAtLevel(CombatLabState* labState, I32 level)
{
	CombatLabReset(labState);
	SetLevel(&labState->player, level);
}

static Entity* func  GetFirstEnemyAtPosition(CombatLabState* labState, V2 position)
{
	Entity* result = 0;
	for (I32 i = 0; i < CombatLabEnemyN; ++i)
	{
		Entity* enemy = &labState->enemies[i];
		if (Distance(enemy->position, position) < enemy->radius)
		{
			result = enemy;
			break;
		}
	}
	return result;
}

static B32 func IsEntityOnLine(Entity* entity, V2 point1, V2 point2)
{
	B32 result = false;
	if (Distance(entity->position, point1) <= entity->radius)
	{
		result = true;
	}
	else if (Distance(entity->position, point2) <= entity->radius)
	{
		result = true;
	}
	else
	{
		Quad quad = {};
		V2 lineDirection = PointDirection(point1, point2);
		V2 crossDirection = TurnVectorToRight(lineDirection);
		quad.points[0] = point1 + entity->radius * crossDirection;
		quad.points[1] = point2 + entity->radius * crossDirection;
		quad.points[2] = point2 - entity->radius * crossDirection;
		quad.points[3] = point1 - entity->radius * crossDirection;
		if (IsPointInQuad(quad, entity->position))
		{
			result = true;
		}
	}

	return result;
}

static void func DamageEnemiesOnLine(CombatLabState* labState, V2 point1, V2 point2, I32 damage)
{
	Entity* player = &labState->player;
	for (I32 i = 0; i < CombatLabEnemyN; ++i)
	{
		Entity* enemy = &labState->enemies[i];
		if (IsEntityOnLine(enemy, point1, point2))
		{
			DoDamage(labState, player, enemy, damage);
		}
	}
}

#define InventorySlotSize 50

static B32 func IsMouseInCharacterInfo(Canvas* canvas, CombatLabState* labState, V2 mousePosition)
{
	Bitmap* bitmap = &canvas->bitmap;

	I32 padding = 5;
	I32 width = 400;
	I32 height = InventorySlotSize + 190 + InventorySlotSize * labState->inventory.rowN;

	I32 right  = (bitmap->width - 1) - padding;
	I32 left   = right - width;
	I32 top    = (bitmap->height - 1) / 2 - height / 2;
	I32 bottom = top + height;

	I32 mouseX = UnitXtoPixel(canvas->camera, mousePosition.x);
	I32 mouseY = UnitYtoPixel(canvas->camera, mousePosition.y);

	B32 result = IsIntBetween(mouseX, left, right) && IsIntBetween(mouseY, top, bottom);
	return result;
}

static B32 func IsValidInventoryItem(InventoryItem item)
{
	B32 result = (item.inventory != 0) && 
				 IsIntBetween(item.row, 0, item.inventory->rowN - 1) &&
				 IsIntBetween(item.col, 0, item.inventory->colN - 1);
	return result;
}

static B32 func CanSwapInventoryItems(InventoryItem item1, InventoryItem item2)
{
	Assert(IsValidInventoryItem(item1));
	Assert(IsValidInventoryItem(item2));

	B32 result = ItemGoesIntoSlot(item1.itemId, item2.slotId) && ItemGoesIntoSlot(item2.itemId, item1.slotId);
	return result;
}

static void func SwapInventoryItems(InventoryItem item1, InventoryItem item2)
{
	Assert(CanSwapInventoryItems(item1, item2));
	SetInventoryItemId(item1.inventory, item1.row, item1.col, item2.itemId);
	SetInventoryItemId(item2.inventory, item2.row, item2.col, item1.itemId);
}

static I32 func GetInventoryItemId(Inventory* inventory, I32 row, I32 col)
{
	Assert(IsIntBetween(row, 0, inventory->rowN - 1));
	Assert(IsIntBetween(col, 0, inventory->colN - 1));

	I32 itemId = inventory->itemIds[row * inventory->colN + col];
	return itemId;
}

static void func RecalculatePlayerAttributes(CombatLabState* labState)
{
	Entity* player = &labState->player;
	Assert(IsIntBetween(player->level, 1, MaxLevel));

	player->constitution = ConstitutionOnLevel[player->level];
	player->strength     = WhiteStrengthOnLevel[player->level];
	player->intellect    = 0;
	player->dexterity    = WhiteDexterityOnLevel[player->level];

	Inventory* equipInventory = &labState->equipInventory;
	for (I32 row = 0; row < equipInventory->rowN; row++)
	{
		for (I32 col = 0; col < equipInventory->colN; col++)
		{
			I32 itemId = GetInventoryItemId(equipInventory, row, col);
			if (itemId != NoItemId)
			{
				ItemAttributes itemAttributes = GetItemAttributes(itemId);
				player->constitution += itemAttributes.constitution;
				player->strength     += itemAttributes.strength;
				player->intellect    += itemAttributes.intellect;
				player->dexterity    += itemAttributes.dexterity;
			}
		}
	}
}

static B32 CanPickUpItem(CombatLabState* labState, DroppedItem* item)
{
	B32 result = false;

	Inventory* inventory = &labState->inventory;
	for (I32 row = 0; row < inventory->rowN; row++)
	{
		for (I32 col = 0; col < inventory->colN; col++)
		{
			I32 itemId = GetInventoryItemId(inventory, row, col);
			if (itemId == NoItemId)
			{
				result = true;
				break;
			}
		}

		if (result)
		{
			break;
		}
	}

	return result;
}

static void func PickUpItem(CombatLabState* labState, DroppedItem* item)
{
	Assert(item != 0);
	Assert(CanPickUpItem(labState, item));

	I32 itemIndex = 0;
	B32 itemFound = false;

	for (I32 i = 0; i < labState->droppedItemN; i++)
	{
		if (item == &labState->droppedItems[i])
		{
			itemFound = true;
			itemIndex = i;
			break;
		}
	}

	Assert(itemFound);
	labState->droppedItemN--;
	for (I32 i = itemIndex; i < labState->droppedItemN; i++)
	{
		labState->droppedItems[i] = labState->droppedItems[i + 1];
	}

	B32 itemPickedUp = false;
	Inventory* inventory = &labState->inventory;
	for (I32 row = 0; row < inventory->rowN; row++)
	{
		for (I32 col = 0; col < inventory->colN; col++)
		{
			I32 itemId = GetInventoryItemId(inventory, row, col);
			if (itemId == NoItemId)
			{
				SetInventoryItemId(inventory, row, col, item->itemId);
				itemPickedUp = true;
				break;
			}
		}

		if (itemPickedUp)
		{
			break;
		}
	}
	Assert(itemPickedUp);

	I8 itemName[32];
	GetItemName(item->itemId, itemName, 32);
	CombatLog(labState, labState->player.name + " picks up " + itemName + ".");
}

static LRESULT CALLBACK func CombatLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	CombatLabState* labState = &gCombatLabState;
	Entity* player = &labState->player;

	switch (message)
	{
		case WM_SIZE:
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			CombatLabResize(labState, width, height);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			CombatLabBlit(&labState->canvas, context, clientRect);

			EndPaint(window, &paint);
			break;
		}
		case WM_SETCURSOR:
		{
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			Inventory* inventory = &labState->inventory;
			V2 mousePosition = GetMousePosition(labState->canvas.camera, window);
			if (labState->showCharacterInfo && IsMouseInCharacterInfo(&labState->canvas, labState, mousePosition))
			{
				InventoryItem* hoverItem = &labState->hoverItem;
				if (hoverItem->inventory != 0 && hoverItem->itemId != NoItemId)
				{
					labState->dragItem = *hoverItem;
				}
			}
			else
			{
				labState->useAbilityId = WhiteAbility1Id;
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			Inventory* inventory = &labState->inventory;
			InventoryItem* dragItem = &labState->dragItem;

			V2 mousePosition = GetMousePosition(labState->canvas.camera, window);
			if (labState->showCharacterInfo)
			{
				InventoryItem* hoverItem = &labState->hoverItem;
				if (dragItem->inventory != 0 && hoverItem->inventory != 0)
				{
					Assert(dragItem->itemId != NoItemId);

					if (CanSwapInventoryItems(*hoverItem, *dragItem))
					{
						SwapInventoryItems(*hoverItem, *dragItem);
						RecalculatePlayerAttributes(labState);
					}
				}
				else if (!IsMouseInCharacterInfo(&labState->canvas, labState, mousePosition))
				{
					if (dragItem->itemId != NoItemId)
					{
						DropItem(labState, player, dragItem->itemId, mousePosition);
						SetInventoryItemId(dragItem->inventory, dragItem->row, dragItem->col, NoItemId);
					}
				}
			}

			labState->useAbilityId = NoAbilityId;
			
			dragItem->inventory = 0;
			dragItem->itemId = NoItemId;

			break;
		}
		case WM_RBUTTONDOWN:
		{
			labState->followMouse = true;
			break;
		}
		case WM_RBUTTONUP:
		{
			if (labState->showCharacterInfo)
			{
				InventoryItem* hoverItem = &labState->hoverItem;
				if (hoverItem->itemId == TestPotionId)
				{
					Entity* player = &labState->player;
					if (player->health > 0)
					{
						HealFromItem(labState, TestPotionId, &labState->player, 30);
						SetInventoryItemId(hoverItem->inventory, hoverItem->row, hoverItem->col, NoItemId);
					}
				}
			}

			labState->followMouse = false;
			break;
		}
		case WM_MOUSEWHEEL: 
		{
			I16 wheelDeltaParam = GET_WHEEL_DELTA_WPARAM(wparam);
			if (wheelDeltaParam > 0)
				labState->camera.unitInPixels *= 1.10f;
			else if (wheelDeltaParam < 0)
				labState->camera.unitInPixels /= 1.10f;
			break;
		}
		case WM_KEYDOWN:
		{
			WPARAM keyCode = wparam;
			switch (keyCode)
			{
				case 'Q':
				{
					labState->useAbilityId = WhiteAbility2Id;
					break;
				}
				case 'E':
				{
					labState->useAbilityId = WhiteAbility3Id;
					break;
				}
				case 'A':
				{
					labState->moveLeft = true;
					break;
				}
				case 'D':
				{
					labState->moveRight = true;
					break;
				}
				case 'W':
				{
					labState->moveUp = true;
					break;
				}
				case 'S':
				{
					labState->moveDown = true;
					break;
				}
				case ' ':
				{
					if (labState->hoverDroppedItem)
					{
						PickUpItem(labState, labState->hoverDroppedItem);
					}
					break;
				}
			}
			break;
		}
		case WM_KEYUP: 
		{
			WPARAM keyCode = wparam;
			switch (keyCode) 
			{
				case '1':
				{
					CombatLabResetAtLevel(labState, 1);
					break;
				}
				case '2':
				{
					CombatLabResetAtLevel(labState, 2);
					break;
				}
				case '3':
				{
					CombatLabResetAtLevel(labState, 3);
					break;
				}
				case '4':
				{
					CombatLabResetAtLevel(labState, 4);
					break;
				}
				case '5':
				{
					CombatLabResetAtLevel(labState, 5);
					break;
				}
				case 'Q':
				{
					labState->useAbilityId = NoAbilityId;
					break;
				}
				case 'E':
				{
					labState->useAbilityId = NoAbilityId;
					break;
				}
				case 'B':
				{
					Entity* player = &labState->player;
					Assert(player->hasSpecialResource);
					player->specialResource = player->maxSpecialResource;
					break;
				}
				case 'A':
				{
					labState->moveLeft = false;
					break;
				}
				case 'D':
				{
					labState->moveRight = false;
					break;
				}
				case 'W':
				{
					labState->moveUp = false;
					break;
				}
				case 'S':
				{
					labState->moveDown = false;
					break;
				}
				case 'C':
				{
					labState->showCharacterInfo = !labState->showCharacterInfo;
					break;
				}
			}
			break;
		}

		case WM_DESTROY:
		case WM_CLOSE:
		{
			labState->running = false;
			break;
		}
		default:
		{
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}
	return result;
}

static void func CombatLabInit(CombatLabState* labState, I32 windowWidth, I32 windowHeight)
{
	CombatLabReset(labState);

	labState->canvas.glyphData = GetGlobalGlyphData();
	CombatLabResize(labState, windowWidth, windowHeight);
}

static void func DrawSlice(Canvas* canvas, V2 center, F32 radius, F32 minAngle, F32 maxAngle, V4 color)
{
	U32 colorCode = GetColorCode(color);

	Camera *camera = canvas->camera;
	I32 centerXPixel = UnitXtoPixel(camera, center.x);
	I32 centerYPixel = UnitYtoPixel(camera, center.y);
	V2 pixelCenter = MakeVector(F32(centerXPixel), F32(centerYPixel));

	I32 leftPixel   = UnitXtoPixel(camera, center.x - radius);
	I32 rightPixel  = UnitXtoPixel(camera, center.x + radius);
	I32 topPixel    = UnitYtoPixel(camera, center.y - radius);
	I32 bottomPixel = UnitYtoPixel(camera, center.y + radius);

	if (topPixel > bottomPixel)
	{
		IntSwap(&topPixel, &bottomPixel);
	}

	if (leftPixel > rightPixel)
	{
		IntSwap(&leftPixel, &rightPixel);
	}

	I32 pixelRadius = I32(radius * camera->unitInPixels);
	I32 pixelRadiusSquare = pixelRadius * pixelRadius;

	Bitmap bitmap = canvas->bitmap;
	topPixel    = IntMax2(topPixel, 0);
	bottomPixel = IntMin2(bottomPixel, bitmap.height - 1);
	leftPixel   = IntMax2(leftPixel, 0);
	rightPixel  = IntMin2(rightPixel, bitmap.width - 1);

	for (I32 row = topPixel; row < bottomPixel; ++row)
	{
		for (I32 col = leftPixel; col < rightPixel; ++col)
		{
			U32* pixel = bitmap.memory + row * bitmap.width + col;
			I32 pixelDistanceSquare = IntSquare(row - centerYPixel) + IntSquare(col - centerXPixel);

			V2 pixelPosition = MakeVector(F32(col), F32(row));
			F32 angle = LineAngle(pixelCenter, pixelPosition);
			angle = NormalizeAngle(angle);

			if (pixelDistanceSquare <= pixelRadiusSquare && IsAngleBetween(minAngle, angle, maxAngle))
			{
				*pixel = colorCode;
			}
		}
	}
}

static void func DrawSliceOutline(Canvas* canvas, V2 center, F32 radius, F32 minAngle, F32 maxAngle, V4 color)
{
	if (minAngle > maxAngle)
	{
		minAngle -= TAU;
	}
	Assert(minAngle <= maxAngle);

	V2 startPoint = center + radius * RotationVector(minAngle);
	V2 endPoint = center + radius * RotationVector(maxAngle);
	Bresenham(canvas, center, startPoint, color);
	Bresenham(canvas, center, endPoint, color);

	I32 lineN = 20;
	for (I32 i = 0; i < lineN; ++i) 
	{
		F32 angle1 = minAngle + (F32(i) / F32(lineN)) * (maxAngle - minAngle);
		F32 angle2 = minAngle + (F32(i + 1) / F32(lineN)) * (maxAngle - minAngle);
		V2 point1 = center + radius * RotationVector(angle1);
		V2 point2 = center + radius * RotationVector(angle2);
		Bresenham(canvas, point1, point2, color);
	}
}

static void func DrawCircle(Canvas* canvas, V2 center, F32 radius, V4 color)
{
	U32 colorCode = GetColorCode(color);

	Camera *camera = canvas->camera;

	I32 centerXPixel = UnitXtoPixel(camera, center.x);
	I32 centerYPixel = UnitYtoPixel(camera, center.y);

	I32 leftPixel   = UnitXtoPixel(camera, center.x - radius);
	I32 rightPixel  = UnitXtoPixel(camera, center.x + radius);
	I32 topPixel    = UnitYtoPixel(camera, center.y - radius);
	I32 bottomPixel = UnitYtoPixel(camera, center.y + radius);

	if (topPixel > bottomPixel)
	{
		IntSwap(&topPixel, &bottomPixel);
	}

	if (leftPixel > rightPixel)
	{
		IntSwap(&leftPixel, &rightPixel);
	}

	I32 pixelRadius = I32(radius * camera->unitInPixels);
	I32 pixelRadiusSquare = pixelRadius * pixelRadius;

	Bitmap bitmap = canvas->bitmap;
	topPixel    = IntMax2(topPixel, 0);
	bottomPixel = IntMin2(bottomPixel, bitmap.height - 1);
	leftPixel   = IntMax2(leftPixel, 0);
	rightPixel  = IntMin2(rightPixel, bitmap.width - 1);

	for (I32 row = topPixel; row < bottomPixel; ++row)
	{
		for (I32 col = leftPixel; col < rightPixel; ++col)
		{
			U32* pixel = bitmap.memory + row * bitmap.width + col;
			I32 pixelDistanceSquare = IntSquare(row - centerYPixel) + IntSquare(col - centerXPixel);
			if (pixelDistanceSquare <= pixelRadiusSquare)
			{
				*pixel = colorCode;
			}
		}
	}
}

static void func DrawCircleOutline(Canvas* canvas, V2 center, F32 radius, V4 color)
{
	I32 lineN = 20;
	F32 angleAdvance = (2.0f * PI) / F32(lineN);
	F32 angle = 0.0f;
	for (I32 i = 0; i < lineN; ++i)
	{
		F32 angle1 = angle;
		F32 angle2 = angle + angleAdvance;
		V2 point1 = center + radius * RotationVector(angle1);
		V2 point2 = center + radius * RotationVector(angle2);
		Bresenham(canvas, point1, point2, color);

		angle += angleAdvance;
	}
}

static void func DrawEntity(Canvas* canvas, Entity* entity)
{
	V4 color = GetEntityColor(entity);

	DrawCircle(canvas, entity->position, entity->radius, color);

	Camera* camera = canvas->camera;
	Bitmap* bitmap = &canvas->bitmap;

	V4 healthBarBackgroundColor = (entity->health > 0.0f) ? MakeColor(0.5f, 0.0f, 0.0f) : MakeColor(0.5f, 0.5f, 0.5f);
	V4 healthBarColor = MakeColor(1.0f, 0.0f, 0.0f);

	I32 healthBarWidth = 40;
	I32 healthBarHeight = 10;

	I32 healthBarX = UnitXtoPixel(camera, entity->position.x);
	I32 healthBarY = UnitYtoPixel(camera, entity->position.y - entity->radius) - healthBarHeight - healthBarHeight / 2;

	I32 healthBarLeft   = healthBarX - healthBarWidth / 2;
	I32 healthBarRight  = healthBarX + healthBarWidth / 2;
	I32 healthBarTop    = healthBarY - healthBarHeight / 2;
	I32 healthBarBottom = healthBarY + healthBarHeight / 2;
	DrawBitmapRect(bitmap, healthBarLeft, healthBarRight, healthBarTop, healthBarBottom, healthBarBackgroundColor);

	F32 healthRatio = F32(entity->health) / GetMaxHealth(entity);
	Assert(IsBetween(healthRatio, 0.0f, 1.0f));
	if (healthRatio > 0.0f)
	{
		I32 healthBarFilledX = healthBarLeft + I32(healthRatio * F32(healthBarWidth));
		DrawBitmapRect(bitmap, healthBarLeft, healthBarFilledX, healthBarTop, healthBarBottom, healthBarColor);
	}

	if (entity->health > 0)
	{
		for (I32 health = 50; health < entity->health; health += 50)
		{
			F32 ratio = F32(health) / GetMaxHealth(entity);
			I32 x = I32(Lerp(F32(healthBarLeft), ratio, F32(healthBarRight)));
			DrawBitmapBresenhamLine(bitmap, healthBarTop, x, healthBarBottom, x, healthBarBackgroundColor);
		}
	}

	if (entity->hasSpecialResource)
	{
		V4 filledColor = MakeColor(1.0f, 1.0f, 0.0f);
		V4 emptyColor = MakeColor(0.3f, 0.3f, 0.3f);
		I32 barTop = healthBarBottom + 2;
		I32 barBottom = barTop + 3;
		I32 barLeft = healthBarLeft;
		I32 barRight = healthBarRight;
		DrawBitmapRect(bitmap, barLeft, barRight, barTop, barBottom, emptyColor);
		
		Assert(entity->maxSpecialResource > 0);
		Assert(IsIntBetween(entity->specialResource, 0, entity->maxSpecialResource));
		F32 filledRatio = F32(entity->specialResource) / F32(entity->maxSpecialResource);
		I32 barFilledX = barLeft + I32(F32(barRight - barLeft) * filledRatio);
		DrawBitmapRect(bitmap, barLeft, barFilledX, barTop, barBottom, filledColor);
	}

	Assert(StringIsTerminated(entity->name, EntityNameSize));
	I32 nameX = healthBarLeft;
	I32 nameLineY = healthBarTop - 2 - TextPixelsBelowBaseLine;
	V4 nameColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapTextLine(bitmap, entity->name, canvas->glyphData, nameX, nameLineY, nameColor);
}

static B32 func IsEntityInSlice(Entity* entity, V2 center, F32 radius, F32 minAngle, F32 maxAngle)
{
	F32 distance = Distance(entity->position, center);
	F32 angle = LineAngle(center, entity->position);
	B32 result = (distance < radius + entity->radius && IsAngleBetween(minAngle, angle, maxAngle));
	return result;
}

static B32 func IsEntityInCircle(Entity* entity, V2 center, F32 radius)
{
	B32 result = false;
	F32 distanceFromCenter = Distance(entity->position, center);
	if (distanceFromCenter < radius + entity->radius)
	{
		result = true;
	}
	return result;
}

static void func DrawWhiteAbility1(Canvas* canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability)
	Assert(ability->id == WhiteAbility1Id);
	V4 color = MakeColor(0.8f, 0.8f, 0.8f);
	F32 radius = entity->radius + WhiteAbility1Radius;

	if (ability->castRatioLeft > 0.0f)
	{
		F32 fillRadius = Lerp(radius, ability->castRatioLeft, entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - WhiteAbility1AngleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + WhiteAbility1AngleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void func DrawWhiteAbility3(Canvas* canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == WhiteAbility3Id);
	V4 color = MakeColor(0.8f, 0.8f, 0.8f);
	F32 radius = entity->radius + WhiteAbility3Radius;

	if (ability->castRatioLeft > 0.0f)
	{
		F32 fillRadius = Lerp(radius - entity->radius, ability->castRatioLeft, entity->radius);
		DrawCircleOutline(canvas, entity->position, radius, color);
		DrawCircle(canvas, entity->position, fillRadius, color);
	}
}

static void func DrawRedAbility1(Canvas* canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == RedAbility1Id);
	V4 color = MakeColor(0.8f, 0.0f, 0.0f);
	F32 radius = entity->radius + RedAbility1Radius;

	if (ability->castRatioLeft > 0.0f)
	{
		F32 fillRadius = Lerp(radius - entity->radius, ability->castRatioLeft, entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - RedAbility1AngleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + RedAbility1AngleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void func DrawRedAbility3(Canvas* canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == RedAbility2Id);

	Assert(entity->level >= 2);
	V4 color = MakeColor(0.8f, 0.0f, 0.0f);
	F32 radius = entity->radius + RedAbility2Radius;

	if (ability->castRatioLeft > 0.0f)
	{
		F32 fillRadius = Lerp(radius - entity->radius, ability->castRatioLeft, entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - RedAbility2AngleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + RedAbility2AngleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void func DrawBlueAbility1(Canvas* canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability)
	Assert(ability->id == BlueAbility1Id);

	V4 color = MakeColor(0.0f, 0.0f, 0.8f);
	if (ability->castRatioLeft > 0.0f)
	{
		F32 fillRadius = Lerp(BlueAbility1Radius, ability->castRatioLeft, 0.0f);

		DrawCircleOutline(canvas, ability->position, BlueAbility1Radius, color);
		DrawCircle(canvas, ability->position, fillRadius, color);
	}
}

static void func DrawBlueAbility2(Canvas* canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == BlueAbility2Id);

	V4 color = MakeColor(0.0f, 0.0f, 0.8f);
	if (ability->castRatioLeft > 0.0f)
	{
		F32 fillRadius = Lerp(entity->radius + BlueAbility2Radius, ability->castRatioLeft, entity->radius);
		
		DrawCircleOutline(canvas, entity->position, entity->radius + BlueAbility2Radius, color);
		DrawCircle(canvas, entity->position, fillRadius, color);
	}
}

static void func DrawBlueAbility3(Canvas* canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == BlueAbility3Id);

	V4 color = MakeColor(0.0f, 0.0f, 0.8f);
	if (ability->castRatioLeft > 0.0f)
	{
		F32 fillRadius = Lerp(BlueAbility3Radius, ability->castRatioLeft, 0.0f);
		
		Bresenham(canvas, entity->position, ability->position, color);
		DrawCircleOutline(canvas, ability->position, BlueAbility3Radius, color);
		DrawCircle(canvas, ability->position, fillRadius, color);
	}
}

static void func DrawAbility(Canvas* canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	if (ability->id == WhiteAbility1Id)
	{
		DrawWhiteAbility1(canvas, entity, ability);
	}
	else if (ability->id == WhiteAbility2Id)
	{
	}
	else if (ability->id == WhiteAbility3Id)
	{
		DrawWhiteAbility3(canvas, entity, ability);
	}
	else if (ability->id == BlueAbility1Id)
	{
		DrawBlueAbility1(canvas, entity, ability);
	}
	else if (ability->id == BlueAbility2Id)
	{
		DrawBlueAbility2(canvas, entity, ability);
	}
	else if (ability->id == BlueAbility3Id)
	{
		DrawBlueAbility3(canvas, entity, ability);
	}
	else if (ability->id == RedAbility1Id)
	{
		DrawRedAbility1(canvas, entity, ability);
	}
	else if (ability->id == RedAbility2Id)
	{
		DrawRedAbility3(canvas, entity, ability);
	}
	else if (ability->id == NoAbilityId)
	{
	}
	else
	{
		DebugBreak();
	}
}

static void func DrawExperienceBar(Canvas* canvas, CombatLabState* labState)
{
	I32 level = labState->player.level;
	if (level < MaxLevel)
	{
		Bitmap* bitmap = &canvas->bitmap;
		I32 left = 0;
		I32 right = bitmap->width - 1;
		I32 bottom = bitmap->height - 1;
		I32 top = bottom - 10;
		V4 unfilledColor = MakeColor(0.3f, 0.3f, 0.3f);
		V4 filledColor = MakeColor(0.6f, 0.6f, 0.6f);
		DrawBitmapRect(bitmap, left, right, top, bottom, unfilledColor);

		I32 experienceGained = labState->experienceGainedThisLevel;
		Assert(IsIntBetween(level, 1, MaxLevel - 1));
		I32 experienceNeeded = ExperienceNeededForNextLevel[level];
		I32 rightFilled = left + Floor(F32(right - left) * F32(experienceGained) / F32(experienceNeeded));
		DrawBitmapRect(bitmap, left, rightFilled, top, bottom, filledColor);
	}
}

static void func DrawUIBox(Bitmap* bitmap, I32 centerX, I32 centerY, I32 size, I8* text, GlyphData* glyphData)
{
	I32 left   = centerX - size / 2;
	I32 right  = centerX + size / 2;
	I32 top    = centerY - size / 2;
	I32 bottom = centerY + size / 2;

	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);

	V4 outlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);

	I32 textBaseLineY = bottom - (size - TextHeightInPixels) / 2 - TextPixelsBelowBaseLine;
	F32 textWidth = GetTextPixelWidth(text, glyphData);
	I32 textLeft = centerX - I32(textWidth * 0.5f);

	V4 textColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapTextLine(bitmap, text, glyphData, textLeft, textBaseLineY, textColor);
}

static void func DrawUIBoxBottom(Bitmap* bitmap, I32 centerX, I32 bottom, I32 size, I8* text, GlyphData* glyphData)
{
	I32 centerY = bottom - size / 2;
	DrawUIBox(bitmap, centerX, centerY, size, text, glyphData);
}

static void func DrawTextAlert(Canvas* canvas, TextAlert* alert)
{
	Assert(alert->timeRemaining > 0.0f);
	I8 text[32] = {};
	String string = StartString(text, 32);

	switch (alert->id)
	{
		case DamageTextAlertId:
		{
			string = string + alert->damage + " Damage";
			break;
		}
		case HealTextAlertId:
		{
			string = string + alert->heal + " Heal";
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	DrawTextLineXCentered(canvas, text, alert->baseLineY, alert->centerX, alert->color);
}

static void func DrawInventorySlotBackground(Bitmap* bitmap, I32 top, I32 left, V4 color)
{
	I32 right  = left + InventorySlotSize;
	I32 bottom = top + InventorySlotSize;
	DrawBitmapRect(bitmap, left, right, top, bottom, color);
}

static void func DrawInventorySlotOutline(Bitmap* bitmap, I32 top, I32 left, V4 color)
{
	I32 right  = left + InventorySlotSize;
	I32 bottom = top + InventorySlotSize;
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, color);
}

static void func DrawInventorySlotText(Bitmap* bitmap, GlyphData* glyphData, I8* text, I32 top, I32 left, V4 color)
{
	I32 right  = left + InventorySlotSize;
	I32 bottom = top + InventorySlotSize;
	DrawBitmapTextLineCentered(bitmap, text, glyphData, left, right, top, bottom, color);
}

static void func DrawInventorySlot(Bitmap* bitmap, GlyphData* glyphData, I32 top, I32 left, I32 itemId, I32 slotId)
{
	V4 outlineColor  = MakeColor(0.5f, 0.5f, 0.5f);
	V4 itemNameColor = WhiteColor;
	V4 slotNameColor = MakeColor(0.5f, 0.5f, 0.5f);
	V4 backgroundColor = MakeColor(0.1f, 0.1f, 0.1f);

	DrawInventorySlotBackground(bitmap, top, left, backgroundColor);

	if (itemId != NoItemId)
	{
		I8 itemName[32];
		GetItemShortName(itemId, itemName, 32);
		DrawInventorySlotText(bitmap, glyphData, itemName, top, left, itemNameColor);
	}
	else if (slotId != AnySlotId)
	{
		I8 slotName[32];
		GetSlotName(slotId, slotName, 32);
		DrawInventorySlotText(bitmap, glyphData, slotName, top, left, slotNameColor);
	}

	DrawInventorySlotOutline(bitmap, top, left, outlineColor);
}

static void func DrawInventory(Canvas* canvas, Inventory* inventory, CombatLabState* labState, V2 mousePosition)
{
	Bitmap* bitmap = &canvas->bitmap;
	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	V4 slotBackgroundColor   = MakeColor(0.1f, 0.1f, 0.1f);
	V4 slotOutlineColor      = MakeColor(0.5f, 0.5f, 0.5f);
	V4 itemNameColor         = WhiteColor;

	I32 mouseX = UnitXtoPixel(canvas->camera, mousePosition.x);
	I32 mouseY = UnitYtoPixel(canvas->camera, mousePosition.y);

	I32 left   = inventory->left;
	I32 right  = inventory->left + inventory->colN * InventorySlotSize;
	I32 top    = inventory->top;
	I32 bottom = inventory->top + inventory->rowN * InventorySlotSize;

	I32 slotLeft = left;
	I32 slotTop  = top;

	InventoryItem* hoverItem = &labState->hoverItem;
	InventoryItem* dragItem = &labState->dragItem;

	for (I32 row = 0; row < inventory->rowN; row++)
	{
		for (I32 col = 0; col < inventory->colN; col++)
		{
			I32 itemId = inventory->itemIds[inventory->colN * row + col];
			I32 slotId = inventory->slotIds[inventory->colN * row + col];
			if (dragItem->inventory != inventory || dragItem->itemId == NoItemId || 
				row != dragItem->row || col != dragItem->col)
			{
				DrawInventorySlot(bitmap, glyphData, slotTop, slotLeft, itemId, slotId);
			}
			else
			{
				DrawInventorySlot(bitmap, glyphData, slotTop, slotLeft, NoItemId, slotId);
			}

			I32 slotRight = slotLeft + InventorySlotSize;
			I32 slotBottom = slotTop + InventorySlotSize;
			if (IsIntBetween(mouseX, slotLeft, slotRight) && IsIntBetween(mouseY, slotTop, slotBottom))
			{
				hoverItem->inventory = inventory;
				hoverItem->itemId = itemId;
				hoverItem->slotId = slotId;
				hoverItem->row = row;
				hoverItem->col = col;
			}

			slotLeft += InventorySlotSize;
		}

		slotTop += InventorySlotSize;
		slotLeft = left;
	}

	V4 outlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);

	if (hoverItem->inventory == inventory)
	{
		Assert(IsValidInventoryItem(*hoverItem));
		I32 slotLeft = left + hoverItem->col * InventorySlotSize;
		I32 slotTop  = top + hoverItem->row * InventorySlotSize;

		V4 slotHoverOutlineColor    = MakeColor(1.0f, 1.0f, 0.0f);	
		V4 slotHoverBadOutlineColor = MakeColor(1.0f, 0.0f, 0.0f);

		V4 color = slotHoverOutlineColor;
		InventoryItem* dragItem = &labState->dragItem;
		if (dragItem->inventory != 0 && !ItemGoesIntoSlot(dragItem->itemId, hoverItem->slotId))
		{
			color = slotHoverBadOutlineColor;
		}

		DrawInventorySlotOutline(bitmap, slotTop, slotLeft, color);
	}

	if (hoverItem->inventory == inventory && hoverItem->itemId != NoItemId)
	{

		static I8 tooltipBuffer[256] = {};
		String tooltipString = GetItemTooltipText(hoverItem->itemId, tooltipBuffer, 256);

		I32 slotLeft = left + hoverItem->col * InventorySlotSize;
		I32 slotTop  = top  + hoverItem->row * InventorySlotSize;

		I32 tooltipLeft   = IntMin2(slotLeft - TooltipWidth / 2, (bitmap->width - 1) - TooltipWidth - 5);
		I32 tooltipBottom = slotTop - 5;
		DrawBitmapStringTooltipBottom(bitmap, tooltipString, glyphData, tooltipBottom, tooltipLeft);
	}
}

static void func DrawCharacterInfo(Canvas* canvas, CombatLabState* labState, V2 mousePosition)
{
	Bitmap* bitmap = &canvas->bitmap;
	Entity* entity = &labState->player;

	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	V4 outlineColor = MakeColor(1.0f, 1.0f, 1.0f);

	I32 padding = 5;
	I32 width = 400;
	I32 height = InventorySlotSize + 190 + InventorySlotSize * labState->inventory.rowN;

	I32 right  = (bitmap->width - 1) - padding;
	I32 left   = right - width;
	I32 top    = (bitmap->height - 1) / 2 - height / 2;
	I32 bottom = top + height;

	DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);

	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	V4 textColor  = MakeColor(1.0f, 1.0f, 1.0f);
	V4 titleColor = MakeColor(1.0f, 1.0f, 0.0f);
	V4 infoColor  = MakeColor(0.3f, 0.3f, 0.3f);

	I32 textLeft = left + padding;
	I32 textY = top + InventorySlotSize + padding + TextPixelsAboveBaseLine;

	DrawBitmapTextLine(bitmap, "Character Info", glyphData, textLeft, textY, titleColor);

	I8 lineBuffer[128] = {};
	textY += TextHeightInPixels;
	String string = StartString(lineBuffer, 128);
	AddLine(string, "Level " + entity->level);
	DrawBitmapTextLine(bitmap, lineBuffer, glyphData, textLeft, textY, textColor);

	textY += TextHeightInPixels;
	textY += TextHeightInPixels;
	DrawBitmapTextLine(bitmap, "Attributes", glyphData, textLeft, textY, titleColor);

	textY += TextHeightInPixels;
	string = StartString(lineBuffer, 128);
	AddLine(string, "Constitution: " + entity->constitution);
	DrawBitmapTextLine(bitmap, lineBuffer, glyphData, textLeft, textY, textColor);

	textY += TextHeightInPixels;
	DrawBitmapTextLine(bitmap, "Increases maximum health.", glyphData, textLeft, textY, infoColor);

	textY += TextHeightInPixels;
	string = StartString(lineBuffer, 128);
	AddLine(string, "Strength: " + entity->strength);
	DrawBitmapTextLine(bitmap, lineBuffer, glyphData, textLeft, textY, textColor);

	textY += TextHeightInPixels;
	DrawBitmapTextLine(bitmap, "Increases damage of physical attacks.", glyphData, textLeft, textY, infoColor);

	textY += TextHeightInPixels;
	string = StartString(lineBuffer, 128);
	AddLine(string, "Intellect: " + entity->intellect);
	DrawBitmapTextLine(bitmap, lineBuffer, glyphData, textLeft, textY, textColor);

	textY += TextHeightInPixels;
	DrawBitmapTextLine(bitmap, "Increases damage of magical attacks.", glyphData, textLeft, textY, infoColor);

	textY += TextHeightInPixels;
	string = StartString(lineBuffer, 128);
	AddLine(string, "Dexterity: " + entity->dexterity);
	DrawBitmapTextLine(bitmap, lineBuffer, glyphData, textLeft, textY, textColor);

	textY += TextHeightInPixels;
	DrawBitmapTextLine(bitmap, "Reduces use time of abilities.", glyphData, textLeft, textY, infoColor);

	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);

	labState->hoverItem.inventory = 0;

	labState->equipInventory.left = left;
	labState->equipInventory.top  = top;
	DrawInventory(canvas, &labState->equipInventory, labState, mousePosition);

	labState->inventory.left = left;
	labState->inventory.top = bottom - labState->inventory.rowN * InventorySlotSize;
	DrawInventory(canvas, &labState->inventory, labState, mousePosition);

	InventoryItem* dragItem = &labState->dragItem;
	if (dragItem->inventory != 0)
	{
		I32 slotLeft = dragItem->inventory->left + dragItem->col * InventorySlotSize;
		I32 slotTop  = dragItem->inventory->top  + dragItem->row * InventorySlotSize;

		V4 slotDragOutlineColor  = MakeColor(1.0f, 0.5f, 0.0f);
		DrawInventorySlotOutline(bitmap, slotTop, slotLeft, slotDragOutlineColor);

		I32 mouseX = UnitXtoPixel(canvas->camera, mousePosition.x);
		I32 mouseY = UnitYtoPixel(canvas->camera, mousePosition.y);

		I32 centerX = mouseX - InventorySlotSize / 2;
		I32 centerY = mouseY - InventorySlotSize / 2;
		DrawInventorySlot(bitmap, glyphData, centerY, centerX, dragItem->itemId, AnySlotId);
	}
}

static void func DrawCombatLog(Canvas* canvas, CombatLabState* labState)
{
	Bitmap* bitmap = &canvas->bitmap;

	I32 padding = 5;
	I32 height = padding + CombatLogLineN * TextHeightInPixels + padding;
	I32 width  = 300;

	I32 bottom = (bitmap->height - 1) - 10 - padding;
	I32 top    = bottom - height;
	I32 left   = padding;
	I32 right  = left + width;

	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	V4 outlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);

	I32 textX = left + padding;
	I32 textY = top + padding + TextPixelsAboveBaseLine;
	V4 textColor = MakeColor(1.0f, 1.0f, 1.0f);
	for (I32 i = 0; i < CombatLogLineN; ++i)
	{
		DrawBitmapTextLine(bitmap, labState->combatLogLines[i], canvas->glyphData, textX, textY, textColor);
		textY += TextHeightInPixels;
	}
}

static void func DrawUI(Canvas* canvas, CombatLabState* labState, V2 mousePosition)
{
	DrawExperienceBar(canvas, labState);

	Bitmap* bitmap = &canvas->bitmap;
	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	I32 mouseX = UnitXtoPixel(canvas->camera, mousePosition.x);
	I32 mouseY = UnitYtoPixel(canvas->camera, mousePosition.y);

	I32 boxSize = 40;
	I32 boxPadding = 5;
	I32 boxCount = 3;
	I32 boxBarWidth = boxCount * boxSize + (boxCount - 1) * boxPadding;

	I32 boxCenterX = (bitmap->width - 1) / 2 - boxBarWidth / 2 + boxSize / 2;
	I32 boxCenterY = (bitmap->height - 1) - 10 - boxPadding - boxSize / 2;
	DrawUIBox(bitmap, boxCenterX, boxCenterY, boxSize, "LMB", glyphData);

	static I8 tooltipBuffer[256] = {};
	String tooltipText = StartString(tooltipBuffer, 256);

	if (IsIntBetween(mouseX, boxCenterX - boxSize / 2, boxCenterX + boxSize / 2) &&
		IsIntBetween(mouseY, boxCenterY - boxSize / 2, boxCenterY + boxSize / 2))
	{
		I32 damage = GetAbilityDamage(&labState->player, WhiteAbility1Id);
		F32 castTime = GetAbilityCastTime(&labState->player, WhiteAbility1Id);

		AddLine(tooltipText, "Ability 1");
		AddLine(tooltipText, "Use time: " + castTime + " sec");
		AddLine(tooltipText, "Hit in a direction, dealing " + damage + " damage");
		AddLine(tooltipText, "to enemies closer than " + WhiteAbility1Radius + " meters.");
		AddLine(tooltipText, "Generates 20 energy if it hits anything");

		I32 tooltipLeft = boxCenterX - TooltipWidth / 2;
		I32 tooltipBottom = boxCenterY - boxSize / 2 - boxPadding;
		DrawBitmapStringTooltipBottom(bitmap, tooltipText, glyphData, tooltipBottom, tooltipLeft);
	}

	boxCenterX += boxSize + boxPadding;
	DrawUIBox(bitmap, boxCenterX, boxCenterY, boxSize, "Q", glyphData);
	if (IsIntBetween(mouseX, boxCenterX - boxSize / 2, boxCenterX + boxSize / 2) &&
		IsIntBetween(mouseY, boxCenterY - boxSize / 2, boxCenterY + boxSize / 2))
	{
		I32 damage = GetAbilityDamage(&labState->player, WhiteAbility2Id);

		AddLine(tooltipText, "Ability 2");
		AddLine(tooltipText, "Energy cost: " + WhiteAbility2ResourceCost);
		AddLine(tooltipText, "Charge in a direction, dealing " + damage + " damage");
		AddLine(tooltipText, "to enemies in your way.");
		AddLine(tooltipText, "Requires level 2");

		I32 tooltipLeft = boxCenterX - TooltipWidth / 2;
		I32 tooltipBottom = boxCenterY - boxSize / 2 - boxPadding;
		DrawBitmapStringTooltipBottom(bitmap, tooltipText, glyphData, tooltipBottom, tooltipLeft);
	}

	boxCenterX += boxSize + boxPadding;
	DrawUIBox(bitmap, boxCenterX, boxCenterY, boxSize, "E", glyphData);
	if (IsIntBetween(mouseX, boxCenterX - boxSize / 2, boxCenterX + boxSize / 2) &&
		IsIntBetween(mouseY, boxCenterY - boxSize / 2, boxCenterY + boxSize / 2))
	{
		I32 damage = GetAbilityDamage(&labState->player, WhiteAbility3Id);
		F32 castTime = GetAbilityCastTime(&labState->player, WhiteAbility3Id);

		AddLine(tooltipText, "Ability 3");
		AddLine(tooltipText, "Energy cost: " + WhiteAbility3ResourceCost);
		AddLine(tooltipText, "Use time: " + castTime + " sec");
		AddLine(tooltipText, "Spin around, kicking enemies closer than");
		AddLine(tooltipText, WhiteAbility3Radius + " meters, knocking them away, dealing");
		AddLine(tooltipText, damage + " damage and interrupting their abilities.");
		AddLine(tooltipText, "Requires level 3.");

		I32 tooltipLeft = boxCenterX - TooltipWidth / 2;
		I32 tooltipBottom = boxCenterY - boxSize / 2 - boxPadding;
		DrawBitmapStringTooltipBottom(bitmap, tooltipText, glyphData, tooltipBottom, tooltipLeft);
	}

	I32 smallBoxSize = 30;
	I32 smallBoxCenterX = (bitmap->width - 1) - boxPadding - smallBoxSize / 2;
	I32 smallBoxCenterY = (bitmap->height - 1) - 10 - boxPadding - smallBoxSize / 2;
	DrawUIBox(bitmap, smallBoxCenterX, smallBoxCenterY, smallBoxSize, "C", glyphData);

	DrawCombatLog(canvas, labState);

	if (labState->showCharacterInfo)
	{
		DrawCharacterInfo(canvas, labState, mousePosition);
	}
}

static void func DrawAndUpdateTextAlerts(Canvas* canvas, CombatLabState* labState, F32 seconds)
{
	for (I32 i = 0; i < labState->textAlertN; ++i)
	{
		TextAlert* alert = &labState->textAlerts[i];
		if (alert->timeRemaining > 0.0f)
		{
			F32 scrollUpSpeed = 1.0f;
			alert->baseLineY -= scrollUpSpeed * seconds;

			DrawTextAlert(canvas, alert);
			alert->timeRemaining -= seconds;
			if (alert->timeRemaining <= 0.0f)
			{
				alert->timeRemaining = 0.0f;
			}
		}
	}

	I32 remainingN = 0;
	for (I32 i = 0; i < labState->textAlertN; ++i)
	{
		TextAlert* alert = &labState->textAlerts[i];
		if (alert->timeRemaining > 0.0f)
		{
			labState->textAlerts[remainingN] = *alert;
			remainingN++;
		}
	}

	labState->textAlertN = remainingN;
}

static F32 func GetEntityDistance(Entity* entity1, Entity* entity2)
{
	F32 distance = Distance(entity1->position, entity2->position) - entity1->radius - entity2->radius;
	return distance;
}

static void func UpdateDroppedItems(Canvas* canvas, CombatLabState* labState, V2 mousePosition, F32 seconds)
{
	labState->hoverDroppedItem = 0;

	I32 itemIndex = 0;
	for (I32 i = 0; i < labState->droppedItemN; i++)
	{
		DroppedItem* item = &labState->droppedItems[i];
		item->timeLeft -= seconds;
		if (item->timeLeft > 0.0f)
		{
			labState->droppedItems[itemIndex] = *item;
			itemIndex++;
		}
	}
	labState->droppedItemN = itemIndex;

	for (I32 i = 0; i < labState->droppedItemN; i++)
	{
		DroppedItem* item = &labState->droppedItems[i];
		I8 itemName[32];
		GetItemName(item->itemId, itemName, 32);
		V4 normalBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
		V4 hoverBackgroundColor  = MakeColor(0.7f, 0.5f, 0.5f);
		V4 color = MakeColor(1.0f, 1.0f, 1.0f);

		F32 textWidth  = GetTextWidth(canvas, itemName);
		F32 textHeight = GetTextHeight(canvas, itemName);

		Camera* camera = canvas->camera;
		Assert(camera->unitInPixels > 0.0f);
		F32 left   = item->position.x - textWidth * 0.5f;
		F32 right  = item->position.x + textWidth * 0.5f;
		F32 top    = item->position.y - TextPixelsAboveBaseLine / camera->unitInPixels;
		F32 bottom = item->position.y + TextPixelsBelowBaseLine / camera->unitInPixels;

		V4 backgroundColor = normalBackgroundColor;
		if (IsPointInRect(mousePosition, left, right, top, bottom))
		{
			backgroundColor = hoverBackgroundColor;
			labState->hoverDroppedItem = item;
		}

		DrawRect(canvas, left, right, top, bottom, backgroundColor);
		DrawTextLineXCentered(canvas, itemName, item->position.y, item->position.x, color);
	}
}

static void func CombatLabUpdate(CombatLabState* labState, V2 mousePosition, F32 seconds)
{
	I32 secondsPassed = 0;
	labState->secondsFraction += seconds;
	while (labState->secondsFraction > 1.0f)
	{
		secondsPassed++;
		labState->secondsFraction -= 1.0f;
	}

	Entity* player = &labState->player;
	Ability* playerAbility = &player->ability;

	F32 playerMoveSpeed = 10.0f;
	if (player->slowTimeLeft > 0.0f)
	{
		playerMoveSpeed *= 0.5f;
		player->slowTimeLeft = Max2(0.0f, player->slowTimeLeft - seconds);
		if (player->slowTimeLeft == 0.0f)
		{
			CombatLog(labState, player->name + " is no longer slowed.");
		}
	}
	Assert(player->slowTimeLeft >= 0.0f);

	V2 targetPosition = mousePosition;
	// V2 moveDirection = PointDirection(player->position, targetPosition);
	V2 moveDirection = MakeVector(0.0f, 0.0f);
	if (labState->moveLeft)
	{
		moveDirection.x -= 1.0f;
	}
	if (labState->moveRight)
	{
		moveDirection.x += 1.0f;
	}

	if (labState->moveUp)
	{
		moveDirection.y -= 1.0f;
	}
	if (labState->moveDown)
	{
		moveDirection.y += 1.0f;
	}

	player->velocity = playerMoveSpeed * NormalVector(moveDirection);

	if (player->health > 0.0f && playerAbility->castRatioLeft == 0.0f)
	{
		player->position = player->position + seconds * player->velocity;
	}

	Canvas* canvas = &labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 gridColor = MakeColor(0.3f, 0.3f, 0.0f);
	F32 gridDistance = 10.0f;

	Camera* camera = &labState->camera;
	camera->center = player->position;
	F32 left   = CameraLeftSide(camera);
	F32 right  = CameraRightSide(camera);
	F32 top    = CameraTopSide(camera);
	F32 bottom = CameraBottomSide(camera);

	I32 firstCol = Floor(left / gridDistance);
	I32 lastCol  = Floor(right / gridDistance);
	for (I32 col = firstCol; col <= lastCol; ++col)
	{
		F32 x = (col * gridDistance);
		V2 point1 = MakePoint(x, top);
		V2 point2 = MakePoint(x, bottom);
		Bresenham(canvas, point1, point2, gridColor);
	}

	I32 firstRow = Floor(top / gridDistance);
	I32 lastRow  = Floor(bottom /gridDistance);
	for (I32 row = firstRow; row <= lastRow; ++row)
	{
		F32 y = (row * gridDistance);
		V2 point1 = MakePoint(left, y);
		V2 point2 = MakePoint(right, y);
		Bresenham(canvas, point1, point2, gridColor);
	}

	DrawAbility(canvas, player, &player->ability);

	V4 enemyAbilityColor = MakeColor(1.0f, 0.5f, 0.0f);
	for (I32 i = 0; i < CombatLabEnemyN; ++i)
	{
		Entity* enemy = &labState->enemies[i];
		DrawAbility(canvas, enemy, &enemy->ability);
	}

	if (player->health > 0)
	{
		if (playerAbility->castRatioLeft > 0.0f)
		{
			F32 totalCastTime = GetAbilityCastTime(player, playerAbility->id);
			F32 castTimeLeft = playerAbility->castRatioLeft * totalCastTime;
			castTimeLeft = Max2(0.0f, castTimeLeft - seconds);
			playerAbility->castRatioLeft = castTimeLeft / totalCastTime;

			if (playerAbility->id == WhiteAbility1Id)
			{
				if (playerAbility->castRatioLeft == 0.0f)
				{
					F32 minAngle = NormalizeAngle(playerAbility->angle - WhiteAbility1AngleRange * 0.5f);
					F32 maxAngle = NormalizeAngle(playerAbility->angle + WhiteAbility1AngleRange * 0.5f);

					F32 hitAnyEnemy = false;
					for (I32 i = 0; i < CombatLabEnemyN; ++i)
					{
						Entity* enemy = &labState->enemies[i];
						if (IsEntityInSlice(enemy, player->position, WhiteAbility1Radius, minAngle, maxAngle))
						{
							B32 enemyWasAlive = (enemy->health > 0);
							I32 damage = GetAbilityDamage(player, WhiteAbility1Id);
							DoDamage(labState, player, enemy, damage);
							B32 enemyIsDead = (enemy->health == 0);

							if (enemyWasAlive)
							{
								hitAnyEnemy = true;
							}
						}
					}

					if (hitAnyEnemy)
					{
						GainSpecialResource(player, 20);
					}
				}
			}
			else if (playerAbility->id == WhiteAbility2Id)
			{
				V2 moveDirection = RotationVector(playerAbility->angle);
				player->position = player->position + seconds * WhiteAbility2MoveSpeed * moveDirection;
				if (playerAbility->castRatioLeft == 0.0f)
				{
					V2 oldPosition = player->position - WhiteAbility2TotalCastTime * WhiteAbility2MoveSpeed * moveDirection;
					I32 damage = GetAbilityDamage(player, WhiteAbility2Id);
					DamageEnemiesOnLine(labState, oldPosition, player->position, damage);
				}
			}
			else if (playerAbility->id == WhiteAbility3Id)
			{
				if (playerAbility->castRatioLeft == 0.0f)
				{
					for (I32 i = 0; i < CombatLabEnemyN; ++i)
					{
						Entity* enemy = &labState->enemies[i];
						if (enemy->health == 0)
						{
							continue;
						}

						F32 playerEnemyDistance = GetEntityDistance(player, enemy);
						if (playerEnemyDistance < WhiteAbility3Radius)
						{
							V2 direction = PointDirection(player->position, enemy->position);
							F32 distance = player->radius + enemy->radius + WhiteAbility3Radius;
							enemy->position = player->position + distance * direction;
							InterruptAbility(enemy);
							I32 damage = GetAbilityDamage(player, WhiteAbility3Id);
							DoDamage(labState, player, enemy, damage);
						}
					}
				}
			}
			else
			{
				DebugBreak();
			}
		}
	
		Assert(playerAbility->castRatioLeft >= 0.0f);
		if (playerAbility->castRatioLeft == 0.0f)
		{
			if (labState->useAbilityId == WhiteAbility1Id)
			{
				playerAbility->id = WhiteAbility1Id;
				playerAbility->angle = LineAngle(player->position, mousePosition);
				playerAbility->castRatioLeft = 1.0f;
			}
			else if (labState->useAbilityId == WhiteAbility2Id)
			{
				if (player->level >= 2)
				{
					Assert(player->hasSpecialResource);
					if (player->specialResource >= WhiteAbility2ResourceCost)
					{
						player->specialResource -= WhiteAbility2ResourceCost;
						playerAbility->id = WhiteAbility2Id;
						playerAbility->angle = LineAngle(player->position, mousePosition);
						playerAbility->castRatioLeft = 1.0f;
					}
				}
			}
			else if (labState->useAbilityId == WhiteAbility3Id)
			{
				if (player->level >= 3)
				{
					Assert(player->hasSpecialResource);
					if (player->specialResource >= WhiteAbility3ResourceCost)
					{
						player->specialResource -= WhiteAbility3ResourceCost;
						playerAbility->id = WhiteAbility3Id;
						playerAbility->castRatioLeft = 1.0f;
					}
				}
			}
			else
			{
				Assert(labState->useAbilityId == NoAbilityId);
			}
		}
	}

	if (player->health > 0)
	{
		Entity* attackingEnemies[CombatLabEnemyN];

		I32 attackingEnemyN = 0;
		for (I32 i = 0; i < CombatLabEnemyN; ++i) 
		{
			Entity* enemy = &labState->enemies[i];

			F32 followDistance = 10.0f;

			if (enemy->health > 0)
			{
				F32 distanceFromPlayer = GetEntityDistance(enemy, player);
				if (distanceFromPlayer <= followDistance)
				{
					enemy->isAttacking = true;
				}

				if (enemy->isAttacking)
				{
					enemy->attackFromAngle = LineAngle(player->position, enemy->position);
					attackingEnemies[attackingEnemyN] = enemy;
					attackingEnemyN++;
				}
				else
				{
					Ability* ability = &enemy->ability;
					ability->castRatioLeft = 0.0f;
				}
			}
		}

		for (I32 i = 0; i < attackingEnemyN; ++i)
		{
			for (I32 j = i + 1; j < attackingEnemyN; ++j)
			{
				if (attackingEnemies[i]->attackFromAngle > attackingEnemies[j]->attackFromAngle)
				{
					Entity* enemy = attackingEnemies[j];
					attackingEnemies[j] = attackingEnemies[i];
					attackingEnemies[i] = enemy;
				}
			}
		}

		if (attackingEnemyN >= 2)
		{
			F32 minAngleBetweenEnemies = (1.0f / F32(attackingEnemyN)) * TAU;
			minAngleBetweenEnemies = Min2(minAngleBetweenEnemies, 0.125f * TAU);
			F32 lastAngle = 0.0f;
			for (I32 i = 0; i < attackingEnemyN; ++i)
			{
				I32 prev = (i > 0) ? (i - 1) : (attackingEnemyN - 1);
				Entity* leftEnemy = attackingEnemies[prev];
				Entity* rightEnemy = attackingEnemies[i];
				F32 leftAngle = LineAngle(player->position, leftEnemy->position);
				F32 rightAngle = LineAngle(player->position, rightEnemy->position);
				F32 angleDifference = AngleDifference(leftAngle, rightAngle);
				if (angleDifference < minAngleBetweenEnemies)
				{
					F32 angleToAdd = minAngleBetweenEnemies - angleDifference;
					leftEnemy->attackFromAngle = NormalizeAngle(leftEnemy->attackFromAngle - 0.5f * angleToAdd);
					rightEnemy->attackFromAngle = NormalizeAngle(rightEnemy->attackFromAngle + 0.5f * angleToAdd);
				}
			}
		}

		for (I32 i = 0; i < attackingEnemyN; ++i)
		{
			Entity* enemy = attackingEnemies[i];
			Assert(enemy->health >= 0.0f);

			if (enemy->typeId == RedEntityId && enemy->level >= 2)
			{
				GainSpecialResource(enemy, 10 * secondsPassed);
			}

			if (enemy->typeId == BlueEntityId)
			{
				if (enemy->level >= 2)
				{
					GainSpecialResource(enemy, 10 * secondsPassed);
				}

				if (enemy->level >= 3)
				{
					Assert(enemy->hasSpecialResource);
					if (enemy->specialResource >= 50)
					{
						F32 distanceFromPlayer = GetEntityDistance(enemy, player);
						if (distanceFromPlayer <= 1.0f && player->slowTimeLeft < 1.0f)
						{
							player->slowTimeLeft = 1.0f;
						}
					}
				}
			}

			F32 enemyMoveSpeed = 5.0f;
			F32 attackDistance = 0.0f;
			if (enemy->typeId == RedEntityId)
			{
				enemyMoveSpeed = 7.5f;
				attackDistance = 4.0f;
				if (enemy->level >= 3)
				{
					Assert(enemy->hasSpecialResource);
					if (enemy->specialResource >= 50)
					{
						enemyMoveSpeed *= 2.0f;
					}
				}
			}
			else if (enemy->typeId == BlueEntityId)
			{
				attackDistance = 10.0f;
			}
			else
			{
				DebugBreak();
			}

			V2 targetPosition = player->position + attackDistance * RotationVector(enemy->attackFromAngle);
			F32 distanceFromTarget = Distance(enemy->position, targetPosition);

			Ability* ability = &enemy->ability;
			if (ability->castRatioLeft > 0.0f)
			{
				F32 totalCastTime = GetAbilityCastTime(enemy, ability->id);
				F32 castTimeLeft = ability->castRatioLeft * totalCastTime;
				castTimeLeft = Max2(0.0f, castTimeLeft - seconds);
				ability->castRatioLeft = castTimeLeft / totalCastTime;

				if (ability->id == RedAbility2Id)
				{
					Assert(enemy->level >= 2);
					ability->angle = LineAngle(enemy->position, player->position);
				}
				else if (ability->id == BlueAbility3Id)
				{
					Assert(enemy->level >= 3);
					ability->position = player->position;
				}

				if (ability->castRatioLeft == 0.0f)
				{
					if (ability->id == RedAbility1Id)
					{
						F32 minAngle = NormalizeAngle(ability->angle - RedAbility1AngleRange * 0.5f);
						F32 maxAngle = NormalizeAngle(ability->angle + RedAbility1AngleRange * 0.5f);
						if (IsEntityInSlice(player, enemy->position, RedAbility1Radius, minAngle, maxAngle))
						{
							I32 damage = GetAbilityDamage(enemy, RedAbility1Id);
							DoDamage(labState, enemy, player, damage);
						}
					}
					else if (ability->id == RedAbility2Id)
					{
						F32 minAngle = NormalizeAngle(ability->angle - RedAbility2AngleRange * 0.5f);
						F32 maxAngle = NormalizeAngle(ability->angle + RedAbility2AngleRange * 0.5f);
						if (IsEntityInSlice(player, enemy->position, RedAbility2Radius, minAngle, maxAngle))
						{
							I32 damage = GetAbilityDamage(enemy, RedAbility2Id);
							DoDamage(labState, enemy, player, damage);
						}
					}
					else if (ability->id == BlueAbility1Id)
					{
						Assert(enemy->typeId == BlueEntityId);
						if (IsEntityInCircle(player, ability->position, BlueAbility1Radius))
						{
							I32 damage = GetAbilityDamage(enemy, BlueAbility1Id);
							DoDamage(labState, enemy, player, damage);
						}
					}
					else if (ability->id == BlueAbility2Id)
					{
						Assert(enemy->typeId == BlueEntityId);
						F32 distanceFromPlayer = GetEntityDistance(enemy, player);
						if (distanceFromPlayer <= BlueAbility2Radius)
						{
							I32 damage = GetAbilityDamage(enemy, BlueAbility2Id);
							DoDamage(labState, enemy, player, damage);
							SlowEntity(labState, enemy, player, BlueAbility2SlowTime);
						}
					}
					else if (ability->id == BlueAbility3Id)
					{
						Assert(enemy->typeId == BlueEntityId);
						if (IsEntityInCircle(player, ability->position, BlueAbility3Radius))
						{
							I32 damage = GetAbilityDamage(enemy, BlueAbility3Id);
							DoDamage(labState, enemy, player, damage);
							SlowEntity(labState, enemy, player, BlueAbility3SlowTime);
						}
					}
					else
					{
						DebugBreak();
					}
				}
			}

			B32 canMove = false;
			if (ability->castRatioLeft == 0.0f)
			{
				canMove = true;
			}
			else if (ability->id == RedAbility2Id && enemy->level >= 3)
			{
				canMove = true;
			}
			else
			{
				canMove = false;
			}

			if (canMove)
			{
				V2 moveDirection = PointDirection(enemy->position, targetPosition);
				enemy->position = enemy->position + seconds * enemyMoveSpeed * moveDirection;
			}
	
			if (ability->castRatioLeft == 0.0f)
			{
				if (enemy->typeId == BlueEntityId && enemy->level == 2)
				{
					F32 distanceFromPlayer = GetEntityDistance(enemy, player);
					Assert(enemy->hasSpecialResource);
					if (distanceFromPlayer <= BlueAbility2Radius && enemy->specialResource >= BlueAbility2ResourceCost)
					{
						ability->id = BlueAbility2Id;
						ability->castRatioLeft = 1.0f;
						enemy->specialResource -= BlueAbility2ResourceCost;
					}
				}
				else if (enemy->typeId == BlueEntityId && enemy->level == 3)
				{	
					Assert(enemy->hasSpecialResource);
					if (enemy->specialResource >= BlueAbility3ResourceCost)
					{
						ability->id = BlueAbility3Id;
						ability->position = player->position;
						ability->castRatioLeft = 1.0f;
						enemy->specialResource -= BlueAbility3ResourceCost;
					}
				}

				if (ability->castRatioLeft == 0.0f && distanceFromTarget < 0.1f)
				{
					if (enemy->typeId == RedEntityId)
					{
						bool useRedAbility = true;
						if (enemy->level >= 2)
						{
							Assert(enemy->hasSpecialResource);
							if (enemy->specialResource >= 100)
							{
								ability->id = RedAbility2Id;
								ability->angle = LineAngle(enemy->position, player->position);
								ability->castRatioLeft = 1.0f;
								enemy->specialResource -= 100;
								useRedAbility = false;
							}
						}

						if (useRedAbility)
						{
							ability->id = RedAbility1Id;
							ability->angle = LineAngle(enemy->position, player->position);
							ability->castRatioLeft = 1.0f;
						}
					}
					else if (enemy->typeId == BlueEntityId)
					{
						ability->id = BlueAbility1Id;
						ability->position = player->position;
						ability->castRatioLeft = 1.0f;
					}
					else
					{
						DebugBreak();
					}
				}
			}
		}
	}
	else
	{
		Assert(player->health == 0);
		for (I32 i = 0; i < CombatLabEnemyN; ++i)
		{
			Entity* enemy = &labState->enemies[i];
			if (enemy->ability.castRatioLeft > 0.0f)
			{
				InterruptAbility(enemy);
			}
		}
	}

	UpdateDroppedItems(canvas, labState, mousePosition, seconds);

	DrawEntity(canvas, &labState->player);
	for (I32 i = 0; i < CombatLabEnemyN; i++)
	{
		Entity* enemy = &labState->enemies[i];
		DrawEntity(canvas, enemy);
	}

	DrawAndUpdateTextAlerts(canvas, labState, seconds);

	DrawUI(canvas, labState, mousePosition);
}

static void func CombatLab(HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = CombatLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "CombatLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"CombatLab",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instance,
		0
	);
	Assert(window != 0);

	CombatLabState* labState = &gCombatLabState;

	RECT rect = {};
	GetClientRect(window, &rect);
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	CombatLabInit(labState, width, height);

	LARGE_INTEGER counterFrequency;
	QueryPerformanceFrequency(&counterFrequency);

	LARGE_INTEGER lastCounter;
	QueryPerformanceCounter(&lastCounter);

	MSG message = {};
	while (labState->running) 
	{
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		I64 microSeconds = counter.QuadPart - lastCounter.QuadPart;
		F32 milliSeconds = ((F32)microSeconds * 1000.0f) / F32(counterFrequency.QuadPart);
		F32 seconds = 0.001f * milliSeconds;
		lastCounter = counter;

		V2 mousePosition = GetMousePosition(&labState->camera, window);
		CombatLabUpdate(labState, mousePosition, seconds);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		CombatLabBlit(&labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}