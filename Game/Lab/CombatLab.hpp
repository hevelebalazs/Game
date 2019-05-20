#pragma once

#include "../Ability.hpp"
#include "../Effect.hpp"
#include "../Item.hpp"
#include "../Map.hpp"
#include "../UserInput.hpp"

#define EntityRadius 1.0f

#define MaxLevel 5
Int32 MaxHealthAtLevel[] =
{
	0,
	100,
	150,
	200,
	250,
	300
};

enum EntityGroupId
{
	PlayerGroupId,
	EnemyGroupId
};

struct Entity
{
	Int8* name;

	Int32 level;

	Vec2 position;
	Vec2 velocity;

	Real32 recharge;
	Real32 rechargeFrom;
	Int32 health;
	Int32 absorbDamage;

	Int32 castedAbility;
	Real32 castTimeTotal;
	Real32 castTimeRemaining;
	Entity* castTarget;

	Int32 classId;
	Int32 groupId;
	Entity* target;

	Int32 strength;
	Int32 intellect;
	Int32 constitution;
	Int32 dexterity;

	Int32 herbalism;

	Vec2 inputDirection;
};

struct AbilityCooldown
{
	Entity* entity;
	Int32 abilityId;
	Real32 timeRemaining;
};

struct ItemCooldown
{
	Entity* entity;
	Int32 itemId;
	Real32 timeRemaining;
};

#define DamageDisplayDuration 2.0f
struct DamageDisplay
{
	Real32 timeRemaining;
	Vec2 position;
	Int32 damage;
};

struct Effect
{
	Entity* entity;
	Int32 effectId;
	Real32 timeRemaining;
};

struct HateTableEntry
{
	Entity* source;
	Entity* target;
	Int32 value;
};

#define MaxHateTableEntryN 64
struct HateTable
{
	HateTableEntry entries[MaxHateTableEntryN];
	Int32 entryN;
};

#define DroppedItemTotalDuration 10.0f

struct DroppedItem
{
	Vec2 position;
	Int32 itemId;
	Real32 timeLeft;
};

struct Flower
{
	Vec2 position;
	Int32 itemId;
};

// #define EntityN 256
#define EntityN 2
#define MaxAbilityCooldownN 64
#define MaxItemCooldownN 64
#define MaxEffectN 64
#define MaxDamageDisplayN 64
#define MaxCombatLogLineN 32
#define MaxCombatLogLineLength 64
#define MaxDroppedItemN 32
#define MaxFlowerN 256
#define CombatLabArenaSize (2 * MegaByte)
struct CombatLabState
{
	Int8 arenaMemory[CombatLabArenaSize];
	MemArena arena;

	Map map;

	AbilityCooldown abilityCooldowns[MaxAbilityCooldownN];
	Int32 abilityCooldownN;

	ItemCooldown itemCooldowns[MaxItemCooldownN];
	Int32 itemCooldownN;

	Effect effects[MaxEffectN];
	Int32 effectN;

	Entity entities[EntityN];

	DroppedItem droppedItems[MaxDroppedItemN];
	Int32 droppedItemN;

	Flower flowers[MaxFlowerN];
	Int32 flowerN;
	Flower* hoverFlower;

	DroppedItem* hoverDroppedItem;

	DamageDisplay damageDisplays[MaxDamageDisplayN];
	Int32 damageDisplayN;

	Int8 combatLogLines[MaxCombatLogLineN][MaxCombatLogLineLength + 1];
	Int32 combatLogLastLineIndex;
	Int32 combatLogLineN;

	HateTable hateTable;

	Int32 hoverAbilityId;

	Bool32 showCharacterInfo;
	Inventory equipInventory;

	Bool32 showInventory;
	Inventory inventory;

	InventoryItem hoverItem;
	InventoryItem dragItem;
};

#define EntityMoveSpeed 11.0f
#define AnimalMoveSpeed 10.0f

#define EnemyAttackRadius 10.0f

#define MaxMeleeAttackDistance (4.0f * EntityRadius)
#define MaxRangedAttackDistance (30.0f)

static Int32
func GetNextCombatLogLineIndex(Int32 index)
{
	Int32 nextIndex = (index < MaxCombatLogLineN - 1) ? (index + 1) : 0;
	return nextIndex;
}

static void
func AddCombatLogLine(CombatLabState* labState, Int8* line)
{
	Int32 lastIndex = labState->combatLogLastLineIndex;
	Int32 nextIndex = GetNextCombatLogLineIndex(lastIndex);

	Bool32 hasZeroByte = false;
	for(Int32 i = 0; i < MaxCombatLogLineLength; i++)
	{
		labState->combatLogLines[nextIndex][i] = line[i];
		if(line[i] == 0)
		{
			hasZeroByte = true;
			break;
		}
	}
	Assert(hasZeroByte);

	labState->combatLogLastLineIndex = nextIndex;
	labState->combatLogLineN++;
}

#define CombatLog(labState, line) {Int8 logLine[MaxCombatLogLineLength + 1]; \
								   OneLineString(logLine, MaxCombatLogLineLength + 1, line); \
								   AddCombatLogLine(labState, logLine);}

static void
func AddCombatLogStringLine(CombatLabState* labState, String line)
{
	AddCombatLogLine(labState, line.buffer);
}

static Vec2
func FindEntityStartPosition(Map* map, Real32 x, Real32 y)
{
	Vec2 initialPosition = MakePoint(x, y);
	IntVec2 initialTile = GetContainingTile(map, initialPosition);
	IntVec2 startTile = FindNearbyNonTreeTile(map, initialTile);
	Vec2 startPosition = GetTileCenter(map, startTile);
	return startPosition;
}

static Int32
func GetEntityMaxHealth(Entity* entity)
{
	Assert(IsIntBetween(entity->level, 1, 5));
	Int32 maxHealth = MaxHealthAtLevel[entity->level] + entity->constitution * 10;
	return maxHealth;
}

static void
func CombatLabInit(CombatLabState* labState, Canvas* canvas)
{
	labState->arena = CreateMemArena(labState->arenaMemory, CombatLabArenaSize);
	labState->map = GenerateForestMap(&labState->arena);

	Map* map = &labState->map;
	map->tileSide = EntityRadius * 3.0f;

	canvas->glyphData = GetGlobalGlyphData();

	Camera* camera = canvas->camera;
	camera->unitInPixels = 15.0f;
	camera->center = MakePoint(0.0f, 0.0f);

	Real32 mapWidth  = GetMapWidth(map);
	Real32 mapHeight = GetMapHeight(map);

	Entity* player = &labState->entities[0];
	player->level = 1;
	player->name = "Player";
	player->position = FindEntityStartPosition(map, mapWidth * 0.5f, mapHeight * 0.5f);
	IntVec2 playerTile = GetContainingTile(map, player->position);
	player->inputDirection = MakePoint(0.0f, 0.0f);
	player->classId = DruidClassId;
	player->groupId = PlayerGroupId;

	player->strength = 1;
	player->constitution = 1;
	player->dexterity = 1;
	player->intellect = 1;

	player->herbalism = 1;

	player->health = GetEntityMaxHealth(player);

	Int32 firstSnakeIndex = 1;
	Int32 lastSnakeIndex = EntityN / 2;
	for(Int32 i = firstSnakeIndex; i <= lastSnakeIndex; i++)
	{
		Entity* enemy = &labState->entities[i];
		enemy->level = 1;
		enemy->name = "Snake";
		enemy->position = GetRandomGroundTileCenter(map);
		enemy->health = GetEntityMaxHealth(enemy);
		enemy->classId = SnakeClassId;
		enemy->groupId = EnemyGroupId;
	}

	Int32 firstCrocodileIndex = lastSnakeIndex + 1;
	Int32 lastCrocodileIndex = EntityN - 1;
	for(Int32 i = firstCrocodileIndex; i <= lastCrocodileIndex; i++)
	{
		Entity* enemy = &labState->entities[i];
		enemy->level = 2;
		enemy->name = "Crocodile";
		enemy->position = GetRandomWaterTileCenter(map);
		enemy->health = GetEntityMaxHealth(enemy);
		enemy->classId = CrocodileClassId;
		enemy->groupId = EnemyGroupId;
	}

	MemArena* arena = &labState->arena;

	Inventory* inventory = &labState->inventory;
	InitInventory(inventory, arena, 3, 4);
	AddItemToInventory(inventory, HealthPotionItemId);
	AddItemToInventory(inventory, AntiVenomItemId);
	AddItemToInventory(inventory, IntellectPotionItemId);
	AddItemToInventory(inventory, TestHelmItemId);

	Inventory* equipInventory = &labState->equipInventory;
	InitInventory(equipInventory, arena, 1, 6);
	SetInventorySlotId(equipInventory, 0, 0, HeadSlotId);
	SetInventorySlotId(equipInventory, 0, 1, ChestSlotId);
	SetInventorySlotId(equipInventory, 0, 2, HandsSlotId);
	SetInventorySlotId(equipInventory, 0, 3, WaistSlotId);
	SetInventorySlotId(equipInventory, 0, 4, LegsSlotId);
	SetInventorySlotId(equipInventory, 0, 5, FeetSlotId);

	labState->flowerN = MaxFlowerN;
	for(Int32 i = 0; i < labState->flowerN; i++)
	{
		Flower* flower = &labState->flowers[i];
		flower->position = GetRandomGroundTileCenter(&labState->map);
		flower->itemId = GetRandomFlowerItemId();
	}
}

#define TileGridColor MakeColor(0.2f, 0.2f, 0.2f)

static Bool32
func AbilityIsOnCooldown(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Bool32 isOnCooldown = false;
	for(Int32 i = 0; i < labState->abilityCooldownN; i++)
	{
		AbilityCooldown* cooldown = &labState->abilityCooldowns[i];
		if(cooldown->entity == entity && cooldown->abilityId == abilityId)
		{
			isOnCooldown = true;
			break;
		}
	}
	return isOnCooldown;
}

static Bool32
func IsDead(Entity* entity)
{
	Bool32 isDead = (entity->health == 0);
	return isDead;
}

static Bool32
func HasEffect(CombatLabState* labState, Entity* entity, Int32 effectId)
{
	Bool32 hasEffect = false;
	for(Int32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		if(effect->entity == entity && effect->effectId == effectId)
		{
			hasEffect = true;
			break;
		}
	}
	return hasEffect;
}

static Bool32
func AbilityIsEnabled(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Bool32 enabled = false;
	Entity* target = entity->target;

	Int32 abilityClassId = GetAbilityClass(abilityId);

	Bool32 hasLivingEnemyTarget = (target != 0) && (!IsDead(target)) && (entity->groupId != target->groupId);
	Bool32 hasLivingFriendlyTarget = (target != 0) && (!IsDead(target)) && (entity->groupId == target->groupId);
	Real32 distanceFromTarget = (target != 0) ? MaxDistance(entity->position, target->position) : 0.0f;

	if(IsDead(entity))
	{
		enabled = false;
	}
	else if(abilityClassId != entity->classId)
	{
		DebugBreak();
		enabled = false;
	}
	else if(entity->castedAbility != NoAbilityId)
	{
		enabled = false;
	}
	else
	{
		switch(abilityId)
		{
			case SmallPunchAbilityId:
			case BigPunchAbilityId:
			case KickAbilityId:
			case SwordStabAbilityId:
			case SnakeStrikeAbilityId:
			case CrocodileBiteAbilityId:
			case CrocodileLashAbilityId:
			{
				enabled = hasLivingEnemyTarget && (distanceFromTarget <= MaxMeleeAttackDistance);
				break;
			}
			case RollAbilityId:
			{
				enabled = (entity->inputDirection.x != 0.0f || entity->inputDirection.y != 0.0f);
				break;
			}
			case LightningAbilityId:
			case EarthShakeAbilityId:
			case BurnAbilityId:
			{
				enabled = hasLivingEnemyTarget && (distanceFromTarget <= MaxRangedAttackDistance);
				break;
			}
			case HealAbilityId:
			{
				Map* map = &labState->map;
				IntVec2 entityTile = GetContainingTile(map, entity->position);
				Bool32 isNearWater = TileIsNearOrInWater(map, entityTile);
				enabled = (hasLivingFriendlyTarget && isNearWater);
				break;
			}
			case EarthShieldAbilityId:
			{
				enabled = hasLivingFriendlyTarget;
				break;
			}
			case SpinningKickAbilityId:
			case AvoidanceAbilityId:
			case SwordSwingAbilityId:
			case RaiseShieldAbilityId:
			case LightOfTheSunAbilityId:
			case BlessingOfTheSunAbilityId:
			case MercyOfTheSunAbilityId:
			{
				enabled = true;
				break;
			}
			default:
			{
				DebugBreak();
			}
		}
	}

	if(entity->classId == PaladinClassId && HasEffect(labState, entity, ShieldRaisedEffectId))
	{
		if(abilityId == SwordStabAbilityId || abilityId == SwordSwingAbilityId)
		{
			enabled = false;
		}
	}
	else if(HasEffect(labState, entity, BlindEffectId))
	{
		enabled = false;
	}

	return enabled;
}

static Bool32
func CanUseAbility(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Bool32 canUseAbility = true;
	Int32 minLevel = GetAbilityMinLevel(abilityId);
	if(entity->level < minLevel)
	{
		canUseAbility = false;
	}
	else if(entity->recharge > 0.0f || AbilityIsOnCooldown(labState, entity, abilityId))
	{
		canUseAbility = false;
	}
	else if(entity->castedAbility != NoAbilityId)
	{
		canUseAbility = false;
	}
	else if(!AbilityIsEnabled(labState, entity, abilityId))
	{
		canUseAbility = false;
	}
	return canUseAbility;
}

static void
func PutOnRecharge(Entity* entity, Real32 rechargeTime)
{
	Assert(rechargeTime > 0.0f);
	Assert(entity->recharge == 0.0f);
	entity->rechargeFrom = rechargeTime;
	entity->recharge = rechargeTime;
}

static Bool32
func CanTakeDamage(CombatLabState* labState, Entity* entity)
{
	Bool32 canDamage = false;
	if(IsDead(entity))
	{
		canDamage = false;
	}
	else if(HasEffect(labState, entity, InvulnerableEffectId))
	{
		canDamage = false;
	}
	else
	{
		canDamage = true;
	}
	return canDamage;
}

static void
func AddDamageDisplay(CombatLabState* labState, Vec2 position, Int32 damage)
{
	Assert(labState->damageDisplayN < MaxDamageDisplayN);
	DamageDisplay* display = &labState->damageDisplays[labState->damageDisplayN];
	labState->damageDisplayN++;
	display->position = position;
	display->damage = damage;
	display->timeRemaining = DamageDisplayDuration;
}

static HateTableEntry*
func GetHateTableEntry(HateTable* hateTable, Entity* source, Entity* target)
{
	HateTableEntry* result = 0;
	for(Int32 i = 0; i < hateTable->entryN; i++)
	{
		HateTableEntry* entry = &hateTable->entries[i];
		if(entry->source == source && entry->target == target)
		{
			result = entry;
			break;
		}
	}
	return result;
}

static HateTableEntry*
func AddEmptyHateTableEntry(HateTable* hateTable, Entity* source, Entity* target)
{
	Assert(GetHateTableEntry(hateTable, source, target) == 0);
	Assert(hateTable->entryN < MaxHateTableEntryN);
	HateTableEntry* entry = &hateTable->entries[hateTable->entryN];
	hateTable->entryN++;

	entry->source = source;
	entry->target = target;
	entry->value = 0;
	return entry;
}

static void
func GenerateHate(HateTable* hateTable, Entity* source, Entity* target, Int32 value)
{
	Assert(value >= 0);
	HateTableEntry* entry = GetHateTableEntry(hateTable, source, target);
	if(entry == 0)
	{
		entry = AddEmptyHateTableEntry(hateTable, source, target);
	}

	Assert(entry != 0);
	entry->value += value;
}

static void
func DropItem(CombatLabState* labState, Entity* entity, Int32 itemId)
{
	Assert(itemId != NoItemId);

	DroppedItem item = {};
	item.itemId = itemId;
	item.position = entity->position;
	item.timeLeft = DroppedItemTotalDuration;

	Assert(labState->droppedItemN + 1 < MaxDroppedItemN);
	labState->droppedItems[labState->droppedItemN] = item;
	labState->droppedItemN++;

	Int8* itemName = GetItemName(itemId);
	CombatLog(labState, entity->name + " drops " + itemName + ".");
}

static void
func DropLoot(CombatLabState* labState, Entity* entity)
{
	if(entity->classId == SnakeClassId)
	{
		DropItem(labState, entity, AntiVenomItemId);
	}
}

static void
func DealFinalDamage(Entity* entity, Int32 damage)
{
	Assert(damage > 0);

	if(entity->absorbDamage >= damage)
	{
		entity->absorbDamage -= damage;
	}
	else
	{
		damage -= entity->absorbDamage;
		entity->absorbDamage = 0;
		entity->health = IntMax2(entity->health - damage, 0);
	}
}

static void
func LevelUp(Entity* entity)
{
	Assert(IsIntBetween(entity->level, 1, MaxLevel - 1));
	entity->level++;
	entity->intellect++;
	entity->strength++;
	entity->dexterity++;
	entity->constitution++;
}

static Int32
func GetFinalDamage(CombatLabState* labState, Entity* source, Entity* target, Int32 damage)
{
	Int32 finalDamage = damage;
	Assert(CanTakeDamage(labState, target));

	if(source)
	{
		if(HasEffect(labState, source, ReducedDamageDoneAndTakenEffectId))
		{
			finalDamage -= finalDamage / 5;
		}
	}

	if(HasEffect(labState, target, ReducedDamageDoneAndTakenEffectId))
	{
		finalDamage -= finalDamage / 5;
	}
	if(HasEffect(labState, target, ShieldRaisedEffectId))
	{
		finalDamage -= finalDamage / 2;
	}
	if(HasEffect(labState, target, EarthShieldEffectId))
	{
		finalDamage -= finalDamage / 10;
	}
	return finalDamage;
}

static void
func DealDamageFromEntity(CombatLabState* labState, Entity* source, Entity* target, Int32 damage)
{
	Assert(source->groupId != target->groupId);
	if(CanTakeDamage(labState, target))
	{
		Int32 finalDamage = GetFinalDamage(labState, source, target, damage);

		DealFinalDamage(target, finalDamage);
		AddDamageDisplay(labState, target->position, finalDamage);

		GenerateHate(&labState->hateTable, target, source, damage);

		CombatLog(labState, source->name + " deals " + damage + " damage to " + target->name + ".");
		if(IsDead(target))
		{
			DropLoot(labState, target);
			CombatLog(labState, target->name + " dies.");

			if(target->groupId == EnemyGroupId)
			{
				Entity* player = &labState->entities[0];
				if(player->level < MaxLevel)
				{
					LevelUp(player);
				}
			}
		}
	}
}

static void
func DealDamageFromEffect(CombatLabState* labState, Int32 effectId, Entity* target, Int32 damage)
{
	if(CanTakeDamage(labState, target))
	{
		Int32 finalDamage = GetFinalDamage(labState, 0, target, damage);
		DealFinalDamage(target, finalDamage);
		AddDamageDisplay(labState, target->position, finalDamage);

		Int8* effectName = GetEffectName(effectId);
		CombatLog(labState, effectName + " deals " + finalDamage + " damage to " + target->name + ".");
		if(IsDead(target))
		{
			DropLoot(labState, target);
			CombatLog(labState, target->name + " dies.");

			if(target->groupId == EnemyGroupId)
			{
				Entity* player = &labState->entities[0];
				if(player->level < MaxLevel)
				{
					LevelUp(player);
				}
			}
		}
	}
}

static Bool32
func CanBeHealed(Entity* entity)
{
	Bool32 canBeHealed = !IsDead(entity);
	return canBeHealed;
}

static void
func Heal(CombatLabState* labState, Entity* source, Entity* target, Int32 healing)
{
	Assert(CanBeHealed(target));
	Assert(!IsDead(source));
	Assert(source->groupId == target->groupId);
	Int32 maxHealth = GetEntityMaxHealth(target);
	target->health = IntMin2(target->health + healing, maxHealth);
	AddDamageDisplay(labState, target->position, -healing);

	if(source == target)
	{
		CombatLog(labState, target->name + " heals for " + healing + ".");
	}
	else
	{
		CombatLog(labState, source->name + " heals " + target->name + " for " + healing + ".");
	}
}

static void
func AttemptToHeal(CombatLabState* labState, Entity* source, Entity* target, Int32 healing)
{
	if(CanBeHealed(target))
	{
		Heal(labState, source, target, healing);
	}
}

static void
func AddAbilityCooldown(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Assert(AbilityHasCooldown(abilityId));
	Real32 duration = GetAbilityCooldownDuration(abilityId);

	Assert(duration > 0.0f);
	Assert(labState->abilityCooldownN < MaxAbilityCooldownN);
	AbilityCooldown* cooldown = &labState->abilityCooldowns[labState->abilityCooldownN];
	labState->abilityCooldownN++;

	cooldown->entity = entity;
	cooldown->abilityId = abilityId;
	cooldown->timeRemaining = duration;
}

static AbilityCooldown*
func GetAbilityCooldown(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	AbilityCooldown* result = 0;
	for(Int32 i = 0; i < labState->abilityCooldownN; i++)
	{
		AbilityCooldown* cooldown = &labState->abilityCooldowns[i];
		if(cooldown->entity == entity && cooldown->abilityId == abilityId)
		{
			result = cooldown;
			break;
		}
	}
	return result;
}

static Vec2
func GetClosestMoveDirection(Vec2 distance)
{
	Vec2 left  = MakeVector(-1.0f, 0.0f);
	Vec2 right = MakeVector(+1.0f, 0.0f);
	Vec2 up    = MakeVector(0.0f, -1.0f);
	Vec2 down  = MakeVector(0.0f, +1.0f);
	Vec2 direction = {};

	if(distance.x > 0.0f)
	{
		if(distance.y < -2.0f * distance.x)
		{
			direction = up;
		}
		else if(distance.y < -0.5f * distance.x)
		{
			direction = up + right;
		}
		else if(distance.y < 0.5f * distance.x)
		{
			direction = right;
		}
		else if(distance.y < 2.0f * distance.x)
		{
			direction = down + right;
		}
		else
		{
			direction = down;
		}
	}
	else
	{
		if(distance.y < 2.0f * distance.x)
		{
			direction = up;
		}
		else if(distance.y < 0.5f * distance.x)
		{
			direction = up + left;
		}
		else if(distance.y < -0.5f * distance.x)
		{
			direction = left;
		}
		else if(distance.y < -2.0f * distance.x)
		{
			direction = down + left;
		}
		else
		{
			direction = down;
		}
	}
	return direction;
}

static void
func RecalculatePlayerAttributes(CombatLabState* labState)
{
	Entity* player = &labState->entities[0];
	Assert(IsIntBetween(player->level, 1, MaxLevel));

	player->constitution = player->level;
	player->strength     = player->level;
	player->intellect    = player->level;
	player->dexterity    = player->level;

	Inventory* equipInventory = &labState->equipInventory;
	for(Int32 row = 0; row < equipInventory->rowN; row++)
	{
		for(Int32 col = 0; col < equipInventory->colN; col++)
		{
			Int32 itemId = GetInventoryItemId(equipInventory, row, col);
			if(itemId != NoItemId)
			{
				ItemAttributes attributes = GetItemAttributes(itemId);
				player->constitution += attributes.constitution;
				player->strength     += attributes.strength;
				player->intellect    += attributes.intellect;
				player->dexterity    += attributes.dexterity;
			}
		}
	}

	for(Int32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		if(effect->entity == player)
		{
			switch(effect->effectId)
			{
				case IntellectPotionEffectId:
				{
					player->intellect += 10;
					break;
				}
				case FeelingSmartEffectId:
				{
					player->intellect += 10;
					player->strength -= 10;
					player->strength = IntMax2(player->strength, 0);
					break;
				}
				case FeelingStrongEffectId:
				{
					player->strength += 10;
					player->intellect -= 10;
					player->intellect = IntMax2(player->intellect, 0);
					break;
				}
				case FeelingQuickEffectId:
				{
					player->constitution += 10;
					player->dexterity += 10;
					break;
				}
			}
		}
	}
}

static void
func RemoveEffect(CombatLabState* labState, Entity* entity, Int32 effectId)
{
	Int32 remainingEffectN = 0;
	for(Int32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		if(effect->entity != entity || effect->effectId != effectId)
		{
			labState->effects[remainingEffectN] = *effect;
			remainingEffectN++;
		}
	}
	labState->effectN = remainingEffectN;

	Entity* player = &labState->entities[0];
	if(entity == player)
	{
		RecalculatePlayerAttributes(labState);
	}
}

static Bool32
func CanAddEffect(CombatLabState* labState, Entity* entity, Int32 effectId)
{
	Bool32 canAddEffect = true;
	switch(effectId)
	{
		case PoisonedEffectId:
		{
			canAddEffect = !HasEffect(labState, entity, ImmuneToPoisonEffectId);
			break;
		}
		default:
		{
			canAddEffect = true;
		}
	}

	return canAddEffect;
}

static void
func AddEffect(CombatLabState* labState, Entity* entity, Int32 effectId)
{
	Assert(CanAddEffect(labState, entity, effectId));
	Assert(labState->effectN < MaxEffectN);
	Effect* effect = &labState->effects[labState->effectN];
	labState->effectN++;

	effect->entity = entity;
	effect->effectId = effectId;
	
	if(EffectHasDuration(effectId))
	{
		Real32 duration = GetEffectDuration(effectId);
		Assert(duration > 0.0f);
		effect->timeRemaining = duration;
	}

	Entity* player = &labState->entities[0];
	if(entity == player)
	{
		RecalculatePlayerAttributes(labState);
	}

	Int8* effectName = GetEffectName(effectId);
	CombatLog(labState, entity->name + " gets " + effectName + ".");
}

static void
func ResetOrAddEffect(CombatLabState* labState, Entity* entity, Int32 effectId)
{
	Bool32 foundEffect = false;
	for(Int32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		if(effect->entity == entity && effect->effectId == effectId)
		{

			if(EffectHasDuration(effectId))
			{
				Real32 duration = GetEffectDuration(effectId);
				Assert(duration > 0.0f);
				effect->timeRemaining = duration;
			}
			foundEffect = true;
			break;
		}
	}

	if(!foundEffect)
	{
		if(CanAddEffect(labState, entity, effectId))
		{
			AddEffect(labState, entity, effectId);
		}
	}
}

static Int32
func GetAbilityDamage(Entity* entity, Int32 abilityId)
{
	Int32 damage = 0;
	switch(abilityId)
	{
		case LightningAbilityId:
		{
			damage = 20 + entity->intellect;
			break;
		}
		case SmallPunchAbilityId:
		{
			damage = 10 + entity->strength;
			break;
		}
		case BigPunchAbilityId:
		{
			damage = 30 + entity->strength;
			break;
		}
		case SpinningKickAbilityId:
		{
			damage = 5 + entity->strength;
			break;
		}
		case SwordStabAbilityId:
		{
			damage = 15 + entity->strength;
			break;
		}
		case SwordSwingAbilityId:
		{
			damage = 10 + entity->strength;
			break;
		}
		case SnakeStrikeAbilityId:
		{
			damage = 10 + entity->strength;
			break;
		}
		case CrocodileBiteAbilityId:
		{
			damage = 15 + entity->strength;
			break;
		}
		case CrocodileLashAbilityId:
		{
			damage = 30 + entity->strength;
			break;
		}
	}
	return damage;
}

static void
func UseInstantAbility(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Assert(!AbilityIsCasted(abilityId));
	Assert(CanUseAbility(labState, entity, abilityId));

	Int8* abilityName = GetAbilityName(abilityId);
	CombatLog(labState, entity->name + " uses ability " + abilityName + ".");

	Entity* target = entity->target;
	Bool32 hasEnemyTarget = (target != 0 && target->groupId != entity->groupId);
	Bool32 hasFriendlyTarget = (target != 0 && target->groupId == entity->groupId);
	Int32 damage = GetAbilityDamage(entity, abilityId);
	switch(abilityId)
	{
		case EarthShakeAbilityId:
		{
			Assert(hasEnemyTarget);
			ResetOrAddEffect(labState, target, EarthShakeEffectId);
			break;
		}
		case SmallPunchAbilityId:
		{
			Assert(hasEnemyTarget);
			DealDamageFromEntity(labState, entity, target, damage);
			break;
		}
		case EarthShieldAbilityId:
		{
			Assert(hasFriendlyTarget && !IsDead(target));
			ResetOrAddEffect(labState, target, EarthShieldEffectId);
			target->absorbDamage += 50;
			break;
		}
		case BigPunchAbilityId:
		{
			Assert(hasEnemyTarget);
			DealDamageFromEntity(labState, entity, target, damage);
			break;
		}
		case KickAbilityId:
		{
			Assert(hasEnemyTarget);
			Real32 moveSpeed = 20.0f;
			target->velocity = moveSpeed * GetClosestMoveDirection(target->position - entity->position);
			AddEffect(labState, target, KickedEffectId);
			break;
		}
		case SpinningKickAbilityId:
		{
			for(Int32 i = 0; i < EntityN; i++)
			{
				Entity* target = &labState->entities[i];
				if(target->groupId != entity->groupId)
				{
					Real32 distance = Distance(entity->position, target->position);
					if(distance <= MaxMeleeAttackDistance)
					{
						DealDamageFromEntity(labState, entity, target, damage);
					}
				}
			}
			break;
		}
		case RollAbilityId:
		{
			Real32 moveSpeed = 20.0f;
			entity->velocity = moveSpeed * GetClosestMoveDirection(entity->inputDirection);
			AddEffect(labState, entity, RollingEffectId);
			break;
		}
		case AvoidanceAbilityId:
		{
			AddEffect(labState, entity, InvulnerableEffectId);
			break;
		}
		case SwordStabAbilityId:
		{
			if(HasEffect(labState, entity, BlessingOfTheSunEffectId))
			{
				damage += 2;
				Heal(labState, entity, entity, 2);
			}

			Assert(hasEnemyTarget);
			DealDamageFromEntity(labState, entity, target, damage);
			break;
		}
		case SwordSwingAbilityId:
		{
			if(HasEffect(labState, entity, BlessingOfTheSunEffectId))
			{
				damage += 2;
				Heal(labState, entity, entity, 2);
			}

			for(Int32 i = 0; i < EntityN; i++)
			{
				Entity* target = &labState->entities[i];
				if(target->groupId != entity->groupId)
				{
					Real32 distance = Distance(entity->position, target->position);
					if(distance <= MaxMeleeAttackDistance)
					{
						DealDamageFromEntity(labState, entity, target, damage);
					}
				}
			}
			break;
		}
		case RaiseShieldAbilityId:
		{
			AddEffect(labState, entity, ShieldRaisedEffectId);
			if(HasEffect(labState, entity, BlessingOfTheSunEffectId))
			{
				Heal(labState, entity, entity, 10);
			}
			break;
		}
		case BurnAbilityId:
		{
			Assert(hasEnemyTarget);
			AddEffect(labState, target, BurningEffectId);
			break;
		}
		case BlessingOfTheSunAbilityId:
		{
			RemoveEffect(labState, entity, BlessingOfTheSunEffectId);
			AddEffect(labState, entity, BlessingOfTheSunEffectId);
			break;
		}
		case MercyOfTheSunAbilityId:
		{
			Heal(labState, entity, entity, 30);
			for(Int32 i = 0; i < EntityN; i++)
			{
				Entity* target = &labState->entities[i];
				if(!IsDead(target) && target->groupId != entity->groupId)
				{
					Real32 distance = Distance(entity->position, target->position);
					if(distance <= MaxMeleeAttackDistance)
					{
						AddEffect(labState, target, BlindEffectId);
					}
				}
			}
			break;
		}
		case SnakeStrikeAbilityId:
		{
			Assert(hasEnemyTarget);
			DealDamageFromEntity(labState, entity, target, damage);
			if(RandomBetween(0.0f, 1.0f) < 0.3f)
			{
				ResetOrAddEffect(labState, target, PoisonedEffectId);
			}
			break;
		}
		case CrocodileBiteAbilityId:
		{
			Assert(hasEnemyTarget);
			DealDamageFromEntity(labState, entity, target, damage);
			ResetOrAddEffect(labState, target, BittenEffectId);
			break;
		}
		case CrocodileLashAbilityId:
		{
			Assert(hasEnemyTarget);
			DealDamageFromEntity(labState, entity, target, damage);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	if(AbilityHasCooldown(abilityId))
	{
		AddAbilityCooldown(labState, entity, abilityId);
	}
}

static void
func FinishCasting(CombatLabState* labState, Entity* entity)
{
	Int32 abilityId = entity->castedAbility;
	Assert(abilityId != NoAbilityId);
	Assert(AbilityIsCasted(abilityId));
	Entity* target = entity->castTarget;
	Bool32 hasEnemyTarget = (target != 0 && target->groupId != entity->groupId);
	Bool32 hasFriendlyTarget = (target != 0 && target->groupId == entity->groupId);
	Int32 damage = GetAbilityDamage(entity, abilityId);
	switch(abilityId)
	{
		case LightningAbilityId:
		{
			Assert(hasEnemyTarget);
			DealDamageFromEntity(labState, entity, target, damage);
			break;
		}
		case HealAbilityId:
		{
			Assert(hasFriendlyTarget);
			AttemptToHeal(labState, entity, target, 30);
			break;
		}
		case LightOfTheSunAbilityId:
		{
			AttemptToHeal(labState, entity, entity, 20);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	if(AbilityHasCooldown(abilityId))
	{
		AddAbilityCooldown(labState, entity, abilityId);
	}

	entity->castedAbility = NoAbilityId;
	entity->castTarget = 0;
	entity->castTimeTotal = 0.0f;
	entity->castTimeRemaining = 0.0f;
}

static void
func UseAbility(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	if(AbilityIsCasted(abilityId))
	{
		Assert(CanUseAbility(labState, entity, abilityId));
		Real32 castDuration = GetAbilityCastDuration(abilityId);
		Assert(castDuration > 0.0f);
		entity->castedAbility = abilityId;
		entity->castTimeTotal = castDuration;
		entity->castTimeRemaining = castDuration;
		entity->castTarget = entity->target;
	}
	else
	{
		UseInstantAbility(labState, entity, abilityId);
	}

	Real32 rechargeDuration = GetAbilityRechargeDuration(abilityId);
	if(rechargeDuration > 0.0f)
	{
		PutOnRecharge(entity, rechargeDuration);
	}
}

static void
func AttemptToUseAbility(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	if(CanUseAbility(labState, entity, abilityId))
	{
		UseAbility(labState, entity, abilityId);
	}
}

static Int32
func GetAvailableClassAbilityAtIndex(Entity* entity, Int32 abilityIndex)
{
	Int32 result = NoAbilityId;
	Int32 classAbilityIndex = 0;
	for(Int32 abilityId = 1; abilityId < AbilityN; abilityId++)
	{
		Int32 abilityLevel = GetAbilityMinLevel(abilityId);
		Int32 abilityClassId = GetAbilityClass(abilityId);
		if(entity->level >= abilityLevel && entity->classId == abilityClassId)
		{
			if(classAbilityIndex == abilityIndex)
			{
				result = abilityId;
				break;
			}
			classAbilityIndex++;
		}
	}
	return result;
}

static void
func AttemptToUseAbilityAtIndex(CombatLabState* labState, Entity* entity, Int32 abilityIndex)
{
	Int32 abilityId = GetAvailableClassAbilityAtIndex(entity, abilityIndex);
	if(abilityId != NoAbilityId)
	{
		AttemptToUseAbility(labState, entity, abilityId);
	}
}

static void
func DrawCircle(Canvas* canvas, Vec2 center, Real32 radius, Vec4 color)
{
	UInt32 colorCode = GetColorCode(color);

	Camera *camera = canvas->camera;

	Int32 centerXPixel = UnitXtoPixel(camera, center.x);
	Int32 centerYPixel = UnitYtoPixel(camera, center.y);

	Int32 leftPixel   = UnitXtoPixel(camera, center.x - radius);
	Int32 rightPixel  = UnitXtoPixel(camera, center.x + radius);
	Int32 topPixel    = UnitYtoPixel(camera, center.y - radius);
	Int32 bottomPixel = UnitYtoPixel(camera, center.y + radius);

	if(topPixel > bottomPixel)
	{
		IntSwap(&topPixel, &bottomPixel);
	}

	if(leftPixel > rightPixel)
	{
		IntSwap(&leftPixel, &rightPixel);
	}

	Int32 pixelRadius = (Int32)(radius * camera->unitInPixels);
	Int32 pixelRadiusSquare = pixelRadius * pixelRadius;

	Bitmap bitmap = canvas->bitmap;
	topPixel    = IntMax2(topPixel, 0);
	bottomPixel = IntMin2(bottomPixel, bitmap.height - 1);
	leftPixel   = IntMax2(leftPixel, 0);
	rightPixel  = IntMin2(rightPixel, bitmap.width - 1);

	for(Int32 row = topPixel; row < bottomPixel; row++)
	{
		for(Int32 col = leftPixel; col < rightPixel; col++)
		{
			UInt32* pixel = bitmap.memory + row * bitmap.width + col;
			Int32 pixelDistanceSquare = IntSquare(row - centerYPixel) + IntSquare(col - centerXPixel);
			if(pixelDistanceSquare <= pixelRadiusSquare)
			{
				*pixel = colorCode;
			}
		}
	}
}

static void
func DrawCircleOutline(Canvas* canvas, Vec2 center, Real32 radius, Vec4 color)
{
	Int32 lineN = 20;
	Real32 angleAdvance = (2.0f * PI) / Real32(lineN);
	Real32 angle = 0.0f;
	for(Int32 i = 0; i < lineN; i++)
	{
		Real32 angle1 = angle;
		Real32 angle2 = angle + angleAdvance;
		Vec2 point1 = center + radius * RotationVector(angle1);
		Vec2 point2 = center + radius * RotationVector(angle2);
		Bresenham(canvas, point1, point2, color);

		angle += angleAdvance;
	}
}

static void
func DrawEntity(Canvas* canvas, Entity* entity, Vec4 color)
{
	Vec4 entityOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawCircle(canvas, entity->position, EntityRadius, color);
	DrawCircleOutline(canvas, entity->position, EntityRadius, entityOutlineColor);
}

static void
func DrawSelectedEntity(Canvas* canvas, Entity* entity, Vec4 color)
{
	Vec4 selectionOutlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawCircle(canvas, entity->position, EntityRadius, color);
	DrawCircleOutline(canvas, entity->position, EntityRadius, selectionOutlineColor);	
}

static void
func DrawEntityBars(Canvas* canvas, Entity* entity)
{
	Vec2 healthBarCenter = entity->position + MakeVector(0.0f, -1.5f * EntityRadius);
	Real32 healthBarWidth = 2.0f * EntityRadius;
	Real32 healthBarHeight = 0.5f * EntityRadius;

	Vec4 healthBarBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
	Vec4 healthBarFilledColor = MakeColor(1.0f, 0.0f, 0.0f);
	Vec4 healthBarOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);

	Real32 maxHealth = (Real32)GetEntityMaxHealth(entity);
	Assert(maxHealth > 0.0f);
	Real32 healthRatio = Real32(entity->health) / maxHealth;
	Assert(IsBetween(healthRatio, 0.0f, 1.0f));

	Rect healthBarBackgroundRect = MakeRect(healthBarCenter, healthBarWidth, healthBarHeight);
	DrawRect(canvas, healthBarBackgroundRect, healthBarBackgroundColor);

	Rect healthBarFilledRect = healthBarBackgroundRect;
	healthBarFilledRect.right = Lerp(healthBarBackgroundRect.left, healthRatio, healthBarBackgroundRect.right);
	DrawRect(canvas, healthBarFilledRect, healthBarFilledColor);

	if(entity->absorbDamage > 0)
	{
		Vec4 absorbDamageBackgroundColor = MakeColor(1.0f, 1.0f, 1.0f);
		Rect absorbDamageRect = healthBarBackgroundRect;
		absorbDamageRect.left = healthBarFilledRect.right;
		Real32 maxHealth = (Real32)GetEntityMaxHealth(entity);
		Assert(maxHealth > 0.0f);
		Real32 absorbDamageWidth = Real32(entity->absorbDamage) / maxHealth * healthBarWidth;
		absorbDamageRect.right = Min2(healthBarBackgroundRect.right, absorbDamageRect.left + absorbDamageWidth);
		DrawRect(canvas, absorbDamageRect, absorbDamageBackgroundColor);
	}

	DrawRectOutline(canvas, healthBarBackgroundRect, healthBarOutlineColor);

	if(entity->castedAbility != NoAbilityId)
	{
		Real32 castBarWidth = healthBarWidth;
		Real32 castBarHeight = 0.3f * EntityRadius;
		Vec2 castBarCenter = healthBarCenter + MakeVector(0.0f, -healthBarHeight * 0.5f - castBarHeight * 0.5f);

		Vec4 castBarBackgroundColor = MakeColor(0.0, 0.5f, 0.5f);
		Vec4 castBarFilledColor = MakeColor(0.0f, 1.0f, 1.0f);
		Vec4 castBarOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);

		Rect castBarBackgroundRect = MakeRect(castBarCenter, castBarWidth, castBarHeight);
		DrawRect(canvas, castBarBackgroundRect, castBarBackgroundColor);

		Assert(entity->castTimeTotal > 0.0f);
		Real32 castRatio = 1.0f - (entity->castTimeRemaining / entity->castTimeTotal);
		Assert(IsBetween(castRatio, 0.0f, 1.0f));

		Rect castBarFilledRect = castBarBackgroundRect;
		castBarFilledRect.right = Lerp(castBarBackgroundRect.left, castRatio, castBarBackgroundRect.right);
		DrawRect(canvas, castBarFilledRect, castBarFilledColor);

		DrawRectOutline(canvas, castBarBackgroundRect, castBarOutlineColor);
	}
}

static String
func GetAbilityTooltipText(Int32 abilityId, Entity* entity, Int8* buffer, Int32 bufferSize)
{
	String text = StartString(buffer, bufferSize);

	Int8* abilityName = GetAbilityName(abilityId);
	AddLine(text, abilityName);

	Real32 castDuration = GetAbilityCastDuration(abilityId);
	if(castDuration > 0.0f)
	{
		if(castDuration == 1.0f)
		{
			AddLine(text, "Cast: 1 second");
		}
		else
		{
			AddLine(text, "Cast: " + castDuration + " seconds");
		}
	}
	else
	{
		AddLine(text, "Instant");
	}

	Real32 rechargeDuration = GetAbilityRechargeDuration(abilityId);
	if(rechargeDuration == 1.0f)
	{
		AddLine(text, "Recharge: 1 second");
	}
	else if(rechargeDuration > 0.0f)
	{
		AddLine(text, "Recharge: " + rechargeDuration + " seconds");
	}

	if(AbilityHasCooldown(abilityId))
	{
		Real32 cooldownDuration = GetAbilityCooldownDuration(abilityId);
		Assert(cooldownDuration > 0.0f);
		if(cooldownDuration == 1.0f)
		{
			AddLine(text, "Cooldown: 1 second");
		}
		else
		{
			AddLine(text, "Cooldown: " + cooldownDuration + " seconds");
		}
	}

	Int32 damage = GetAbilityDamage(entity, abilityId);

	switch(abilityId)
	{
		case LightningAbilityId:
		{
			AddLine(text, "Ranged");
			AddLine(text, "Strike enemy target with lightning,");
			AddLine(text, "dealing " + damage + " damage.");
			break;
		}
		case EarthShakeAbilityId:
		{
			AddLine(text, "Ranged");
			AddLine(text, "Make the earth shake under target enemy,");
			AddLine(text, "reducing their movement speed by 50%");
			AddLine(text, "while on the ground for 6 seconds.");
			break;
		}
		case HealAbilityId:
		{
			AddLine(text, "Heal friendly target for " + damage + ". You must be");
			AddLine(text, "near a source of water.");
			break;
		}
		case EarthShieldAbilityId:
		{
			AddLine(text, "Apply an earth shield to friendly target,");
			AddLine(text, "reducing their damage taken by 10% and");
			AddLine(text, "absorbing 50 damage for 5 seconds.");
			break;
		}
		case SmallPunchAbilityId:
		{
			AddLine(text, "Melee range")
			AddLine(text, "Punch target enemy lightly, dealing");
			AddLine(text, damage + " damage.");
			break;
		}
		case BigPunchAbilityId:
		{
			AddLine(text, "Melee range");
			AddLine(text, "Forcefully punch target enemy, dealing");
			AddLine(text, damage + " damage.");
			break;
		}
		case KickAbilityId:
		{
			AddLine(text, "Melee range");
			AddLine(text, "Kick target enemy, knocking them away.");
			break;
		}
		case SpinningKickAbilityId:
		{
			AddLine(text, "Deal " + damage + " damage to all enemies within");
			AddLine(text, "melee range.");
			break;
		}
		case RollAbilityId:
		{
			AddLine(text, "Roll away in a direction.");
			break;
		}
		case AvoidanceAbilityId:
		{
			AddLine(text, "Become invulnerable for 2 seconds.");
			break;
		}
		case SwordStabAbilityId:
		{
			AddLine(text, "Melee range");
			AddLine(text, "Stab target enemy with your sword,");
			AddLine(text, "dealing " + damage + " damage.");
			break;
		}
		case SwordSwingAbilityId:
		{
			AddLine(text, "Deal " + damage + " damage to all enemies in front");
			AddLine(text, "of you (melee range).");
			break;
		}
		case RaiseShieldAbilityId:
		{
			AddLine(text, "Raise up your shield, reducing the");
			AddLine(text, "effectiveness of incoming attacks,");
			AddLine(text, "decreasing the damage you take from");
			AddLine(text, "melee attacks by 50%.");
			AddLine(text, "You cannot use melee attacks while");
			AddLine(text, "your shield is raised.");
			AddLine(text, "Lasts for 3 seconds.");
			break;
		}
		case BurnAbilityId:
		{
			AddLine(text, "Burn target enemy with the power of");
			AddLine(text, "the sun, dealing 2 damage every second");
			AddLine(text, "for 8 seconds.");
			break;
		}
		case LightOfTheSunAbilityId:
		{
			AddLine(text, "Use the light of the Sun to heal yourself");
			AddLine(text, "for 20 damage.");
			break;
		}
		case BlessingOfTheSunAbilityId:
		{
			AddLine(text, "Heat up your weapon and shield with the");
			AddLine(text, "light of the Sun, making your melee attacks");
			AddLine(text, "deal 2 additional damage and heal you");
			AddLine(text, "for 2 damage.");
			AddLine(text, "Raising your shield burns enemies within");
			AddLine(text, "melee range for 10 damage and heals you");
			AddLine(text, "for 10 damage.");
			AddLine(text, "Lasts for 5 minutes.");
			break;
		}
		case MercyOfTheSunAbilityId:
		{
			AddLine(text, "Plead for the mercy of the Sun, healing");
			AddLine(text, "you for 30 and blinding all");
			AddLine(text, "enemies within your melee range");
			AddLine(text, "for 5 seconds, making them unable to");
			AddLine(text, "use abilities while it lasts.");
			break;
		}
		case SnakeStrikeAbilityId:
		{
			AddLine(text, "Strike your target enemy dealing " + damage + " damage,");
			AddLine(text, "having a 30% chance to poison them.");
			break;
		}
		case CrocodileBiteAbilityId:
		{
			AddLine(text, "Bite target enemy, dealing " + damage + " damage and");
			AddLine(text, "slowing their movement speed by 50% for");
			AddLine(text, "5 seconds.");
			break;
		}
		case CrocodileLashAbilityId:
		{
			AddLine(text, "Attack target enemy, dealing " + damage + " damage.");
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return text;
}

static Vec4
func GetFlowerColor(Int32 itemId)
{
	Vec4 color = {};
	switch(itemId)
	{
		case BlueFlowerItemId:
		case BlueFlowerOfIntellectItemId:
		case BlueFlowerOfHealingItemId:
		case BlueFlowerOfDampeningItemId:
		{
			color = MakeColor(0.1f, 0.1f, 1.0f);
			break;
		}
		case RedFlowerItemId:
		case RedFlowerOfStrengthItemId:
		case RedFlowerOfHealthItemId:
		case RedFlowerOfPoisonItemId:
		{
			color = MakeColor(1.0f, 0.1f, 0.1f);
			break;
		}
		case YellowFlowerItemId:
		case YellowFlowerOfDexterityItemId:
		case YellowFlowerOfAntivenomItemId:
		case YellowFlowerOfRageItemId:
		{
			color = MakeColor(1.0f, 1.0f, 0.1f);
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
func UpdateAndDrawFlowers(Canvas* canvas, CombatLabState* labState, Vec2 mousePosition)
{
	GlyphData* glyphData = canvas->glyphData;

	Real32 radius = 0.5f;

	Vec4 textColor = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 textBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
	Vec4 textHoverBackgroundColor = MakeColor(0.7f, 0.5f, 0.5f);

	labState->hoverFlower = 0;

	for(Int32 i = 0; i < labState->flowerN; i++)
	{
		Flower* flower = &labState->flowers[i];
		Vec4 color = GetFlowerColor(flower->itemId);
		DrawCircle(canvas, flower->position, radius, color);

		Int8* text = GetItemName(flower->itemId);

		Real32 textWidth = GetTextWidth(canvas, text);
		Real32 textHeight = GetTextHeight(canvas, text);

		Real32 textBottom = flower->position.y - radius;
		Real32 textCenterX = flower->position.x;
		
		Vec2 textBottomCenter = MakePoint(textCenterX, textBottom);
		Rect textBackgroundRect = MakeRectBottom(textBottomCenter, textWidth, textHeight);

		Bool32 isHover = IsPointInRect(mousePosition, textBackgroundRect);
		Vec4 backgroundColor = (isHover) ? textHoverBackgroundColor : textBackgroundColor;

		if(isHover)
		{
			labState->hoverFlower = flower;
		}

		DrawRect(canvas, textBackgroundRect, backgroundColor);
		DrawTextLineBottomXCentered(canvas, text, textBottom, textCenterX, textColor);
	}
}

#define UIBoxSide 40
#define UIBoxPadding 5
static void
func DrawUIBox(Bitmap* bitmap, Int32 left, Int32 right, Int32 top, Int32 bottom, Real32 rechargeRatio, Vec4 color)
{
	Vec4 outlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 rechargeColor = MakeColor(0.5f, 0.5f, 0.5f);

	DrawBitmapRect(bitmap, left, right, top, bottom, color);

	Assert(IsBetween(rechargeRatio, 0.0f, 1.0f));
	Int32 rechargeRight = (Int32)Lerp((Real32)left, rechargeRatio, (Real32)right);
	DrawBitmapRect(bitmap, left, rechargeRight, top, bottom, rechargeColor);

	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);
}

static void
func DrawUIBoxWithText(Bitmap* bitmap, Int32 left, Int32 right, Int32 top, Int32 bottom, Real32 rechargeRatio,
								   Int8* text, GlyphData* glyphData, Vec4 color)
{
	DrawUIBox(bitmap, left, right, top, bottom, rechargeRatio, color);
	Vec4 textColor = MakeColor(1.0f, 1.0f, 1.0f);
	Assert(text != 0 && glyphData != 0);
	DrawBitmapTextLineCentered(bitmap, text, glyphData, left, right, top, bottom, textColor);
}

static void
func DrawEffectUIBox(Bitmap* bitmap, Effect* effect, Int32 top, Int32 left)
{
	Int32 right = left + UIBoxSide;
	Int32 bottom = top + UIBoxSide;

	Real32 durationRatio = 0.0f;
	if(EffectHasDuration(effect->effectId))
	{
		Real32 totalDuration = GetEffectDuration(effect->effectId);
		Assert(totalDuration > 0.0f);

		durationRatio = (effect->timeRemaining / totalDuration);
	}
	else
	{
		durationRatio = 1.0f;
	}

	Assert(IsBetween(durationRatio, 0.0f, 1.0f));

	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawUIBox(bitmap, left, right, top, bottom, durationRatio, backgroundColor);
}

static void
func DrawPlayerEffectBar(Canvas* canvas, CombatLabState* labState)
{
	Bitmap* bitmap = &canvas->bitmap;
	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	Entity* player = &labState->entities[0];
	Assert(player->groupId == PlayerGroupId);

	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);

	Int32 left = UIBoxPadding;
	Int32 top = UIBoxPadding;

	for(Int32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		if(effect->entity == player)
		{
			DrawEffectUIBox(bitmap, effect, top, left);
			left += UIBoxSide + UIBoxPadding;
		}
	}
}

static void
func DrawTargetEffectBar(Canvas* canvas, CombatLabState* labState)
{
	Bitmap* bitmap = &canvas->bitmap;
	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	Entity* player = &labState->entities[0];
	Assert(player->groupId == PlayerGroupId);

	Entity* target = player->target;

	if(target)
	{
		Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);

		Int32 left = (bitmap->width - 1) - UIBoxPadding - UIBoxSide;
		Int32 top = UIBoxPadding;

		for(Int32 i = 0; i < labState->effectN; i++)
		{
			Effect* effect = &labState->effects[i];
			if(effect->entity == target)
			{
				DrawEffectUIBox(bitmap, effect, top, left);
				left -= UIBoxSide + UIBoxPadding;
			}
		}
	}
}

static void
func DrawHelpBar(Canvas* canvas, CombatLabState* labState, IntVec2 mousePosition)
{
	Bitmap* bitmap = &canvas->bitmap;
	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	Int32 right  = (bitmap->width - 1) - UIBoxPadding;
	Int32 left   = right - UIBoxSide;
	Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
	Int32 top    = bottom - UIBoxSide;

	Vec4 boxColor = MakeColor(0.0f, 0.0f, 0.0f);

	Int8 textBuffer[8] = {};
	OneLineString(textBuffer, 8, "Help");

	DrawUIBoxWithText(bitmap, left, right, top, bottom, 0.0f, textBuffer, glyphData, boxColor);

	if(IsIntBetween(mousePosition.col, left, right) && IsIntBetween(mousePosition.row, top, bottom))
	{
		Int8 tooltipBuffer[256] = {};
		String tooltip = StartString(tooltipBuffer, 256);
		AddLine(tooltip, "Key binds");
		AddLine(tooltip, "[W][A][S][D] or Arrows - Move");
		AddLine(tooltip, "[Tab] - Target closest enemy");
		AddLine(tooltip, "[F1] - Target player");
		AddLine(tooltip, "[1]-[2] - Use Ability");
		AddLine(tooltip, "[V] - Show/hide inventory");
		AddLine(tooltip, "[C] - Show/hide character info");

		Int32 tooltipBottom = top - UIBoxPadding;
		Int32 tooltipRight = (bitmap->width - 1);
		DrawBitmapStringTooltipBottomRight(bitmap, tooltip, glyphData, tooltipBottom, tooltipRight);
	}
}

static void
func DrawAbilityBar(Canvas* canvas, CombatLabState* labState, IntVec2 mousePosition)
{
	Bitmap* bitmap = &canvas->bitmap;
	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	Entity* entity = &labState->entities[0];
	Assert(entity->groupId == PlayerGroupId);
	Int32 classId = entity->classId;
	Assert(classId != NoClassId);

	Int32 abilityN = 0;
	for(Int32 abilityId = 1; abilityId < AbilityN; abilityId++)
	{
		Int32 abilityClassId = GetAbilityClass(abilityId);
		Int32 abilityMinLevel = GetAbilityMinLevel(abilityId);
		if(entity->level >= abilityMinLevel && entity->classId == abilityClassId)
		{
			abilityN++;
		}
	}

	Int32 centerX = (bitmap->width - 1) / 2;
	Int32 width = UIBoxSide * abilityN + UIBoxPadding * (abilityN - 1);

	Int32 left   = centerX - width / 2;
	Int32 right  = centerX + width / 2;
	Int32 bottom = (bitmap->height - 1) - 30;
	Int32 top    = bottom - UIBoxSide;

	Int32 boxLeft = left;
	Int32 abilityIndex = 0;

	labState->hoverAbilityId = NoAbilityId;

	for(Int32 abilityId = 1; abilityId < AbilityN; abilityId++)
	{
		Int32 abilityClassId = GetAbilityClass(abilityId);
		Int32 abilityMinLevel = GetAbilityMinLevel(abilityId);
		if(entity->level >= abilityMinLevel && entity->classId == abilityClassId)
		{
			Int32 boxRight = boxLeft + UIBoxSide;
			Int32 boxTop = top;
			Int32 boxBottom = bottom;

			Vec4 boxBackgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
			Vec4 boxCannotUseColor = MakeColor(0.2f, 0.0f, 0.0f);

			Vec4 color = boxBackgroundColor;

			Real32 recharge = 0.0f;
			Real32 rechargeFrom = 0.0f;
			AbilityCooldown* cooldown = GetAbilityCooldown(labState, entity, abilityId);
			if(cooldown)
			{
				recharge = cooldown->timeRemaining;
				rechargeFrom = GetAbilityCooldownDuration(abilityId);
			}
			else if(entity->recharge > 0.0f)
			{
				recharge = entity->recharge;
				rechargeFrom = entity->rechargeFrom;
			}

			if(!AbilityIsEnabled(labState, entity, abilityId))
			{
				color = boxCannotUseColor;
			}

			Real32 rechargeRatio = (rechargeFrom > 0.0f) ? (recharge / rechargeFrom) : 0.0f;

			Int8 name[8] = {};
			OneLineString(name, 8, (abilityIndex + 1));

			DrawUIBoxWithText(bitmap, boxLeft, boxRight, boxTop, boxBottom, rechargeRatio, name, glyphData, color);

			if(IsIntBetween(mousePosition.col, boxLeft, boxRight) && IsIntBetween(mousePosition.row, boxTop, boxBottom))
			{
				Int32 tooltipLeft = (boxLeft + boxRight) / 2 - TooltipWidth / 2;
				Int32 tooltipBottom = boxTop - 5;

				static Int8 tooltipBuffer[512] = {};
				String tooltipText = GetAbilityTooltipText(abilityId, entity, tooltipBuffer, 512);
				DrawBitmapStringTooltipBottom(bitmap, tooltipText, glyphData, tooltipBottom, tooltipLeft);

				labState->hoverAbilityId = abilityId;
			}

			boxLeft = boxRight + UIBoxPadding;
			abilityIndex++;
		}
	}
}

static void
func UpdateEntityRecharge(Entity* entity, Real32 seconds)
{
	entity->recharge = Max2(entity->recharge - seconds, 0.0f);
}

static void
func UpdateAbilityCooldowns(CombatLabState* labState, Real32 seconds)
{
	Int32 remainingN = 0;
	for(Int32 i = 0; i < labState->abilityCooldownN; i++)
	{
		AbilityCooldown* cooldown = &labState->abilityCooldowns[i];
		cooldown->timeRemaining -= seconds;
		if(cooldown->timeRemaining > 0.0f)
		{
			labState->abilityCooldowns[remainingN] = *cooldown;
			remainingN++;
		}
	}
	labState->abilityCooldownN = remainingN;
}

static void
func UpdateItemCooldowns(CombatLabState* labState, Real32 seconds)
{
	Int32 remainingN = 0;
	for(Int32 i = 0; i < labState->itemCooldownN; i++)
	{
		ItemCooldown* cooldown = &labState->itemCooldowns[i];
		cooldown->timeRemaining -= seconds;
		if(cooldown->timeRemaining > 0.0f)
		{
			labState->itemCooldowns[remainingN] = *cooldown;
			remainingN++;
		}
	}
	labState->itemCooldownN = remainingN;
}

static void
func DrawCombatLog(Canvas* canvas, CombatLabState* labState)
{
	Bitmap* bitmap = &canvas->bitmap;
	Int32 width  = 300;
	Int32 height = 200;

	Int32 left   = UIBoxPadding;
	Int32 right  = left + width;
	Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
	Int32 top    = bottom - height;

	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 outlineColor = MakeColor(0.5f, 0.5f, 0.5f);
	Vec4 textColor = MakeColor(1.0f, 1.0f, 1.0f);

	DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);

	Int32 textLeft   = left + UIBoxPadding;
	Int32 textRight  = right - UIBoxPadding;
	Int32 textTop    = top + UIBoxPadding;
	Int32 textBottom = bottom + UIBoxPadding;

	Int32 textHeight = textBottom - textTop;

	Int32 lastIndex = labState->combatLogLastLineIndex;
	Int32 firstIndex = GetNextCombatLogLineIndex(lastIndex);

	Int32 logHeight = labState->combatLogLineN * TextHeightInPixels;
	if(logHeight > textHeight)
	{
		firstIndex = lastIndex - (textHeight / TextHeightInPixels - 1);
		if(firstIndex < 0)
		{
			firstIndex += MaxCombatLogLineN;
		}
		Assert(IsIntBetween(firstIndex, 0, MaxCombatLogLineN + 1));
	}

	Int32 index = firstIndex;
	while(1)
	{
		Int8* text = labState->combatLogLines[index];
		if(text[0] != 0)
		{
			DrawBitmapTextLineTopLeft(bitmap, text, canvas->glyphData, textLeft, textTop, textColor);
			textTop += TextHeightInPixels;
		}

		if(index == lastIndex)
		{
			break;
		}
		index = GetNextCombatLogLineIndex(index);
	}
}

static Int32
GetHateTableEntityEntryN(HateTable* hateTable, Entity* source)
{
	Assert(source != 0);
	Int32 entryN = 0;
	for(Int32 i = 0; i < hateTable->entryN; i++)
	{
		HateTableEntry* entry = &hateTable->entries[i];
		if(entry->source == source)
		{
			entryN++;
		}
	}
	return entryN;
}

static void
func RemoveDeadEntitiesFromHateTable(HateTable* hateTable)
{
	Int32 remainingEntryN = 0;
	for(Int32 i = 0; i < hateTable->entryN; i++)
	{
		HateTableEntry* entry = &hateTable->entries[i];
		if(!IsDead(entry->source) && !IsDead(entry->target))
		{
			hateTable->entries[remainingEntryN] = *entry;
			remainingEntryN++;
		}
	}
	hateTable->entryN = remainingEntryN;
}

static void
func SortHateTable(HateTable* hateTable)
{
	for(Int32 index = 0; index < hateTable->entryN; index++)
	{
		HateTableEntry entry = hateTable->entries[index];
		for(Int32 previousIndex = index - 1; previousIndex >= 0; previousIndex--)
		{
			HateTableEntry previousEntry = hateTable->entries[previousIndex];
			Bool32 needSwap = false;
			if(previousEntry.source > entry.source)
			{
				needSwap = true;
			}
			else if(previousEntry.source == entry.source && previousEntry.value < entry.value)
			{
				needSwap = true;
			}

			if(needSwap)
			{
				hateTable->entries[previousIndex] = entry;
				hateTable->entries[previousIndex + 1] = previousEntry;
			}
			else
			{
				break;
			}
		}
	}
}

static void
func UpdateEnemyTargets(CombatLabState* labState)
{
	HateTable* hateTable = &labState->hateTable;
	HateTableEntry* previousEntry = 0;
	for(Int32 i = 0; i < hateTable->entryN; i++)
	{
		HateTableEntry* entry = &hateTable->entries[i];
		if(entry->source->groupId == EnemyGroupId)
		{
			if(previousEntry != 0)
			{
				Assert(previousEntry->source <= entry->source);
				if(previousEntry->source != entry->source)
				{
					entry->source->target = entry->target;
				}
				else
				{
					Assert(previousEntry->value >= entry->value);
				}
			}
			else
			{
				entry->source->target = entry->target;
			}
		}
		previousEntry = entry;
	}
}

static ItemCooldown*
func GetItemCooldown(CombatLabState* labState, Entity* entity, Int32 itemId)
{
	itemId = GetItemIdForCooldown(itemId);
	Assert(ItemHasOwnCooldown(itemId));

	ItemCooldown* result = 0;
	for(Int32 i = 0; i < labState->itemCooldownN; i++)
	{
		ItemCooldown* cooldown = &labState->itemCooldowns[i];
		if(cooldown->entity == entity && cooldown->itemId == itemId)
		{
			result = cooldown;
			break;
		}
	}
	return result;
}

static Bool32
func CanSwapItems(InventoryItem* item1, InventoryItem* item2)
{
	Assert(InventoryItemIsValid(item1));
	Assert(InventoryItemIsValid(item2));

	Bool32 canPutItem1 = ItemGoesIntoSlot(item1->itemId, item2->slotId);
	Bool32 canPutItem2 = ItemGoesIntoSlot(item2->itemId, item1->slotId);
	Bool32 canSwap = (canPutItem1 && canPutItem2);
	return canSwap;
}

static void
func SwapItems(InventoryItem* item1, InventoryItem* item2)
{
	Assert(CanSwapItems(item1, item2));

	SetSlotItemId(item1->inventory, item1->slot, item2->itemId);
	SetSlotItemId(item2->inventory, item2->slot, item1->itemId);
}

static void
func AttemptToSwapItems(InventoryItem* item1, InventoryItem* item2)
{
	if(CanSwapItems(item1, item2))
	{
		SwapItems(item1, item2);
	}
}

#define InventorySlotSide 50

static void
func DrawInventorySlot(Canvas* canvas, CombatLabState* labState, Int32 slotId, Int32 itemId, Int32 top, Int32 left)
{
	Int32 bottom = top + InventorySlotSide;
	Int32 right = left + InventorySlotSide;

	Vec4 slotBackgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 itemBackgroundColor = MakeColor(0.1f, 0.1f, 0.1f);
	Vec4 cooldownColor = MakeColor(0.2f, 0.0f, 0.0f);
	Vec4 itemNameColor = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 slotNameColor = MakeColor(0.3f, 0.3f, 0.3f);

	Bitmap* bitmap = &canvas->bitmap;
	GlyphData* glyphData = canvas->glyphData;

	if(itemId == NoItemId)
	{
		DrawBitmapRect(bitmap, left, right, top, bottom, slotBackgroundColor);
		if(slotId != AnySlotId)
		{
			Int8* slotName = GetSlotName(slotId);
			Assert(slotName != 0);
			DrawBitmapTextLineCentered(bitmap, slotName, glyphData, left, right, top, bottom, slotNameColor);
		}
	}
	else
	{
		DrawBitmapRect(bitmap, left, right, top, bottom, itemBackgroundColor);

		Real32 totalCooldown = GetItemCooldownDuration(itemId);
		if(totalCooldown > 0.0f)
		{
			Entity* player = &labState->entities[0];
			ItemCooldown* cooldown = GetItemCooldown(labState, player, itemId);
			if(cooldown)
			{
				Real32 remainingCooldown = cooldown->timeRemaining;
				Real32 cooldownRatio = (remainingCooldown / totalCooldown);
				Assert(IsBetween(cooldownRatio, 0.0f, 1.0f));

				Int32 cooldownRight = (Int32)Lerp((Real32)left, cooldownRatio, (Real32)right);
				DrawBitmapRect(bitmap, left, cooldownRight, top, bottom, cooldownColor);
			}
		}

		Int8* name = GetItemSlotName(itemId);
		DrawBitmapTextLineCentered(bitmap, name, glyphData, left, right, top, bottom, itemNameColor);
	}
}

static void
func DrawInventorySlotOutline(Canvas* canvas, Int32 top, Int32 left, Vec4 color)
{
	Bitmap* bitmap = &canvas->bitmap;
	Int32 bottom = top + InventorySlotSide;
	Int32 right = left + InventorySlotSide;
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, color);
}

#define InventorySlotPadding 2

static Int32
func GetInventoryWidth(Inventory* inventory)
{
	Int32 width = InventorySlotPadding + inventory->colN * (InventorySlotSide + InventorySlotPadding);
	return width;
}

static Int32
func GetInventoryHeight(Inventory* inventory)
{
	Int32 height = InventorySlotPadding + inventory->rowN * (InventorySlotSide + InventorySlotPadding);
	return height;
}

static void
func UpdateAndDrawInventory(Canvas* canvas, CombatLabState* labState, Inventory* inventory, IntVec2 mousePosition)
{
	Bitmap* bitmap = &canvas->bitmap;
	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	Int32 slotPadding = 2;

	Int32 width = GetInventoryWidth(inventory);
	Int32 height = GetInventoryHeight(inventory);

	Int32 left = inventory->left;
	Int32 right = left + width;
	Int32 top = inventory->top;
	Int32 bottom = top + height;

	Vec4 backgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
	Vec4 hoverOutlineColor = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 invalidOutlineColor = MakeColor(1.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);

	InventoryItem* dragItem = &labState->dragItem;
	InventoryItem* hoverItem = &labState->hoverItem;

	Int32 slotTop = top + slotPadding;
	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		Int32 slotLeft = left + slotPadding;
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			Int32 slotId = GetInventorySlotId(inventory, row, col);
			Int32 itemId = GetInventoryItemId(inventory, row, col);

			Bool32 isHover = (IsIntBetween(mousePosition.col, slotLeft, slotLeft + InventorySlotSide) &&
							  IsIntBetween(mousePosition.row, slotTop, slotTop + InventorySlotSide));
			Bool32 isDrag = (dragItem->inventory == inventory && 
							 dragItem->slot.row == row && dragItem->slot.col == col);

			if(!isDrag)
			{
				DrawInventorySlot(canvas, labState, slotId, itemId, slotTop, slotLeft);
			}
			else
			{
				DrawInventorySlot(canvas, labState, slotId, NoItemId, slotTop, slotLeft);
			}

			if(isHover)
			{
				if(!isDrag && itemId != NoItemId)
				{
					Int32 tooltipBottom = slotTop - UIBoxPadding;
					Int32 tooltipRight = IntMin2((slotLeft + InventorySlotSide / 2) + TooltipWidth / 2,
												 (bitmap->width - 1) - UIBoxPadding);

					tooltipRight = IntMax2(tooltipRight, UIBoxPadding + TooltipWidth);

					Int8 tooltipBuffer[128];
					String tooltip = GetItemTooltipText(itemId, tooltipBuffer, 128);
					DrawBitmapStringTooltipBottomRight(bitmap, tooltip, glyphData, tooltipBottom, tooltipRight);
				}

				Vec4 outlineColor = hoverOutlineColor;
				if(dragItem->inventory && !ItemGoesIntoSlot(dragItem->itemId, slotId))
				{
					outlineColor = invalidOutlineColor;
				}

				DrawInventorySlotOutline(canvas, slotTop, slotLeft, outlineColor);
				hoverItem->inventory = inventory;
				hoverItem->slotId = slotId;
				hoverItem->itemId = itemId;
				hoverItem->slot.row = row;
				hoverItem->slot.col = col;
			}

			slotLeft += (InventorySlotSide + slotPadding);
		}

		slotTop += (InventorySlotSide + slotPadding);
	}
}

static void
func DrawCharacterInfo(Canvas* canvas, Entity* entity, CombatLabState* labState, IntVec2 mousePosition)
{
	Bitmap* bitmap = &canvas->bitmap;
	
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	Vec4 outlineColor = MakeColor(1.0f, 1.0f, 1.0f);

	Inventory* inventory = &labState->equipInventory;
	Int32 width = GetInventoryWidth(inventory);
	Int32 height = UIBoxPadding + 14 * TextHeightInPixels + UIBoxPadding;

	Int32 left   = UIBoxPadding;
	Int32 right  = left + width;
	Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
	Int32 top    = bottom - height;

	inventory->left = right - GetInventoryWidth(inventory);
	inventory->top = top - GetInventoryHeight(inventory);
	UpdateAndDrawInventory(canvas, labState, inventory, mousePosition);

	DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);

	GlyphData* glyphData = canvas->glyphData;
	Assert(glyphData != 0);

	Vec4 textColor  = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 titleColor = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 infoColor  = MakeColor(0.3f, 0.3f, 0.3f);

	Int32 textLeft = left + UIBoxPadding;
	Int32 textTop = top + UIBoxPadding;

	DrawBitmapTextLineTopLeft(bitmap, "Character Info", glyphData, textLeft, textTop, titleColor);
	textTop += TextHeightInPixels;

	Int8 lineBuffer[128] = {};
	OneLineString(lineBuffer, 128, "Level " + entity->level);
	DrawBitmapTextLineTopLeft(bitmap, lineBuffer, glyphData, textLeft, textTop, textColor);
	textTop += TextHeightInPixels;

	textTop += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Attributes", glyphData, textLeft, textTop, titleColor);
	textTop += TextHeightInPixels;

	OneLineString(lineBuffer, 128, "Constitution: " + entity->constitution);
	DrawBitmapTextLineTopLeft(bitmap, lineBuffer, glyphData, textLeft, textTop, textColor);
	textTop += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Increases maximum health.",
							  glyphData, textLeft, textTop, infoColor);
	textTop += TextHeightInPixels;

	OneLineString(lineBuffer, 128, "Strength: " + entity->strength);
	DrawBitmapTextLineTopLeft(bitmap, lineBuffer, glyphData, textLeft, textTop, textColor);
	textTop += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Increases damage of physical attacks.",
							  glyphData, textLeft, textTop, infoColor);
	textTop += TextHeightInPixels;

	OneLineString(lineBuffer, 128, "Intellect: " + entity->intellect);
	DrawBitmapTextLineTopLeft(bitmap, lineBuffer, glyphData, textLeft, textTop, textColor);
	textTop += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Increases damage of magical attacks.",
							  glyphData, textLeft, textTop, infoColor);
	textTop += TextHeightInPixels;

	OneLineString(lineBuffer, 128, "Dexterity: " + entity->dexterity);
	DrawBitmapTextLineTopLeft(bitmap, lineBuffer, glyphData, textLeft, textTop, textColor);
	textTop += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Reduces cast time of abilities.",
							  glyphData, textLeft, textTop, infoColor);
	textTop += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Professions", glyphData, textLeft, textTop, titleColor);
	textTop += TextHeightInPixels;

	OneLineString(lineBuffer, 128, "Herbalism: " + entity->herbalism);
	DrawBitmapTextLineTopLeft(bitmap, lineBuffer, glyphData, textLeft, textTop, textColor);
	textTop += TextHeightInPixels;

	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);
}

static void
func DrawHateTable(Canvas* canvas, CombatLabState* labState)
{
	Bitmap* bitmap = &canvas->bitmap;
	
	Entity* player = &labState->entities[0];
	Entity* target = player->target;

	HateTable* hateTable = &labState->hateTable;

	if(target != 0 && target->groupId != player->groupId)
	{
		Int32 lineN = GetHateTableEntityEntryN(hateTable, target);
		if(lineN > 0)
		{
			Int32 width = 200;
			Int32 height = UIBoxPadding + TextHeightInPixels + lineN * TextHeightInPixels + UIBoxPadding;
			Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
			Vec4 outlineColor = MakeColor(0.5f, 0.5f, 0.5f);
			Vec4 titleColor = MakeColor(1.0f, 1.0f, 0.0f);
			Vec4 textColor = MakeColor(1.0f, 1.0f, 1.0f);

			Int32 right  = (bitmap->width - 1) - UIBoxPadding - UIBoxSide - UIBoxPadding;
			Int32 left   = right - width;
			Int32 bottom = (bitmap->height - 1) - UIBoxPadding;
			Int32 top    = bottom - height;

			DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);
			DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);

			Int32 textLeft = left + UIBoxPadding;
			Int32 textRight = right - UIBoxPadding;
			Int32 textTop  = top + UIBoxPadding;

			DrawBitmapTextLineTopLeft(bitmap, "Hate", canvas->glyphData, textLeft, textTop, titleColor);
			textTop += TextHeightInPixels;

			for(Int32 i = 0; i < hateTable->entryN; i++)
			{
				HateTableEntry* entry = &hateTable->entries[i];
				if(entry->source == target)
				{
					Int8* name = entry->target->name;
					Int8 value[8];
					OneLineString(value, 8, entry->value);
					DrawBitmapTextLineTopLeft(bitmap, name, canvas->glyphData, textLeft, textTop, textColor);
					DrawBitmapTextLineTopRight(bitmap, value, canvas->glyphData, textRight, textTop, textColor);
					textTop += TextHeightInPixels;
				}
			}
		}
	}
}

static void
func UpdateEffects(CombatLabState* labState, Real32 seconds)
{
	Int32 remainingEffectN = 0;
	for(Int32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		effect->timeRemaining -= seconds;
		if(!EffectHasDuration(effect->effectId) || effect->timeRemaining > 0.0f)
		{
			labState->effects[remainingEffectN] = *effect;
			remainingEffectN++;
		}
		else if(effect->effectId == EarthShieldEffectId)
		{
			effect->entity->absorbDamage = 0;
		}
	}
	labState->effectN = remainingEffectN;

	for(Int32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		Real32 time = effect->timeRemaining;
		Real32 previousTime = time + seconds;
		Int32 effectId = effect->effectId;
		switch(effectId)
		{
			case BurningEffectId:
			{
				if(Floor(time) != Floor(previousTime))
				{
					DealDamageFromEffect(labState, effect->effectId, effect->entity, 2);
				}
				break;
			}
			case PoisonedEffectId:
			{
				if(Floor(time * (1.0f / 3.0f)) != Floor(previousTime * (1.0f / 3.0f)))
				{
					DealDamageFromEffect(labState, effect->effectId, effect->entity, 2);
				}
				break;
			}
			case RegenerateEffectId:
			{
				if(Floor(time) != Floor(previousTime))
				{
					AttemptToHeal(labState, effect->entity, effect->entity, 5);
				}
				break;
			}
			case HealOverTimeEffectId:
			{
				if(Floor(time * (1.0f / 3.0f)) != Floor(previousTime * (1.0f / 3.0f)))
				{
					AttemptToHeal(labState, effect->entity, effect->entity, 5);
				}
				break;
			}
		}
	}
}

static void
func UpdateDamageDisplays(CombatLabState* labState, Real32 seconds)
{
	Real32 scrollUpSpeed = 1.0f;

	Int32 remainingDisplayN = 0;
	for(Int32 i = 0; i < labState->damageDisplayN; i++)
	{
		DamageDisplay* display = &labState->damageDisplays[i];
		display->position.y -= scrollUpSpeed * seconds;

		display->timeRemaining -= seconds;
		if(display->timeRemaining > 0.0f)
		{
			labState->damageDisplays[remainingDisplayN] = *display;
			remainingDisplayN++;
		}
	}
	labState->damageDisplayN = remainingDisplayN;
}

static Bool32
func CanMove(CombatLabState* labState, Entity* entity)
{
	Bool32 canMove = false;
	if(IsDead(entity))
	{
		canMove = false;
	}
	else if(HasEffect(labState, entity, KickedEffectId) || HasEffect(labState, entity, RollingEffectId))
	{
		canMove = false;
	}
	else
	{
		canMove = true;
	}
	return canMove;
}

static Bool32
func CanPickUpItem(CombatLabState* labState, Entity* entity, DroppedItem* item)
{
	Bool32 isAlive = !IsDead(entity);
	Bool32 hasEmptySlot = HasEmptySlot(&labState->inventory);
	Bool32 canPickUpItem = (isAlive && hasEmptySlot);
	return canPickUpItem;
}

static void
func PickUpItem(CombatLabState* labState, Entity* entity, DroppedItem* item)
{
	Assert(item != 0);
	Assert(CanPickUpItem(labState, entity, item));

	Int32 itemIndex = 0;
	Bool32 itemFound = false;
	for(Int32 i = 0; i < labState->droppedItemN; i++)
	{
		if(item == &labState->droppedItems[i])
		{
			itemFound = true;
			itemIndex = i;
			break;
		}
	}

	Assert(itemFound);
	labState->droppedItemN--;
	for(Int32 i = itemIndex; i < labState->droppedItemN; i++)
	{
		labState->droppedItems[i] = labState->droppedItems[i + 1];
	}

	AddItemToInventory(&labState->inventory, item->itemId);

	Int8* itemName = GetItemName(item->itemId);
	CombatLog(labState, entity->name + " picks up " + itemName + ".");
}

static void
func PickUpFlower(CombatLabState* labState, Flower* flower)
{
	Inventory* inventory = &labState->inventory;
	Assert(HasEmptySlot(inventory));

	Bool32 foundFlower = false;
	Int32 flowerIndex = 0;
	for(Int32 i = 0; i < labState->flowerN; i++)
	{
		if(&labState->flowers[i] == flower)
		{
			foundFlower = true;
			flowerIndex = i;
			break;
		}
	}
	Assert(foundFlower);

	AddItemToInventory(inventory, flower->itemId);

	for(Int32 i = flowerIndex + 1; i < labState->flowerN; i++)
	{
		labState->flowers[i - 1] = labState->flowers[i];
	}

	labState->flowerN--;
}

static void
func UpdateAndDrawDroppedItems(Canvas* canvas, CombatLabState* labState, Vec2 mousePosition, Real32 seconds)
{
	labState->hoverDroppedItem = 0;

	Int32 remainingN = 0;
	for(Int32 i = 0; i < labState->droppedItemN; i++)
	{
		DroppedItem* item = &labState->droppedItems[i];
		item->timeLeft -= seconds;
		if(item->timeLeft > 0.0f)
		{
			labState->droppedItems[remainingN] = *item;
			remainingN++;
		}
	}
	labState->droppedItemN = remainingN;

	for(Int32 i = 0; i < labState->droppedItemN; i++)
	{
		DroppedItem* item = &labState->droppedItems[i];
		Int8* itemName = GetItemName(item->itemId);

		Vec4 normalBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
		Vec4 hoverBackgroundColor = MakeColor(0.7f, 0.5f, 0.5f);
		Vec4 textColor = MakeColor(1.0f, 1.0f, 1.0f);

		Real32 textWidth = GetTextWidth(canvas, itemName);
		Real32 textHeight = GetTextHeight(canvas, itemName);
		
		Camera* camera = canvas->camera;
		Assert(camera->unitInPixels > 0.0f);
		Rect rect = MakeRect(item->position, textWidth, textHeight);

		Vec4 backgroundColor = normalBackgroundColor;
		if(IsPointInRect(mousePosition, rect))
		{
			backgroundColor = hoverBackgroundColor;
			labState->hoverDroppedItem = item;
		}

		DrawRect(canvas, rect, backgroundColor);
		DrawTextLineXYCentered(canvas, itemName, item->position.y, item->position.x, textColor);
	}
}

static void
func DrawDamageDisplays(Canvas* canvas, CombatLabState* labState)
{
	Vec4 damageColor = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 healColor = MakeColor(0.0f, 0.5f, 0.0f);
	for(Int32 i = 0; i < labState->damageDisplayN; i++)
	{
		DamageDisplay* display = &labState->damageDisplays[i];
		Int8 text[16] = {};
		Vec4 textColor = {};
		if(display->damage > 0)
		{
			textColor = damageColor;
			OneLineString(text, 16, display->damage);
		}
		else if(display->damage < 0)
		{
			textColor = healColor;
			OneLineString(text, 16, -display->damage);
		}
		else
		{
			DebugBreak();
		}
		DrawTextLineXYCentered(canvas, text, display->position.y, display->position.x, textColor);
	}
}

static Vec2
func GetClosestRectOutlinePosition(Rect rect, Vec2 point)
{
	Vec2 result = {};

	Assert(IsPointInRect(point, rect));

	Real32 leftDistance = (point.x - rect.left);
	Assert(leftDistance >= 0.0f);

	Real32 minDistance = leftDistance;
	result = MakePoint(rect.left, point.y);;

	Real32 rightDistance = (rect.right - point.x);
	Assert(rightDistance >= 0.0f);
	if(rightDistance < minDistance)
	{
		minDistance = rightDistance;
		result = MakePoint(rect.right, point.y);
	}

	Real32 topDistance = (point.y - rect.top);
	Assert(topDistance >= 0.0f);
	if(topDistance < minDistance)
	{
		minDistance = topDistance;
		result = MakePoint(point.x, rect.top);
	}

	Real32 bottomDistance = (rect.bottom - point.y);
	Assert(bottomDistance >= 0.0f);
	if(bottomDistance < minDistance)
	{
		minDistance = bottomDistance;
		result = MakePoint(point.x, rect.bottom);
	}

	return result;
}

static Vec2
func GetEntityLeft(Entity* entity)
{
	Vec2 left = entity->position + MakeVector(-EntityRadius, 0.0f);
	return left;
}

static Vec2
func GetEntityRight(Entity* entity)
{
	Vec2 right = entity->position + MakeVector(+EntityRadius, 0.0f);
	return right;
}

static Vec2
func GetEntityTop(Entity* entity)
{
	Vec2 top = entity->position + MakeVector(0.0f, -EntityRadius);
	return top;
}

static Vec2
func GetEntityBottom(Entity* entity)
{
	Vec2 bottom = entity->position + MakeVector(0.0f, +EntityRadius);
	return bottom;
}

static void
func UpdateEntityMovement(Entity* entity, Map* map, Real32 seconds)
{
	Vec2 moveVector = seconds * entity->velocity;
	Vec2 oldPosition = entity->position;
	Vec2 newPosition = entity->position + moveVector;

	Poly16 collisionPoly = {};

	IntVec2 topTile = GetContainingTile(map, GetEntityTop(entity));
	topTile.row--;

	IntVec2 bottomTile = GetContainingTile(map, GetEntityBottom(entity));
	bottomTile.row++;

	IntVec2 leftTile = GetContainingTile(map, GetEntityLeft(entity));
	leftTile.col--;

	IntVec2 rightTile = GetContainingTile(map, GetEntityRight(entity));
	rightTile.col++;

	Bool32 treeFound = false;
	for(Int32 row = topTile.row; row <= bottomTile.row; row++)
	{
		for(Int32 col = leftTile.col; col <= rightTile.col; col++)
		{
			IntVec2 tile = MakeTile(row, col);
			if(IsValidTile(map, tile))
			{
				if(IsTileType(map, tile, TreeTileId))
				{
					collisionPoly = GetExtendedTreeOutline(map, tile, EntityRadius);
					treeFound = true;
					break;
				}
			}
		}

		if(treeFound)
		{
			break;
		}
	}

	if(treeFound)
	{
		Assert(collisionPoly.pointN >= 2);
		Int32 index1 = collisionPoly.pointN - 1;
		for(Int32 index2 = 0; index2 < collisionPoly.pointN; index2++)
		{
			Vec2 point1 = collisionPoly.points[index1];
			Vec2 point2 = collisionPoly.points[index2];
			if(point1.x == point2.x)
			{
				Real32 x = point1.x;
				if(point1.y < point2.y)
				{
					if(IsBetween(oldPosition.y, point1.y, point2.y) && (oldPosition.x >= x && newPosition.x < x))
					{
						newPosition.x = x;
					}
				}
				else if(point2.y < point1.y)
				{
					if(IsBetween(oldPosition.y, point2.y, point1.y) && (oldPosition.x <= x && newPosition.x > x))
					{
						newPosition.x = x;
					}
				}
				else
				{
					DebugBreak();
				}
			}
			else if(point1.y == point2.y)
			{
				Real32 y = point1.y;
				if(point1.x < point2.x)
				{
					if(IsBetween(oldPosition.x, point1.x, point2.x) && (oldPosition.y <= y && newPosition.y > y))
					{
						newPosition.y = y;
					}
				}
				else if(point2.x < point1.x)
				{
					if(IsBetween(oldPosition.x, point2.x, point1.x) && (oldPosition.y >= y && newPosition.y < y))
					{
						newPosition.y = y;
					}
				}
			}
			else
			{
				DebugBreak();
			}

			index1 = index2;
		}
	}

	Real32 mapWidth  = GetMapWidth(map);
	Real32 mapHeight = GetMapHeight(map);
	newPosition.x = Clip(newPosition.x, +EntityRadius, mapWidth  - EntityRadius);
	newPosition.y = Clip(newPosition.y, +EntityRadius, mapHeight - EntityRadius);

	entity->position = newPosition;
}

static void
func RemoveEffectsOfDeadEntities(CombatLabState* labState)
{
	Int32 remainingEffectN = 0;
	for(Int32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		if(!IsDead(effect->entity))
		{
			labState->effects[remainingEffectN] = *effect;
			remainingEffectN++;
		}
		else
		{
			Entity* player = &labState->entities[0];
			if(effect->entity == player)
			{
				RecalculatePlayerAttributes(labState);
			}
		}
	}
	labState->effectN = remainingEffectN;
}

static void
func DeleteInventoryItem(InventoryItem item)
{
	Inventory* inventory = item.inventory;
	Assert(inventory != 0);

	SetSlotItemId(item.inventory, item.slot, NoItemId);
}

static Bool32
func ItemIsOnCooldown(CombatLabState* labState, Entity* entity, Int32 itemId)
{
	Bool32 isOnCooldown = false;

	Int32 cooldownItemId = GetItemIdForCooldown(itemId);
	Assert(ItemHasOwnCooldown(cooldownItemId));

	for(Int32 i = 0; i < labState->itemCooldownN; i++)
	{
		ItemCooldown* cooldown = &labState->itemCooldowns[i];
		Assert(ItemHasOwnCooldown(cooldown->itemId));
		if(cooldown->entity == entity && cooldown->itemId == cooldownItemId)
		{
			isOnCooldown = true;
			break;
		}
	}
	return isOnCooldown;
}

static void
func AddItemCooldown(CombatLabState* labState, Entity* entity, Int32 itemId)
{
	Assert(!ItemIsOnCooldown(labState, entity, itemId));

	itemId = GetItemIdForCooldown(itemId);
	Assert(ItemHasOwnCooldown(itemId));

	Real32 duration = GetItemCooldownDuration(itemId);
	Assert(duration > 0.0f);

	ItemCooldown cooldown = {};
	cooldown.entity = entity;
	cooldown.itemId = itemId;
	cooldown.timeRemaining = duration;

	Assert(labState->itemCooldownN < MaxItemCooldownN);
	labState->itemCooldowns[labState->itemCooldownN] = cooldown;
	labState->itemCooldownN++;
}

static Bool32
func CanUseItem(CombatLabState* labState, Entity* entity, Int32 itemId)
{
	Bool32 canUse = true;
	if(IsDead(entity))
	{
		canUse = false;
	}
	else if(ItemIsOnCooldown(labState, entity, itemId))
	{
		canUse = false;
	}
	else
	{
		switch(itemId)
		{
			case HealthPotionItemId:
			{
				canUse = CanBeHealed(entity);
				break;
			}
			case AntiVenomItemId:
			{
				bool hasFriendlyTarget = (entity->target != 0 && entity->target->groupId == entity->groupId);
				bool targetIsAlive = (entity->target != 0 && !IsDead(entity->target));
				canUse = (hasFriendlyTarget && targetIsAlive);
				break;
			}
			case IntellectPotionItemId:
			{
				canUse = true;
				break;
			}
			case BlueFlowerItemId:
			case RedFlowerItemId:
			case YellowFlowerItemId:
			{
				DebugBreak();
				break;
			}
			case BlueFlowerOfIntellectItemId:
			case BlueFlowerOfHealingItemId:
			case BlueFlowerOfDampeningItemId:
			case RedFlowerOfStrengthItemId:
			case RedFlowerOfHealthItemId:
			case RedFlowerOfPoisonItemId:
			case YellowFlowerOfDexterityItemId:
			case YellowFlowerOfAntivenomItemId:
			case YellowFlowerOfRageItemId:
			{
				canUse = true;
				break;
			}
			default:
			{
				canUse = false;
			}
		}
	}
	return canUse;
}

static void
func UseItem(CombatLabState* labState, Entity* entity, InventoryItem item)
{
	Int32 itemId = item.itemId;
	Assert(CanUseItem(labState, entity, itemId));
	switch(itemId)
	{
		case HealthPotionItemId:
		{
			Heal(labState, entity, entity, 30);
			break;
		}
		case AntiVenomItemId:
		{
			RemoveEffect(labState, entity->target, PoisonedEffectId);
			break;
		}
		case IntellectPotionItemId:
		{
			ResetOrAddEffect(labState, entity, IntellectPotionEffectId);
			break;
		}
		case BlueFlowerItemId:
		case RedFlowerItemId:
		case YellowFlowerItemId:
		{
			DebugBreak();
			break;
		}
		case BlueFlowerOfIntellectItemId:
		{
			ResetOrAddEffect(labState, entity, FeelingSmartEffectId);
			break;
		}
		case BlueFlowerOfHealingItemId:
		{
			ResetOrAddEffect(labState, entity, HealOverTimeEffectId);
			break;
		}
		case BlueFlowerOfDampeningItemId:
		{
			ResetOrAddEffect(labState, entity, ReducedDamageDoneAndTakenEffectId);
			break;
		}
		case RedFlowerOfStrengthItemId:
		{
			ResetOrAddEffect(labState, entity, FeelingStrongEffectId);
			break;
		}
		case RedFlowerOfHealthItemId:
		{
			Heal(labState, entity, entity, 30);
			break;
		}
		case RedFlowerOfPoisonItemId:
		{
			ResetOrAddEffect(labState, entity, PoisonedEffectId);
			break;
		}
		case YellowFlowerOfDexterityItemId:
		{
			ResetOrAddEffect(labState, entity, FeelingQuickEffectId);
			break;
		}
		case YellowFlowerOfAntivenomItemId:
		{
			RemoveEffect(labState, entity->target, PoisonedEffectId);
			ResetOrAddEffect(labState, entity, ImmuneToPoisonEffectId);
			break;
		}
		case YellowFlowerOfRageItemId:
		{
			ResetOrAddEffect(labState, entity, IncreasedDamageDoneAndTakenEffectId);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	Int8* itemName = GetItemName(itemId);
	CombatLog(labState, entity->name + " uses item " + itemName + ".");
	DeleteInventoryItem(item);

	if(GetItemCooldownDuration(itemId) > 0.0f)
	{
		AddItemCooldown(labState, entity, itemId);
	}
}

static Real32
func GetEntityMoveSpeed(CombatLabState* labState, Entity* entity)
{
	Real32 moveSpeed = EntityMoveSpeed;
	if(IsDead(entity))
	{
		moveSpeed = 0.0f;
	}
	else
	{
		switch(entity->classId)
		{
			case SnakeClassId:
			case CrocodileClassId:
			{
				moveSpeed = AnimalMoveSpeed;
				break;
			}
			default:
			{
				moveSpeed = EntityMoveSpeed;
			}
		}

		if(HasEffect(labState, entity, BittenEffectId))
		{
			moveSpeed *= 0.5f;
		}

		if(HasEffect(labState, entity, EarthShakeEffectId))
		{
			Map* map = &labState->map;
			IntVec2 tile = GetContainingTile(map, entity->position);
			TileTypeId tileType = GetTileType(map, tile);
			if(tileType == GroundTileId)
			{
				moveSpeed *= 0.5f;
			}
		}
	}

	return moveSpeed;
}

static void
func CombatLabUpdate(CombatLabState* labState, Canvas* canvas, Real32 seconds, UserInput* userInput)
{
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	DrawMap(canvas, &labState->map);

	Vec2 mousePosition = PixelToUnit(canvas->camera, userInput->mousePixelPosition);
	UpdateAndDrawFlowers(canvas, labState, mousePosition);

	Real32 mapWidth  = GetMapWidth(&labState->map);
	Real32 mapHeight = GetMapHeight(&labState->map);

	Entity* player = &labState->entities[0];
	Assert(player->groupId == PlayerGroupId);

	UpdateAbilityCooldowns(labState, seconds);
	UpdateItemCooldowns(labState, seconds);
	UpdateEffects(labState, seconds);
	UpdateDamageDisplays(labState, seconds);

	player->inputDirection = MakeVector(0.0f, 0.0f);

	Map* map = &labState->map;

	Bool32 inputMoveLeft  = IsKeyDown(userInput, 'A') || IsKeyDown(userInput, VK_LEFT);
	Bool32 inputMoveRight = IsKeyDown(userInput, 'D') || IsKeyDown(userInput, VK_RIGHT);
	Bool32 inputMoveUp    = IsKeyDown(userInput, 'W') || IsKeyDown(userInput, VK_UP);
	Bool32 inputMoveDown  = IsKeyDown(userInput, 'S') || IsKeyDown(userInput, VK_DOWN);	

	IntVec2 playerTile = GetContainingTile(map, player->position);
	if(TileIsNearTree(map, playerTile))
	{
		ResetOrAddEffect(labState, player, RegenerateEffectId);
	}
	else
	{
		RemoveEffect(labState, player, RegenerateEffectId);
	}

	if(inputMoveLeft && inputMoveRight)
	{
		player->inputDirection.x = 0.0f;
	}
	else if(inputMoveLeft)
	{
		player->inputDirection.x = -1.0f;
	}
	else if(inputMoveRight)
	{
		player->inputDirection.x = +1.0f;
	}

	if(inputMoveUp && inputMoveDown)
	{
		player->inputDirection.y = 0.0f;
	}
	else if(inputMoveUp)
	{
		player->inputDirection.y = -1.0f;
	}
	else if(inputMoveDown)
	{
		player->inputDirection.y = +1.0f;
	}

	if(IsDead(player))
	{
		player->velocity = MakeVector(0.0f, 0.0f);
	}
	if(CanMove(labState, player))
	{
		Real32 moveSpeed = GetEntityMoveSpeed(labState, player);
		player->velocity = moveSpeed * player->inputDirection;
	}

	UpdateEntityMovement(player, map, seconds);

	if(player->inputDirection.x != 0.0f || player->inputDirection.y != 0.0f)
	{
		if(player->castedAbility != NoAbilityId)
		{
			player->castedAbility = NoAbilityId;
		}
	}

	Real32 enemyPullDistance = 30.0f;
	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* enemy = &labState->entities[i];
		if (enemy->groupId != EnemyGroupId)
		{
			continue;
		}

		if(enemy->target == 0)
		{
			Real32 distanceFromPlayer = MaxDistance(player->position, enemy->position);
			if(distanceFromPlayer <= enemyPullDistance)
			{
				enemy->target = player;
				AddEmptyHateTableEntry(&labState->hateTable, enemy, player);
			}
		}

		Entity* target = enemy->target;
		if(IsDead(enemy))
		{
			enemy->velocity = MakeVector(0.0f, 0.0f);
		}
		if(target == 0)
		{
			if(CanMove(labState, enemy))
			{
				enemy->velocity = MakeVector(0.0f, 0.0f);
			}
		}
		else if(CanMove(labState, enemy))
		{
			IntVec2 enemyTile = GetContainingTile(map, enemy->position);
			Assert(enemy->groupId != enemy->target->groupId);
			IntVec2 targetTile = GetContainingTile(map, enemy->target->position);
			if(enemyTile == targetTile)
			{
				enemy->velocity = MakeVector(0.0f, 0.0f);
			}
			else
			{
				IntVec2 nextTile = GetNextTileOnPath(map, enemyTile, targetTile);
				Real32 moveSpeed = GetEntityMoveSpeed(labState, enemy);
				enemy->velocity = moveSpeed * PointDirection(enemy->position, GetTileCenter(map, nextTile));
			}
		}

		UpdateEntityMovement(enemy, map, seconds);
	}

	if(WasKeyPressed(userInput, VK_TAB))
	{
		Entity* target = 0;
		Real32 targetPlayerDistance = 0.0f;
		for(Int32 i = 0; i < EntityN; i++)
		{
			Entity* enemy = &labState->entities[i];
			if(enemy->groupId == player->groupId || IsDead(enemy) || enemy == player->target)
			{
				continue;
			}
			Assert(enemy != player);

			Real32 enemyPlayerDistance = Distance(enemy->position, player->position);
			if(target == 0 || enemyPlayerDistance < targetPlayerDistance)
			{
				target = enemy;
				targetPlayerDistance = enemyPlayerDistance;
			}
		}

		if(target)
		{
			player->target = target;
		}
	}

	if(WasKeyPressed(userInput, VK_F1))
	{
		player->target = player;
	}

	if(WasKeyReleased(userInput, VK_LBUTTON))
	{
		if(labState->hoverAbilityId != NoAbilityId)
		{
			AttemptToUseAbility(labState, player, labState->hoverAbilityId);
		}
		else
		{
			for(Int32 i = 0; i < EntityN; i++)
			{
				Entity* entity = &labState->entities[i];
				Real32 distanceFromMouse = Distance(mousePosition, entity->position);
				if(!IsDead(entity) && distanceFromMouse <= EntityRadius)
				{
					player->target = entity;
					break;
				}
			}
		}
	}

	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* entity = &labState->entities[i];
		UpdateEntityRecharge(entity, seconds);
	}

	for(Int32 i = '1'; i <= '7'; i++)
	{
		if(WasKeyPressed(userInput, i))
		{
			AttemptToUseAbilityAtIndex(labState, player, i - '1');
		}
	}

	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* entity = &labState->entities[i];
		if(entity->classId == SnakeClassId)
		{
			AttemptToUseAbility(labState, entity, SnakeStrikeAbilityId);
		}
		else if(entity->classId == CrocodileClassId)
		{
			AttemptToUseAbility(labState, entity, CrocodileBiteAbilityId);
			AttemptToUseAbility(labState, entity, CrocodileLashAbilityId);
		}
	}

	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* entity = &labState->entities[i];
		if(entity->castedAbility != NoAbilityId)
		{
			if(IsDead(entity))
			{
				entity->castedAbility = NoAbilityId;
			}
			else
			{
				Assert(entity->castTimeTotal > 0.0f);
				entity->castTimeRemaining -= seconds;
				if(entity->castTimeRemaining <= 0.0f)
				{
					FinishCasting(labState, entity);
				}
			}
		}
	}

	if(WasKeyReleased(userInput, VK_RBUTTON))
	{
		if(labState->hoverItem.itemId != NoItemId)
		{
			if(CanUseItem(labState, player, labState->hoverItem.itemId))
			{
				UseItem(labState, player, labState->hoverItem);
			}
		}
	}

	RemoveDeadEntitiesFromHateTable(&labState->hateTable);
	SortHateTable(&labState->hateTable);
	UpdateEnemyTargets(labState);
	RemoveEffectsOfDeadEntities(labState);

	Vec4 playerColor = MakeColor(0.0f, 1.0f, 1.0f);
	if(player == player->target)
	{
		DrawSelectedEntity(canvas, player, playerColor);
	}
	else
	{
		DrawEntity(canvas, player, playerColor);
	}

	Vec4 enemyColor = MakeColor(1.0f, 0.0f, 0.0f);
	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* enemy = &labState->entities[i];
		if(enemy->groupId != EnemyGroupId)
		{
			continue;
		}

		if(enemy == player->target)
		{
			DrawSelectedEntity(canvas, enemy, enemyColor);
		}
		else
		{
			DrawEntity(canvas, enemy, enemyColor);
		}
	}

	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* entity = &labState->entities[i];
		DrawEntityBars(canvas, entity);
	}

	Vec4 entityNameColor = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 castAbilityNameColor = MakeColor(0.0f, 1.0f, 1.0f);
	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* entity = &labState->entities[i];
		Real32 textCenterX = entity->position.x;
		Real32 textBottom = entity->position.y - 2.0f * EntityRadius;

		if(entity->castedAbility == NoAbilityId)
		{
			Int8 name[32];
			OneLineString(name, 32, "L" + entity->level + " " + entity->name);
			DrawTextLineBottomXCentered(canvas, name, textBottom, textCenterX, entityNameColor);
		}
		else
		{
			Int8* abilityName = GetAbilityName(entity->castedAbility);
			DrawTextLineBottomXCentered(canvas, abilityName, textBottom, textCenterX, castAbilityNameColor);
		}
	}

	canvas->camera->center = player->position;

	if(WasKeyReleased(userInput, 'V'))
	{
		labState->showInventory = !labState->showInventory;
	}

	if(WasKeyReleased(userInput, 'C'))
	{
		labState->showCharacterInfo = !labState->showCharacterInfo;
	}

	if(WasKeyPressed(userInput, VK_LBUTTON))
	{
		if(labState->hoverItem.inventory != 0 && labState->hoverItem.itemId != NoItemId)
		{
			labState->dragItem = labState->hoverItem;
		}
	}

	if(WasKeyReleased(userInput, VK_LBUTTON))
	{
		InventoryItem* dragItem = &labState->dragItem;
		InventoryItem* hoverItem = &labState->hoverItem;
		if(dragItem->inventory != 0)
		{
			Assert(dragItem->itemId != NoItemId);
			if(hoverItem->inventory != 0)
			{
				AttemptToSwapItems(dragItem, hoverItem);
			}
			else
			{
				DropItem(labState, player, dragItem->itemId);
				SetSlotItemId(dragItem->inventory, dragItem->slot, NoItemId);
			}
		}
		labState->dragItem.inventory = 0;
	}

	if(WasKeyPressed(userInput, ' '))
	{
		DroppedItem* item = labState->hoverDroppedItem;
		if(item != 0)
		{
			if(CanPickUpItem(labState, player, item))
			{
				PickUpItem(labState, player, item);
			}
		}

		Flower* flower = labState->hoverFlower;
		if(flower)
		{
			if(HasEmptySlot(&labState->inventory))
			{
				PickUpFlower(labState, flower);
				player->herbalism++;
			}
		}
	}

	labState->hoverItem.inventory = 0;
	if(labState->showInventory)
	{
		Inventory* inventory = &labState->inventory;
		Int32 right = (canvas->bitmap.width - 1) - UIBoxPadding - UIBoxSide - UIBoxPadding;
		Int32 bottom = (canvas->bitmap.height - 1) - UIBoxPadding;
		inventory->left = right - GetInventoryWidth(inventory);
		inventory->top = bottom - GetInventoryHeight(inventory);
		UpdateAndDrawInventory(canvas, labState, inventory, userInput->mousePixelPosition);
	}
	else
	{
		DrawHateTable(canvas, labState);
	}

	DrawAbilityBar(canvas, labState, userInput->mousePixelPosition);
	DrawHelpBar(canvas, labState, userInput->mousePixelPosition);
	DrawPlayerEffectBar(canvas, labState);
	DrawTargetEffectBar(canvas, labState);
	DrawDamageDisplays(canvas, labState);

	if(labState->showCharacterInfo)
	{
		DrawCharacterInfo(canvas, player, labState, userInput->mousePixelPosition);
	}
	else
	{
		DrawCombatLog(canvas, labState);
	}

	InventoryItem* dragItem = &labState->dragItem;
	if(dragItem->inventory != 0)
	{
		Assert(dragItem->itemId != 0);
		
		Int32 slotTop  = userInput->mousePixelPosition.row - InventorySlotSide / 2;
		Int32 slotLeft = userInput->mousePixelPosition.col - InventorySlotSide / 2;

		Vec4 dragOutlineColor = MakeColor(1.0f, 0.5f, 0.0f);

		DrawInventorySlot(canvas, labState, dragItem->slotId, dragItem->itemId, slotTop, slotLeft);
		DrawInventorySlotOutline(canvas, slotTop, slotLeft, dragOutlineColor);
	}

	UpdateAndDrawDroppedItems(canvas, labState, mousePosition, seconds);
}

// TODO: Put different items on the same cooldown!
// TODO: Stop spamming combat log when dying next to a tree!
// TODO: Don't log overheal!
// TODO: Mana
// TODO: Offensive and defensive target
// TODO: Invisibility
// TODO: Neutral enemies
// TODO: Event queue
// TODO: Icons?
// TODO: Show effects above entities' heads
// TODO: Entities attacking each other get too close
// TODO: Druid: Tame ability?
// TODO: Druid: Debuff/buff abilities?
// TODO: General pet handling code
// TODO: Effect tooltips
// TODO: Simplify pathfinding!
// TODO: Smooth automatic movement around trees!
// TODO: Handle entities bigger than a tile!
// TODO: Computer controlled enemies shouldn't stack upon each other
// TODO: Momentum and acceleration
// TODO: Remove limitation of 8 directions!
// TODO: Damage types
// TODO: Text display for effects
// TODO: Sword swing should only damage enemies in front of the paladin!
// TODO: Entity facing direction!