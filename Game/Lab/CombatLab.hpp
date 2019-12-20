#pragma once

#include "../Ability.hpp"
#include "../Effect.hpp"
#include "../Item.hpp"
#include "../Map.hpp"
#include "../UserInput.hpp"

#define EntityRadius 1.0f

#define MaxLevel 5
I32 MaxHealthAtLevel[] =
{
	0,
	100,
	150,
	200,
	250,
	300
};

enum GroupId
{
	PlayerGroupId,
	EnemyGroupId
};

struct Entity
{
	I8 *name;

	I32 level;

	V2 position;
	V2 velocity;

	R32 recharge;
	R32 recharge_from;
	I32 health;
	I32 absorb_damage;

	AbilityId casted_ability;
	R32 cast_time_total;
	R32 cast_time_remaining;
	Entity *cast_target;

	ClassId class_id;
	GroupId group_id;
	Entity *target;

	I32 strength;
	I32 intellect;
	I32 constitution;
	I32 dexterity;

	I32 herbalism;

	V2 input_direction;
};

struct AbilityCooldown
{
	Entity *entity;
	AbilityId ability_id;
	R32 time_remaining;
};

struct ItemCooldown
{
	Entity *entity;
	ItemId item_id;
	R32 time_remaining;
};

#define DamageDisplayDuration 2.0f
struct DamageDisplay
{
	R32 time_remaining;
	V2 position;
	I32 damage;
};

struct Effect
{
	Entity *entity;
	EffectId effect_id;
	R32 time_remaining;
};

struct HateTableEntry
{
	Entity *source;
	Entity *target;
	I32 value;
};

#define MaxHateTableEntryN 64
struct HateTable
{
	HateTableEntry entries[MaxHateTableEntryN];
	I32 entry_n;
};

#define DroppedItemTotalDuration 10.0f

struct DroppedItem
{
	V2 position;
	ItemId item_id;
	R32 time_left;
};

struct Flower
{
	V2 position;
	ItemId item_id;
};

#define EntityN 256
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
	I8 arena_memory[CombatLabArenaSize];
	MemArena arena;

	Map map;

	AbilityCooldown ability_cooldowns[MaxAbilityCooldownN];
	I32 ability_cooldown_n;

	ItemCooldown item_cooldowns[MaxItemCooldownN];
	I32 item_cooldown_n;

	Effect effects[MaxEffectN];
	I32 effect_n;

	Entity entities[EntityN];

	DroppedItem dropped_items[MaxDroppedItemN];
	I32 dropped_item_n;

	Flower flowers[MaxFlowerN];
	I32 flower_n;
	Flower *hover_flower;

	DroppedItem *hover_dropped_item;

	DamageDisplay damage_displays[MaxDamageDisplayN];
	I32 damage_display_n;

	I8 combat_log_lines[MaxCombatLogLineN][MaxCombatLogLineLength + 1];
	I32 combat_log_last_line_index;
	I32 combat_log_line_n;

	HateTable hate_table;

	AbilityId hover_ability_id;

	B32 show_character_info;
	Inventory equip_inventory;

	B32 show_inventory;
	Inventory inventory;

	InventoryItem hover_item;
	InventoryItem drag_item;
};

#define EntityMoveSpeed 11.0f
#define AnimalMoveSpeed 10.0f

#define EnemyAttackRadius 10.0f

#define MaxMeleeAttackDistance (4.0f * EntityRadius)
#define MaxRangedAttackDistance (30.0f)

static I32
func GetNextCombatLogLineIndex(I32 index)
{
	I32 next_index = (index < MaxCombatLogLineN - 1) ? (index + 1) : 0;
	return next_index;
}

static void
func AddCombatLogLine(CombatLabState *lab_state, I8* line)
{
	I32 last_index = lab_state->combat_log_last_line_index;
	I32 next_index = GetNextCombatLogLineIndex(last_index);

	B32 has_zero_byte = false;
	for(I32 i = 0; i < MaxCombatLogLineLength; i++)
	{
		lab_state->combat_log_lines[next_index][i] = line[i];
		if(line[i] == 0)
		{
			has_zero_byte = true;
			break;
		}
	}
	Assert(has_zero_byte);

	lab_state->combat_log_last_line_index = next_index;
	lab_state->combat_log_line_n++;
}

#define CombatLog(lab_state, line) {I8 log_line[MaxCombatLogLineLength + 1]; \
								   OneLineString(log_line, MaxCombatLogLineLength + 1, line); \
								   AddCombatLogLine(lab_state, log_line);}

static void
func AddCombatLogStringLine(CombatLabState *lab_state, String line)
{
	AddCombatLogLine(lab_state, line.buffer);
}

static V2
func FindEntityStartPosition(Map *map)
{
	IV2 initial_tile = GetRandomTile(map);
	V2 start_position = GetTileCenter(map, initial_tile);
	return start_position;
}

static I32
func GetEntityMaxHealth(Entity *entity)
{
	Assert(IsIntBetween(entity->level, 1, 5));
	I32 max_health = MaxHealthAtLevel[entity->level] + entity->constitution * 10;
	return max_health;
}

static void
func CombatLabInit(CombatLabState *lab_state, Canvas *canvas)
{
	lab_state->arena = CreateMemArena(lab_state->arena_memory, CombatLabArenaSize);

	Map *map = &lab_state->map;

	canvas->glyph_data = GetGlobalGlyphData();

	Camera *camera = canvas->camera;
	camera->unit_in_pixels = 15.0f;
	camera->center = MakePoint(0.0f, 0.0f);

	R32 map_width  = GetMapWidth(map);
	R32 map_height = GetMapHeight(map);

	Entity *player = &lab_state->entities[0];
	player->level = 1;
	player->name = "Player";
	player->position = FindEntityStartPosition(map);
	IV2 player_tile = GetContainingTile(map, player->position);
	player->input_direction = MakePoint(0.0f, 0.0f);
	player->class_id = DruidClassId;
	player->group_id = PlayerGroupId;

	player->strength = 1;
	player->constitution = 1;
	player->dexterity = 1;
	player->intellect = 1;

	player->herbalism = 1;

	player->health = GetEntityMaxHealth(player);

	for(I32 i = 1; i < EntityN; i++)
	{
		Entity *enemy = &lab_state->entities[i];
		IV2 tile = GetRandomTile(map);
		enemy->position = GetTileCenter(map, tile);

		enemy->group_id = EnemyGroupId;
		enemy->health = GetEntityMaxHealth(enemy);
	}

	MemArena *arena = &lab_state->arena;

	Inventory *inventory = &lab_state->inventory;
	InitInventory(inventory, arena, 3, 4);
	AddItemToInventory(inventory, HealthPotionItemId);
	AddItemToInventory(inventory, AntiVenomItemId);
	AddItemToInventory(inventory, IntellectPotionItemId);
	AddItemToInventory(inventory, TestHelmItemId);

	Inventory *equip_inventory = &lab_state->equip_inventory;
	InitInventory(equip_inventory, arena, 1, 6);
	SetInventorySlotId(equip_inventory, 0, 0, HeadSlotId);
	SetInventorySlotId(equip_inventory, 0, 1, ChestSlotId);
	SetInventorySlotId(equip_inventory, 0, 2, HandsSlotId);
	SetInventorySlotId(equip_inventory, 0, 3, WaistSlotId);
	SetInventorySlotId(equip_inventory, 0, 4, LegsSlotId);
	SetInventorySlotId(equip_inventory, 0, 5, FeetSlotId);

	lab_state->flower_n = MaxFlowerN;
	for(I32 i = 0; i < lab_state->flower_n; i++)
	{
		Flower *flower = &lab_state->flowers[i];
		flower->position = GetRandomTileCenter(&lab_state->map);
		flower->item_id = GetRandomFlowerItemId();
	}
}

#define TileGridColor MakeColor(0.2f, 0.2f, 0.2f)

static B32
func AbilityIsOnCooldown(CombatLabState *labState, Entity *entity, AbilityId ability_id)
{
	B32 is_on_cooldown = false;
	for(I32 i = 0; i < labState->ability_cooldown_n; i++)
	{
		AbilityCooldown *cooldown = &labState->ability_cooldowns[i];
		if(cooldown->entity == entity && cooldown->ability_id == ability_id)
		{
			is_on_cooldown = true;
			break;
		}
	}
	return is_on_cooldown;
}

static B32
func IsDead(Entity *entity)
{
	B32 is_dead = (entity->health == 0);
	return is_dead;
}

static B32
func HasEffect(CombatLabState *lab_state, Entity *entity, EffectId effect_id)
{
	B32 has_effect = false;
	for(I32 i = 0; i < lab_state->effect_n; i++)
	{
		Effect *effect = &lab_state->effects[i];
		if(effect->entity == entity && effect->effect_id == effect_id)
		{
			has_effect = true;
			break;
		}
	}
	return has_effect;
}

static B32
func AbilityIsEnabled(CombatLabState *lab_state, Entity *entity, AbilityId ability_id)
{
	B32 enabled = false;
	Entity *target = entity->target;

	I32 ability_class_id = GetAbilityClass(ability_id);

	B32 has_living_enemy_target = (target != 0) && 
									 (!IsDead(target)) && 
									 (entity->group_id != target->group_id);

	B32 has_living_friendly_target = (target != 0) && 
										(!IsDead(target)) && 
										(entity->group_id == target->group_id);

	R32 distance_from_target = (target != 0) 
									? MaxDistance(entity->position, target->position) 
									: 0.0f;

	if(IsDead(entity))
	{
		enabled = false;
	}
	else if(ability_class_id != entity->class_id)
	{
		DebugBreak();
		enabled = false;
	}
	else if(entity->casted_ability != NoAbilityId)
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
			case TigerBiteAbilityId:
			{
				enabled = has_living_enemy_target && (distance_from_target <= MaxMeleeAttackDistance);
				break;
			}
			case RollAbilityId:
			{
				enabled = (entity->input_direction.x != 0.0f || entity->input_direction.y != 0.0f);
				break;
			}
			case LightningAbilityId:
			case EarthShakeAbilityId:
			case BurnAbilityId:
			{
				enabled = has_living_enemy_target && (distance_from_target <= MaxRangedAttackDistance);
				break;
			}
			case HealAbilityId:
			{
				Map* map = &labState->map;
				IV2 entity_tile = GetContainingTile(map, entity->position);
				enabled = (has_living_friendly_target);
				break;
			}
			case EarthShieldAbilityId:
			{
				enabled = has_living_friendly_target;
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

	if(entity->class_id == PaladinClassId && HasEffect(lab_state, entity, ShieldRaisedEffectId))
	{
		if(ability_id == SwordStabAbilityId || ability_id == SwordSwingAbilityId)
		{
			enabled = false;
		}
	}
	else if(HasEffect(lab_state, entity, BlindEffectId))
	{
		enabled = false;
	}

	return enabled;
}

static B32
func CanUseAbility(CombatLabState *lab_state, Entity *entity, AbilityId ability_id)
{
	B32 can_use_ability = true;
	I32 min_level = GetAbilityMinLevel(ability_id);
	if(entity->level < min_level)
	{
		can_use_ability = false;
	}
	else if(entity->recharge > 0.0f || AbilityIsOnCooldown(lab_state, entity, ability_id))
	{
		can_use_ability = false;
	}
	else if(entity->casted_ability != NoAbilityId)
	{
		can_use_ability = false;
	}
	else if(!AbilityIsEnabled(lab_state, entity, ability_id))
	{
		can_use_ability = false;
	}
	return can_use_ability;
}

static void
func PutOnRecharge(Entity *entity, R32 recharge_time)
{
	Assert(recharge_time > 0.0f);
	Assert(entity->recharge == 0.0f);
	entity->recharge_from = recharge_time;
	entity->recharge = recharge_time;
}

static B32
func CanTakeDamage(CombatLabState *lab_state, Entity* entity)
{
	B32 can_damage = false;
	if(IsDead(entity))
	{
		can_damage = false;
	}
	else if(HasEffect(lab_state, entity, InvulnerableEffectId))
	{
		can_damage = false;
	}
	else
	{
		can_damage = true;
	}
	return can_damage;
}

static void
func AddDamageDisplay(CombatLabState *lab_state, V2 position, I32 damage)
{
	Assert(lab_state->damageDisplayN < MaxDamageDisplayN);
	DamageDisplay *display = &lab_state->damageDisplays[labState->damage_display_n];
	lab_state->damage_display_n++;
	display->position = position;
	display->damage = damage;
	display->time_remaining = DamageDisplayDuration;
}

static HateTableEntry *
func GetHateTableEntry(HateTable *hate_table, Entity* source, Entity* target)
{
	HateTableEntry *result = 0;
	for(I32 i = 0; i < hate_table->entry_n; i++)
	{
		HateTableEntry *entry = &hate_table->entries[i];
		if(entry->source == source && entry->target == target)
		{
			result = entry;
			break;
		}
	}
	return result;
}

static HateTableEntry *
func AddEmptyHateTableEntry(HateTable *hate-table, Entity *source, Entity *target)
{
	Assert(GetHateTableEntry(hate_table, source, target) == 0);
	Assert(hate_table->entry_n < MaxHateTableEntryN);
	HateTableEntry *entry = &hate_table->entries[hateTable->entry_n];
	hateTable->entry_n++;

	entry->source = source;
	entry->target = target;
	entry->value = 0;
	return entry;
}

static void
func GenerateHate(HateTable *hate_table, Entity *source, Entity *target, I32 value)
{
	Assert(value >= 0);
	HateTableEntry *entry = GetHateTableEntry(hate_table, source, target);
	if(entry == 0)
	{
		entry = AddEmptyHateTableEntry(hate_table, source, target);
	}

	Assert(entry != 0);
	entry->value += value;
}

static ItemId
func GetVisibleItemId(CombatLabState *lab_state, ItemId item_id)
{
	Entity *player = &lab_state->entities[0];
	Assert(item_id != NoItemId);

	I32 herbalism = player->herbalism;
	ItemId visible_item_id = NoItemId;
	switch(item_id)
	{
		case BlueFlowerOfIntellectItemId:
		case BlueFlowerOfHealingItemId:
		case BlueFlowerOfDampeningItemId:
		{
			visible_item_id = (herbalism >= 5) ? item_Id : BlueFlowerItemId;
			break;
		}
		case RedFlowerOfStrengthItemId:
		case RedFlowerOfHealthItemId:
		case RedFlowerOfPoisonItemId:
		{
			visible_item_id = (herbalism >= 10) ? item_id : RedFlowerItemId;
			break;
		}
		case YellowFlowerOfDexterityItemId:
		case YellowFlowerOfAntivenomItemId:
		case YellowFlowerOfRageItemId:
		{
			visible_item_id = (herbalism >= 15) ? item_id : YellowFlowerItemId;
			break;
		}
		default:
		{
			visible_item_id = item_id;
		}
	}

	Assert(visible_item_id != NoItemId);
	return visible_item_id;
}

static I8 *
func GetVisibleItemName(CombatLabState *lab_state, ItemId itemId)
{
	ItemId visible_item_id = GetVisibleItemId(lab_state, item_id);
	I8 *name = GetItemName(visible_item_id);
	return name;
}

static void
func DropItem(CombatLabState *lab_state, Entity* entity, ItemId item_id)
{
	Assert(item_id != NoItemId);

	DroppedItem item = {};
	item.item_id = itemId;
	item.position = entity->position;
	item.time_left = DroppedItemTotalDuration;

	Assert(lab_state->dropped_item_n + 1 < MaxDroppedItemN);
	lab_state->dropped_items[lab_state->dropped_item_n] = item;
	lab_state->dropped_item_n++;

	I8 *item_name = GetVisibleItemName(lab_state, item_id);
	CombatLog(lab_state, entity->name + " drops " + item_name + ".");
}

static void
func DropLoot(CombatLabState *lab_state, Entity *entity)
{
	if(entity->class_id == SnakeClassId)
	{
		DropItem(lab_state, entity, AntiVenomItemId);
	}
}

static void
func DealFinalDamage(Entity *entity, I32 damage)
{
	Assert(damage > 0);

	if(entity->absorb_damage >= damage)
	{
		entity->absorb_damage -= damage;
	}
	else
	{
		damage -= entity->absorb_damage;
		entity->absorb_damage = 0;
		entity->health = IntMax2(entity->health - damage, 0);
	}
}

static void
func LevelUp(Entity *entity)
{
	Assert(IsIntBetween(entity->level, 1, MaxLevel - 1));
	entity->level++;
	entity->intellect++;
	entity->strength++;
	entity->dexterity++;
	entity->constitution++;
}

static I32
func GetFinalDamage(CombatLabState *lab_state, Entity *source, Entity *target, I32 damage)
{
	I32 final_damage = damage;
	Assert(CanTakeDamage(lab_state, target));

	if(source)
	{
		if(HasEffect(lab_state, source, ReducedDamageDoneAndTakenEffectId))
		{
			final_damage -= final_damage / 5;
		}
	}

	if(HasEffect(lab_state, target, ReducedDamageDoneAndTakenEffectId))
	{
		final_damage -= final_damage / 5;
	}
	if(HasEffect(lab_state, target, ShieldRaisedEffectId))
	{
		final_damage -= final_damage / 2;
	}
	if(HasEffect(lab_state, target, EarthShieldEffectId))
	{
		final_damage -= final_damage / 10;
	}
	return final_damage;
}

static void
func DealDamageFromEntity(CombatLabState *lab_state, Entity *source, Entity *target, I32 damage)
{
	Assert(source->group_id != target->group_id);
	if(CanTakeDamage(lab_state, target))
	{
		I32 final_damage = GetFinalDamage(lab_state, source, target, damage);

		DealFinalDamage(target, final_damage);
		AddDamageDisplay(lab_state, target->position, final_damage);

		GenerateHate(&lab_state->hate_table, target, source, damage);

		CombatLog(lab_state, source->name + " deals " + damage + " damage to " + target->name + ".");
		if(IsDead(target))
		{
			DropLoot(lab_state, target);
			CombatLog(lab_state, target->name + " dies.");

			if(target->group_id == EnemyGroupId)
			{
				Entity* player = &lab_state->entities[0];
				if(player->level < MaxLevel)
				{
					LevelUp(player);
				}
			}
		}
	}
}

static void
func DealDamageFromEffect(CombatLabState *lab_state, EffectId effect_id, 
						  Entity *target, I32 damage)
{
	if(CanTakeDamage(lab_state, target))
	{
		I32 final_damage = GetFinalDamage(lab_state, 0, target, damage);
		DealFinalDamage(target, final_damage);
		AddDamageDisplay(lab_state, target->position, final_damage);

		I8 *effect_name = GetEffectName(effect_id);
		CombatLog(lab_state, effect_name + " deals " + final_damage + " damage to " + target->name + ".");
		if(IsDead(target))
		{
			DropLoot(lab_state, target);
			CombatLog(lab_state, target->name + " dies.");

			if(target->group_id == EnemyGroupId)
			{
				Entity *player = &lab_state->entities[0];
				if(player->level < MaxLevel)
				{
					LevelUp(player);
				}
			}
		}
	}
}

static B32
func CanBeHealed(Entity *entity)
{
	B32 can_be_healed = !IsDead(entity);
	return can_be_healed;
}

static void
func Heal(CombatLabState *lab_state, Entity *source, Entity *target, I32 healing)
{
	Assert(CanBeHealed(target));
	Assert(!IsDead(source));
	Assert(source->groupId == target->group_id);
	I32 max_health = GetEntityMaxHealth(target);
	target->health = IntMin2(target->health + healing, max_health);
	AddDamageDisplay(lab_state, target->position, -healing);

	if(source == target)
	{
		CombatLog(lab_state, target->name + " heals for " + healing + ".");
	}
	else
	{
		CombatLog(lab_state, source->name + " heals " + target->name + " for " + healing + ".");
	}
}

static void
func AttemptToHeal(CombatLabState *lab_state, Entity *source, Entity *target, I32 healing)
{
	if(CanBeHealed(target))
	{
		Heal(lab_state, source, target, healing);
	}
}

static void
func AddAbilityCooldown(CombatLabState *lab_state, Entity *entity, AbilityId ability_id)
{
	Assert(AbilityHasCooldown(ability_id));
	R32 duration = GetAbilityCooldownDuration(ability_id);

	Assert(duration > 0.0f);
	Assert(lab_state->ability_cooldown_n < MaxAbilityCooldownN);
	AbilityCooldown* cooldown = &lab_state->ability_cooldowns[lab_state->ability_cooldown_n];
	lab_state->ability_cooldown_n++;

	cooldown->entity = entity;
	cooldown->ability_id = ability_id;
	cooldown->time_remaining = duration;
}

static AbilityCooldown *
func GetAbilityCooldown(CombatLabState *lab_state, Entity *entity, AbilityId ability_id)
{
	AbilityCooldown *result = 0;
	for(I32 i = 0; i < labState->ability_cooldown_n; i++)
	{
		AbilityCooldown *cooldown = &lab_state->ability_cooldowns[i];
		if(cooldown->entity == entity && cooldown->ability_id == ability_id)
		{
			result = cooldown;
			break;
		}
	}
	return result;
}

static V2
func GetClosestMoveDirection(V2 distance)
{
	V2 left  = MakeVector(-1.0f, 0.0f);
	V2 right = MakeVector(+1.0f, 0.0f);
	V2 up    = MakeVector(0.0f, -1.0f);
	V2 down  = MakeVector(0.0f, +1.0f);
	V2 direction = {};

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
func RecalculatePlayerAttributes(CombatLabState *lab_state)
{
	Entity *player = &lab_state->entities[0];
	Assert(IsIntBetween(player->level, 1, MaxLevel));

	player->constitution = player->level;
	player->strength     = player->level;
	player->intellect    = player->level;
	player->dexterity    = player->level;

	Inventory *equip_inventory = &labState->equip_inventory;
	for(I32 row = 0; row < equip_inventory->rowN; row++)
	{
		for(I32 col = 0; col < equip_inventory->colN; col++)
		{
			ItemId item_id = GetInventoryItemId(equip_inventory, row, col);
			if(item_id != NoItemId)
			{
				ItemAttributes attributes = GetItemAttributes(itemId);
				player->constitution += attributes.constitution;
				player->strength     += attributes.strength;
				player->intellect    += attributes.intellect;
				player->dexterity    += attributes.dexterity;
			}
		}
	}

	for(I32 i = 0; i < lab_state->effect_n; i++)
	{
		Effect *effect = &lab_state->effects[i];
		if(effect->entity == player)
		{
			switch(effect->effect_id)
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
func RemoveEffect(CombatLabState *lab_state, Entity *entity, EffectId effect_id)
{
	I32 remaining_effect_n = 0;
	for(I32 i = 0; i < lab_state->effect_n; i++)
	{
		Effect *effect = &lab_state->effects[i];
		if(effect->entity != entity || effect->effect_id != effect_id)
		{
			lab_state->effects[remaining_effect_n] = *effect;
			remaining_effect_n++;
		}
	}
	lab_state->effectN = remaining_effect_n;

	Entity *player = &lab_state->entities[0];
	if(entity == player)
	{
		RecalculatePlayerAttributes(lab_state);
	}
}

static B32
func CanAddEffect(CombatLabState *lab_state, Entity *entity, EffectId effect_id)
{
	B32 can_add_effect = true;
	switch(effect_id)
	{
		case PoisonedEffectId:
		{
			can_add_effect = !HasEffect(lab_state, entity, ImmuneToPoisonEffectId);
			break;
		}
		default:
		{
			can_add_effect = true;
		}
	}

	return can_add_effect;
}

static void
func AddEffect(CombatLabState *lab_state, Entity *entity, EffectId effect_id)
{
	Assert(CanAddEffect(lab_state, entity, effect_id));
	Assert(lab_state->effect_n < MaxEffectN);
	Effect *effect = &lab_state->effects[lab_state->effect_n];
	lab_state->effect_n++;

	effect->entity = entity;
	effect->effect_id = effect_id;
	
	if(EffectHasDuration(effect_id))
	{
		R32 duration = GetEffectDuration(effect_id);
		Assert(duration > 0.0f);
		effect->time_remaining = duration;
	}

	Entity *player = &lab_state->entities[0];
	if(entity == player)
	{
		RecalculatePlayerAttributes(lab_state);
	}

	I8 *effect_name = GetEffectName(effect_id);
	CombatLog(lab_state, entity->name + " gets " + effect_name + ".");
}

static void
func ResetOrAddEffect(CombatLabState *lab_state, Entity *entity, EffectId effect_id)
{
	B32 found_effect = false;
	for(I32 i = 0; i < lab_state->effect_n; i++)
	{
		Effect *effect = &lab_state->effects[i];
		if(effect->entity == entity && effect->effect_id == effect_id)
		{

			if(EffectHasDuration(effect_id))
			{
				R32 duration = GetEffectDuration(effect_id);
				Assert(duration > 0.0f);
				effect->time_remaining = duration;
			}
			found_effect = true;
			break;
		}
	}

	if(!found_effect)
	{
		if(CanAddEffect(lab_state, entity, effect_id))
		{
			AddEffect(lab_state, entity, effect_id);
		}
	}
}

static I32
func GetAbilityDamage(Entity *entity, AbilityId ability_id)
{
	I32 damage = 0;
	switch(ability_Id)
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
		case TigerBiteAbilityId:
		{
			damage = 25 + entity->strength;
			break;
		}
	}
	return damage;
}

static void
func UseInstantAbility(CombatLabState *lab_state, Entity *entity, AbilityId ability_id)
{
	Assert(!AbilityIsCasted(ability_id));
	Assert(CanUseAbility(lab_state, entity, ability_id));

	I8 *ability_name = GetAbilityName(ability_id);
	CombatLog(lab_state, entity->name + " uses ability " + ability_name + ".");

	Entity *target = entity->target;
	B32 has_enemy_target = (target != 0 && target->group_id != entity->group_id);
	B32 has_friendly_target = (target != 0 && target->group_id == entity->group_id);
	I32 damage = GetAbilityDamage(entity, ability_id);
	switch(ability_id)
	{
		case EarthShakeAbilityId:
		{
			Assert(has_enemy_target);
			ResetOrAddEffect(lab_state, target, EarthShakeEffectId);
			break;
		}
		case SmallPunchAbilityId:
		{
			Assert(has_enemy_target);
			DealDamageFromEntity(lab_state, entity, target, damage);
			break;
		}
		case EarthShieldAbilityId:
		{
			Assert(has_friendly_target && !IsDead(target));
			ResetOrAddEffect(lab_state, target, EarthShieldEffectId);
			target->absorb_damage += 50;
			break;
		}
		case BigPunchAbilityId:
		{
			Assert(has_enemy_target);
			DealDamageFromEntity(lab_state, entity, target, damage);
			break;
		}
		case KickAbilityId:
		{
			Assert(has_enemy_target);
			R32 move_speed = 20.0f;
			target->velocity = move_speed * GetClosestMoveDirection(target->position - entity->position);
			AddEffect(lab_state, target, KickedEffectId);
			break;
		}
		case SpinningKickAbilityId:
		{
			for(I32 i = 0; i < EntityN; i++)
			{
				Entity *target = &lab_state->entities[i];
				if(target->group_id != entity->group_id)
				{
					R32 distance = Distance(entity->position, target->position);
					if(distance <= MaxMeleeAttackDistance)
					{
						DealDamageFromEntity(lab_state, entity, target, damage);
					}
				}
			}
			break;
		}
		case RollAbilityId:
		{
			R32 move_speed = 20.0f;
			entity->velocity = moveSpeed * GetClosestMoveDirection(entity->input_direction);
			AddEffect(lab_state, entity, RollingEffectId);
			break;
		}
		case AvoidanceAbilityId:
		{
			AddEffect(lab_state, entity, InvulnerableEffectId);
			break;
		}
		case SwordStabAbilityId:
		{
			if(HasEffect(lab_state, entity, BlessingOfTheSunEffectId))
			{
				damage += 2;
				Heal(lab_state, entity, entity, 2);
			}

			Assert(has_enemy_target);
			DealDamageFromEntity(lab_state, entity, target, damage);
			break;
		}
		case SwordSwingAbilityId:
		{
			if(HasEffect(lab_state, entity, BlessingOfTheSunEffectId))
			{
				damage += 2;
				Heal(lab_state, entity, entity, 2);
			}

			for(I32 i = 0; i < EntityN; i++)
			{
				Entity *target = &lab_state->entities[i];
				if(target->group_id != entity->group_id)
				{
					R32 distance = Distance(entity->position, target->position);
					if(distance <= MaxMeleeAttackDistance)
					{
						DealDamageFromEntity(lab_state, entity, target, damage);
					}
				}
			}
			break;
		}
		case RaiseShieldAbilityId:
		{
			AddEffect(lab_state, entity, ShieldRaisedEffectId);
			if(HasEffect(lab_state, entity, BlessingOfTheSunEffectId))
			{
				Heal(lab_state, entity, entity, 10);
			}
			break;
		}
		case BurnAbilityId:
		{
			Assert(hasEnemyTarget);
			AddEffect(lab_state, target, BurningEffectId);
			break;
		}
		case BlessingOfTheSunAbilityId:
		{
			RemoveEffect(lab_state, entity, BlessingOfTheSunEffectId);
			AddEffect(lab_state, entity, BlessingOfTheSunEffectId);
			break;
		}
		case MercyOfTheSunAbilityId:
		{
			Heal(lab_state, entity, entity, 30);
			for(I32 i = 0; i < EntityN; i++)
			{
				Entity* target = &lab_state->entities[i];
				if(!IsDead(target) && target->group_id != entity->group_id)
				{
					R32 distance = Distance(entity->position, target->position);
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
			Assert(has_enemy_target);
			DealDamageFromEntity(lab_state, entity, target, damage);
			if(RandomBetween(0.0f, 1.0f) < 0.3f)
			{
				ResetOrAddEffect(lab_state, target, PoisonedEffectId);
			}
			break;
		}
		case CrocodileBiteAbilityId:
		{
			Assert(has_enemy_target);
			DealDamageFromEntity(lab_state, entity, target, damage);
			ResetOrAddEffect(lab_state, target, BittenEffectId);
			break;
		}
		case CrocodileLashAbilityId:
		{
			Assert(has_enemy_target);
			DealDamageFromEntity(lab_state, entity, target, damage);
			break;
		}
		case TigerBiteAbilityId:
		{
			Assert(has_enemy_target);
			DealDamageFromEntity(lab_state, entity, target, damage);
			ResetOrAddEffect(lab_state, target, BleedingEffectId);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	if(AbilityHasCooldown(ability_id))
	{
		AddAbilityCooldown(lab_state, entity, abilityId);
	}
}

static void
func FinishCasting(CombatLabState *lab_state, Entity *entity)
{
	AbilityId ability_id = entity->casted_ability;
	Assert(ability_id != NoAbilityId);
	Assert(AbilityIsCasted(ability_id));
	Entity *target = entity->cast_target;
	B32 has_enemy_target = (target != 0 && target->group_id != entity->group_id);
	B32 has_friendly_target = (target != 0 && target->group_id == entity->group_id);
	I32 damage = GetAbilityDamage(entity, ability_id);
	switch(ability_id)
	{
		case LightningAbilityId:
		{
			Assert(has_enemy_target);
			DealDamageFromEntity(lab_state, entity, target, damage);
			break;
		}
		case HealAbilityId:
		{
			Assert(has_friendly_target);
			AttemptToHeal(lab_state, entity, target, 30);
			break;
		}
		case LightOfTheSunAbilityId:
		{
			AttemptToHeal(lab_state, entity, entity, 20);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	if(AbilityHasCooldown(ability_id))
	{
		AddAbilityCooldown(lab_state, entity, ability_id);
	}

	entity->casted_ability = NoAbilityId;
	entity->cast_target = 0;
	entity->cast_time_total = 0.0f;
	entity->cast_time_remaining = 0.0f;
}

static void
func UseAbility(CombatLabState *lab_state, Entity *entity, AbilityId ability_id)
{
	if(AbilityIsCasted(ability_id))
	{
		Assert(CanUseAbility(lab_state, entity, ability_id));
		R32 cast_duration = GetAbilityCastDuration(ability_id);
		Assert(cast_durection > 0.0f);
		entity->casted_ability = ability_id;
		entity->cast_time_total = cast_duration;
		entity->cast_time_remaining = cast_duration;
		entity->cast_target = entity->target;
	}
	else
	{
		UseInstantAbility(lab_state, entity, ability_id);
	}

	R32 recharge_duration = GetAbilityRechargeDuration(ability_id);
	if(recharge_duration > 0.0f)
	{
		PutOnRecharge(entity, recharge_duration);
	}
}

static void
func AttemptToUseAbility(CombatLabState *lab_state, Entity *entity, AbilityId ability_id)
{
	if(CanUseAbility(lab_state, entity, ability_id))
	{
		UseAbility(lab_state, entity, ability_id);
	}
}

static AbilityId
func GetAvailableClassAbilityAtIndex(Entity *entity, I32 ability_index)
{
	AbilityId result = NoAbilityId;
	I32 class_ability_index = 0;
	for(I32 id = 1; id < AbilityN; id++)
	{
		AbilityId ability_id = (AbilityId)id;
		I32 ability_level = GetAbilityMinLevel(ability_id);
		I32 ability_class_id = GetAbilityClass(ability_id);
		if(entity->level >= ability_level && entity->class_id == ability_class_id)
		{
			if(class_ability_index == ability_index)
			{
				result = ability_id;
				break;
			}
			class_ability_index++;
		}
	}
	return result;
}

static void
func AttemptToUseAbilityAtIndex(CombatLabState *lab_state, Entity *entity, I32 ability_index)
{
	AbilityId ability_id = GetAvailableClassAbilityAtIndex(entity, ability_index);
	if(ability_id != NoAbilityId)
	{
		AttemptToUseAbility(lab_state, entity, ability_id);
	}
}

static void
func DrawEntity(Canvas *canvas, Entity *entity, V4 color)
{
	V4 entity_outline_color = MakeColor(0.0f, 0.0f, 0.0f);
	DrawCircle(canvas, entity->position, EntityRadius, color);
	DrawCircleOutline(canvas, entity->position, EntityRadius, entity_outline_color);
}

static void
func DrawSelectedEntity(Canvas *canvas, Entity *entity, V4 color)
{
	V4 selection_outline_color = MakeColor(1.0f, 1.0f, 1.0f);
	DrawCircle(canvas, entity->position, EntityRadius, color);
	DrawCircleOutline(canvas, entity->position, EntityRadius, selection_outline_color);
}

static void
func DrawEntityBars(Canvas *canvas, Entity *entity)
{
	V2 health_bar_center = entity->position + MakeVector(0.0f, -1.5f * EntityRadius);
	R32 health_bar_width = 2.0f * EntityRadius;
	R32 health_bar_height = 0.5f * EntityRadius;

	V4 health_bar_background_color = MakeColor(0.5f, 0.5f, 0.5f);
	V4 health_bar_filled_color = MakeColor(1.0f, 0.0f, 0.0f);
	V4 health_bar_outline_color = MakeColor(0.0f, 0.0f, 0.0f);

	R32 max_health = (R32)GetEntityMaxHealth(entity);
	Assert(max_health > 0.0f);
	R32 health_ratio = R32(entity->health) / max_health;
	Assert(IsBetween(health_ratio, 0.0f, 1.0f));

	Rect health_bar_background_rect = MakeRect(health_bar_center, health_bar_width, health_bar_height);
	DrawRect(canvas, health_bar_background_rect, health_bar_background_color);

	Rect health_bar_filled_rect = health_bar_background_rect;
	health_bar_filled_rect.right = Lerp(health_bar_background_rect.left, health_ratio, health_bar_background_rect.right);
	DrawRect(canvas, health_bar_filled_rect, health_bar_filled_color);

	if(entity->absorb_damage > 0)
	{
		V4 absorb_damage_background_color = MakeColor(1.0f, 1.0f, 1.0f);
		Rect absorb_damage_rect = health_bar_background_rect;
		absorb_damage_rect.left = health_bar_filled_rect.right;
		R32 max_health = (R32)GetEntityMaxHealth(entity);
		Assert(max_health > 0.0f);
		R32 absorb_damage_width = R32(entity->absorb_damage) / max_health * health_bar_width;
		absorb_damage_rect.right = Min2(health_bar_background_rect.right, absorb_damage_rect.left + absorb_damage_width);
		DrawRect(canvas, absorb_damage_rect, absorb_damage_background_color);
	}

	DrawRectOutline(canvas, health_bar_background_rect, health_bar_outline_color);

	if(entity->casted_ability != NoAbilityId)
	{
		R32 cast_bar_width = health_bar_width;
		R32 cast_bar_height = 0.3f * EntityRadius;
		V2 cast_bar_center = health_bar_center + MakeVector(0.0f, -health_bar_height * 0.5f - cast_bar_height * 0.5f);

		V4 cast_bar_background_color = MakeColor(0.0, 0.5f, 0.5f);
		V4 cast_bar_filled_color = MakeColor(0.0f, 1.0f, 1.0f);
		V4 cast_bar_outline_color = MakeColor(0.0f, 0.0f, 0.0f);

		Rect cast_bar_background_rect = MakeRect(cast_bar_center, cast_bar_width, cast_bar_height);
		DrawRect(canvas, cast_bar_background_rect, cast_bar_background_color);

		Assert(entity->cast_time_total > 0.0f);
		R32 cast_ratio = 1.0f - (entity->cast_time_remaining / entity->cast_time_total);
		Assert(IsBetween(cast_ratio, 0.0f, 1.0f));

		Rect cast_bar_filled_rect = cast_bar_background_rect;
		cast_bar_filled_rect.right = Lerp(cast_bar_background_rect.left, castRatio, cast_bar_background_rect.right);
		DrawRect(canvas, cast_bar_filled_rect, cast_bar_filled_color);

		DrawRectOutline(canvas, cast_bar_background_rect, cast_bar_outline_color);
	}
}

static String
func GetAbilityTooltipText(AbilityId ability_id, Entity *entity, I8 *buffer, I32 buffer_size)
{
	String text = StartString(buffer, buffer_size);

	I8 *ability_name = GetAbilityName(ability_id);
	AddLine(text, ability_name);

	R32 cast_duration = GetAbilityCastDuration(ability_id);
	if(cast_duration > 0.0f)
	{
		if(cast_duration == 1.0f)
		{
			AddLine(text, "Cast: 1 second");
		}
		else
		{
			AddLine(text, "Cast: " + cast_duration + " seconds");
		}
	}
	else
	{
		AddLine(text, "Instant");
	}

	R32 recharge_duration = GetAbilityRechargeDuration(ability_id);
	if(recharge_duration == 1.0f)
	{
		AddLine(text, "Recharge: 1 second");
	}
	else if(recharge_duration > 0.0f)
	{
		AddLine(text, "Recharge: " + recharge_duration + " seconds");
	}

	if(AbilityHasCooldown(ability_id))
	{
		R32 cooldown_duration = GetAbilityCooldownDuration(ability_id);
		Assert(cooldown_duration > 0.0f);
		if(cooldown_duration == 1.0f)
		{
			AddLine(text, "Cooldown: 1 second");
		}
		else
		{
			AddLine(text, "Cooldown: " + cooldown_duration + " seconds");
		}
	}

	I32 damage = GetAbilityDamage(entity, ability_id);

	switch(ability_id)
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

static V4
func GetFlowerColor(ItemId item_id)
{
	V4 color = {};
	switch(item_id)
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
func UpdateAndDrawFlowers(Canvas *canvas, CombatLabState *lab_state, V2 mouse_position)
{
	GlyphData *glyph_data = canvas->glyph_data;

	R32 radius = 0.5f;

	V4 text_color = MakeColor(1.0f, 1.0f, 1.0f);
	V4 text_backgroud_color = MakeColor(0.5f, 0.5f, 0.5f);
	V4 text_hover_background_color = MakeColor(0.7f, 0.5f, 0.5f);

	labState->hover_flower = 0;

	for(I32 i = 0; i < lab_state->flower_n; i++)
	{
		Flower *flower = &lab_state->flowers[i];
		V4 color = GetFlowerColor(flower->item_id);
		DrawCircle(canvas, flower->position, radius, color);

		I8 *text = GetVisibleItemName(lab_state, flower->item_id);

		R32 text_width = GetTextWidth(canvas, text);
		R32 text_height = GetTextHeight(canvas, text);

		R32 text_bottom = flower->position.y - radius;
		R32 text_center_x = flower->position.x;
		
		V2 text_bottom_center = MakePoint(text_center_x, text_bottom);
		Rect text_background_rect = MakeRectBottom(text_bottom_center, text_width, text_height);

		B32 is_hover = IsPointInRect(mouse_position, text_background_rect);
		V4 background_color = (is_hover) ? text_hover_background_color : text_background_color;

		if(is_hover)
		{
			lab_state->hover_flower = flower;
		}

		DrawRect(canvas, text_background_rect, background_color);
		DrawTextLineBottomXCentered(canvas, text, text_bottom, text_center_x, text_color);
	}
}

#define UIBoxSide 40
#define UIBoxPadding 5

static void
func DrawUIBox(Bitmap *bitmap, I32 left, I32 right, I32 top, I32 bottom, R32 recharge_ratio, V4 color)
{
	V4 outline_color = MakeColor(1.0f, 1.0f, 1.0f);
	V4 recharge_color = MakeColor(0.5f, 0.5f, 0.5f);

	DrawBitmapRect(bitmap, left, right, top, bottom, color);

	Assert(IsBetween(recharge_ratio, 0.0f, 1.0f));
	I32 recharge_right = (I32)Lerp((R32)left, recharge_ratio, (R32)right);
	DrawBitmapRect(bitmap, left, recharge_right, top, bottom, recharge_color);

	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outline_color);
}

static void
func DrawUIBoxWithText(Bitmap *bitmap, I32 left, I32 right, I32 top, I32 bottom, R32 recharge_ratio,
					   I8 *text, GlyphData *glyph_data, V4 color)
{
	DrawUIBox(bitmap, left, right, top, bottom, recharge_ratio, color);
	V4 text_color = MakeColor(1.0f, 1.0f, 1.0f);
	Assert(text != 0 && glyphData != 0);
	DrawBitmapTextLineCentered(bitmap, text, glyph_data, left, right, top, bottom, text_color);
}

static void
func DrawEffectUIBox(Bitmap *bitmap, Effect *effect, I32 top, I32 left)
{
	I32 right = left + UIBoxSide;
	I32 bottom = top + UIBoxSide;

	R32 duration_ratio = 0.0f;
	if(EffectHasDuration(effect->effect_id))
	{
		R32 total_duration = GetEffectDuration(effect->effect_id);
		Assert(total_duration > 0.0f);

		duration_ratio = (effect->time_remaining / total_duration);
	}
	else
	{
		duration_ratio = 1.0f;
	}

	Assert(IsBetween(duration_ratio, 0.0f, 1.0f));

	V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	DrawUIBox(bitmap, left, right, top, bottom, duration_ratio, background_color);
}

static void
func DrawPlayerEffectBar(Canvas *canvas, CombatLabState *lab_state)
{
	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;
	Assert(glyph_data != 0);

	Entity* player = &lab_state->entities[0];
	Assert(player->group_id == PlayerGroupId);

	V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);

	I32 left = UIBoxPadding;
	I32 top = UIBoxPadding;

	for(I32 i = 0; i < lab_state->effect_n; i++)
	{
		Effect *effect = &lab_state->effects[i];
		if(effect->entity == player)
		{
			DrawEffectUIBox(bitmap, effect, top, left);
			left += UIBoxSide + UIBoxPadding;
		}
	}
}

static void
func DrawTargetEffectBar(Canvas *canvas, CombatLabState *lab_state)
{
	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;
	Assert(glyph_data != 0);

	Entity *player = &lab_state->entities[0];
	Assert(player->group_id == PlayerGroupId);

	Entity *target = player->target;

	if(target)
	{
		V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);

		I32 left = (bitmap->width - 1) - UIBoxPadding - UIBoxSide;
		I32 top = UIBoxPadding;

		for(I32 i = 0; i < lab_state->effect_n; i++)
		{
			Effect *effect = &lab_state->effects[i];
			if(effect->entity == target)
			{
				DrawEffectUIBox(bitmap, effect, top, left);
				left -= UIBoxSide + UIBoxPadding;
			}
		}
	}
}

static void
func DrawHelpBar(Canvas *canvas, CombatLabState *lab_state, IV2 mouse_position)
{
	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;
	Assert(glyph_data != 0);

	I32 right  = (bitmap->width - 1) - UIBoxPadding;
	I32 left   = right - UIBoxSide;
	I32 bottom = (bitmap->height - 1) - UIBoxPadding;
	I32 top    = bottom - UIBoxSide;

	V4 box_color = MakeColor(0.0f, 0.0f, 0.0f);

	I8 text_buffer[8] = {};
	OneLineString(text_buffer, 8, "Help");

	DrawUIBoxWithText(bitmap, left, right, top, bottom, 0.0f, text_buffer, glyph_data, box_color);

	if(IsIntBetween(mouse_position.col, left, right) && IsIntBetween(mouse_position.row, top, bottom))
	{
		I8 tooltip_buffer[256] = {};
		String tooltip = StartString(tooltip_buffer, 256);
		AddLine(tooltip, "Key binds");
		AddLine(tooltip, "[W][A][S][D] or Arrows - Move");
		AddLine(tooltip, "[Tab] - Target closest enemy");
		AddLine(tooltip, "[F1] - Target player");
		AddLine(tooltip, "[1]-[2] - Use Ability");
		AddLine(tooltip, "[V] - Show/hide inventory");
		AddLine(tooltip, "[C] - Show/hide character info");

		I32 tooltip_bottom = top - UIBoxPadding;
		I32 tooltip_right = (bitmap->width - 1);
		DrawBitmapStringTooltipBottomRight(bitmap, tooltip, glyph_data, tooltip_bottom, tooltip_right);
	}
}

static void
func DrawAbilityBar(Canvas *canvas, CombatLabState *lab_state, IV2 mouse_position)
{
	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;
	Assert(glyph_data != 0);

	Entity *entity = &lab_state->entities[0];
	Assert(entity->group_id == PlayerGroupId);
	ClassId class_id = entity->class_id;
	Assert(class_id != NoClassId);

	I32 ability_n = 0;
	for(I32 id = 1; id < AbilityN; id++)
	{
		AbilityId ability_id = (AbilityId)id;
		ClassId ability_class_id = GetAbilityClass(ability_id);
		I32 ability_min_level = GetAbilityMinLevel(ability_id);
		if(entity->level >= ability_min_level && entity->classId == ability_class_id)
		{
			ability_n++;
		}
	}

	I32 center_x = (bitmap->width - 1) / 2;
	I32 width = UIBoxSide * ability_n + UIBoxPadding * (ability_n - 1);

	I32 left   = center_x - width / 2;
	I32 right  = center_x + width / 2;
	I32 bottom = (bitmap->height - 1) - 30;
	I32 top    = bottom - UIBoxSide;

	I32 box_left = left;
	I32 ability_index = 0;

	lab_state->hover_ability_id = NoAbilityId;

	for(I32 id = 1; id < AbilityN; id++)
	{
		AbilityId ability_id = (AbilityId)id;
		ClassId ability_class_id = GetAbilityClass(ability_id);
		I32 ability_min_level = GetAbilityMinLevel(ability_id);
		if(entity->level >= ability_min_level && entity->class_id == ability_class_id)
		{
			I32 box_right = box_left + UIBoxSide;
			I32 box_top = top;
			I32 box_bottom = bottom;

			V4 box_background_color = MakeColor(0.0f, 0.0f, 0.0f);
			V4 box_cannot_use_color = MakeColor(0.2f, 0.0f, 0.0f);

			V4 color = boxBackgroundColor;

			R32 recharge = 0.0f;
			R32 recharge_from = 0.0f;
			AbilityCooldown *cooldown = GetAbilityCooldown(lab_state, entity, ability_id);
			if(cooldown)
			{
				recharge = cooldown->time_remaining;
				recharge_from = GetAbilityCooldownDuration(abilityId);
			}
			else if(entity->recharge > 0.0f)
			{
				recharge = entity->recharge;
				recharge_from = entity->recharge_from;
			}

			if(!AbilityIsEnabled(lab_state, entity, ability_id))
			{
				color = box_cannot_use_color;
			}

			R32 recharge_ratio = (recharge_from > 0.0f) ? (recharge / recharge_from) : 0.0f;

			I8 name[8] = {};
			OneLineString(name, 8, (ability_index + 1));

			DrawUIBoxWithText(bitmap, box_left, box_right, box_top, box_bottom, recharge_ratio, name, glyph_data, color);

			if(IsIntBetween(mouse_position.col, box_left, box_right) && IsIntBetween(mousePosition.row, box_top, box_bottom))
			{
				I32 tooltip_left = (box_left + box_right) / 2 - TooltipWidth / 2;
				I32 tooltip_bottom = box_top - 5;

				static I8 tooltip_buffer[512] = {};
				String tooltip_text = GetAbilityTooltipText(ability_id, entity, tooltip_buffer, 512);
				DrawBitmapStringTooltipBottom(bitmap, tooltip_text, glyph_data, tooltip_bottom, tooltip_left);

				lab_state->hover_ability_id = ability_id;
			}

			box_left = box_right + UIBoxPadding;
			ability_index++;
		}
	}
}

static void
func UpdateEntityRecharge(Entity *entity, R32 seconds)
{
	entity->recharge = Max2(entity->recharge - seconds, 0.0f);
}

static void
func UpdateAbilityCooldowns(CombatLabState *lab_state, R32 seconds)
{
	I32 remaining_n = 0;
	for(I32 i = 0; i < lab_state->ability_cooldown_n; i++)
	{
		AbilityCooldown *cooldown = &lab_state->ability_cooldowns[i];
		cooldown->time_remaining -= seconds;
		if(cooldown->time_remaining > 0.0f)
		{
			lab_state->ability_cooldowns[remaining_n] = *cooldown;
			remaining_n++;
		}
	}

	lab_state->ability_cooldown_n = remaining_n;
}

static void
func UpdateItemCooldowns(CombatLabState *lab_state, R32 seconds)
{
	I32 remaining_n = 0;
	for(I32 i = 0; i < lab_state->item_cooldown_n; i++)
	{
		ItemCooldown *cooldown = &lab_state->item_cooldowns[i];
		cooldown->time_remaining -= seconds;
		if(cooldown->time_remaining > 0.0f)
		{
			lab_state->item_cooldowns[remaining_n] = *cooldown;
			remaining_n++;
		}
	}

	lab_state->item_cooldown_n = remaining_n;
}

static void
func DrawCombatLog(Canvas *canvas, CombatLabState *lab_state)
{
	Bitmap *bitmap = &canvas->bitmap;
	I32 width  = 300;
	I32 height = 200;

	I32 left   = UIBoxPadding;
	I32 right  = left + width;
	I32 bottom = (bitmap->height - 1) - UIBoxPadding;
	I32 top    = bottom - height;

	V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	V4 outline_color = MakeColor(0.5f, 0.5f, 0.5f);
	V4 text_color = MakeColor(1.0f, 1.0f, 1.0f);

	DrawBitmapRect(bitmap, left, right, top, bottom, backgrond_color);
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outline_color);

	I32 text_left   = left + UIBoxPadding;
	I32 text_right  = right - UIBoxPadding;
	I32 text_top    = top + UIBoxPadding;
	I32 text_bottom = bottom + UIBoxPadding;

	I32 text_height = text_bottom - text_top;

	I32 last_index = lab_state->combat_log_last_line_index;
	I32 first_index = GetNextCombatLogLineIndex(last_index);

	I32 log_height = lab_state->combat_log_line_n * TextHeightInPixels;
	if(log_height > text_height)
	{
		first_index = last_index - (text_height / TextHeightInPixels - 1);
		if(first_index < 0)
		{
			first_index += MaxCombatLogLineN;
		}
		Assert(IsIntBetween(first_index, 0, MaxCombatLogLineN + 1));
	}

	I32 index = first_index;
	while(1)
	{
		I8 *text = lab_state->combat_log_lines[index];
		if(text[0] != 0)
		{
			DrawBitmapTextLineTopLeft(bitmap, text, canvas->glyph_data, text_left, text_top, text_color);
			text_top += TextHeightInPixels;
		}

		if(index == last_index)
		{
			break;
		}
		index = GetNextCombatLogLineIndex(index);
	}
}

static I32
GetHateTableEntityEntryN(HateTable *htae_table, Entity* source)
{
	Assert(source != 0);
	I32 entry_n = 0;
	for(I32 i = 0; i < hate_table->entry_n; i++)
	{
		HateTableEntry *entry = &hate_table->entries[i];
		if(entry->source == source)
		{
			entry_n++;
		}
	}

	return entry_n;
}

static void
func RemoveDeadEntitiesFromHateTable(HateTable *hate_table)
{
	I32 remaining_entry_n = 0;
	for(I32 i = 0; i < hate_table->entry_n; i++)
	{
		HateTableEntry *entry = &hate_table->entries[i];
		if(!IsDead(entry->source) && !IsDead(entry->target))
		{
			hate_table->entries[remaining_entry_n] = *entry;
			remaining_entry_n++;
		}
	}

	hate_table->entry_n = remaining_entry_n;
}

static void
func SortHateTable(HateTable *hate_table)
{
	for(I32 index = 0; index < hate_table->entry_n; index++)
	{
		HateTableEntry entry = hate_table->entries[index];
		for(I32 previous_index = index - 1; previous_index >= 0; previous_index--)
		{
			HateTableEntry previous_entry = hate_table->entries[previous_index];
			B32 need_swap = false;
			if(previous_entry.source > entry.source)
			{
				need_swap = true;
			}
			else if(previous_entry.source == entry.source && previous_entry.value < entry.value)
			{
				need_swap = true;
			}

			if(need_swap)
			{
				hate_table->entries[previous_index] = entry;
				hate_table->entries[previous_index + 1] = previous_entry;
			}
			else
			{
				break;
			}
		}
	}
}

static void
func UpdateEnemyTargets(CombatLabState *lab_state)
{
	HateTable *hate_table = &lab_state->hate_table;
	HateTableEntry *previous_entry = 0;
	for(I32 i = 0; i < hate_table->entry_n; i++)
	{
		HateTableEntry *entry = &hate_table->entries[i];
		if(entry->source->group_id == EnemyGroupId)
		{
			if(previous_entry != 0)
			{
				Assert(previous_entry->source <= entry->source);
				if(previous_entry->source != entry->source)
				{
					entry->source->target = entry->target;
				}
				else
				{
					Assert(previous_entry->value >= entry->value);
				}
			}
			else
			{
				entry->source->target = entry->target;
			}
		}

		previous_entry = entry;
	}
}

static ItemCooldown *
func GetItemCooldown(CombatLabState *lab_state, Entity *entity, ItemId item_id)
{
	item_id = GetItemIdForCooldown(item_id);
	Assert(ItemHasOwnCooldown(item_id));

	ItemCooldown *result = 0;
	for(I32 i = 0; i < lab_state->item_cooldown_n; i++)
	{
		ItemCooldown *cooldown = &lab_state->item_cooldowns[i];
		if(cooldown->entity == entity && cooldown->item_id == item_id)
		{
			result = cooldown;
			break;
		}
	}
	return result;
}

static B32
func CanSwapItems(InventoryItem *item1, InventoryItem *item2)
{
	Assert(InventoryItemIsValid(item1));
	Assert(InventoryItemIsValid(item2));

	B32 can_put_item1 = ItemGoesIntoSlot(item1->item_id, item2->slot_id);
	B32 can_put_item2 = ItemGoesIntoSlot(item2->item_id, item1->slot_id);
	B32 can_swap = (can_put_item1 && can_put_item2);
	return can_swap;
}

static void
func SwapItems(InventoryItem *item1, InventoryItem *item2)
{
	Assert(CanSwapItems(item1, item2));

	SetSlotItemId(item1->inventory, item1->slot, item2->item_id);
	SetSlotItemId(item2->inventory, item2->slot, item1->item_id);
}

static void
func AttemptToSwapItems(InventoryItem *item1, InventoryItem *item2)
{
	if(CanSwapItems(item1, item2))
	{
		SwapItems(item1, item2);
	}
}

#define InventorySlotSide 50

static void
func DrawInventorySlot(Canvas *canvas, CombatLabState *lab_state, SlotId slotId, ItemId item_id, I32 top, I32 left)
{
	I32 bottom = top + InventorySlotSide;
	I32 right  = left + InventorySlotSide;

	V4 slot_background_color = MakeColor(0.0f, 0.0f, 0.0f);
	V4 item_background_color = MakeColor(0.1f, 0.1f, 0.1f);
	V4 cooldown_color = MakeColor(0.2f, 0.0f, 0.0f);
	V4 item_name_color = MakeColor(1.0f, 1.0f, 1.0f);
	V4 slot_name_color = MakeColor(0.3f, 0.3f, 0.3f);

	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;

	if(item_id == NoItemId)
	{
		DrawBitmapRect(bitmap, left, right, top, bottom, slot_background_color);
		if(slot_id != AnySlotId)
		{
			I8 *slot_name = GetSlotName(slot_id);
			Assert(slot_name != 0);
			DrawBitmapTextLineCentered(bitmap, slot_name, glyph_data, left, right, top, bottom, slot_name_color);
		}
	}
	else
	{
		DrawBitmapRect(bitmap, left, right, top, bottom, item_background_color);

		R32 total_cooldown = GetItemCooldownDuration(item_id);
		if(total_cooldown > 0.0f)
		{
			Entity *player = &lab_state->entities[0];
			ItemCooldown* cooldown = GetItemCooldown(lab_state, player, itemId);
			if(cooldown)
			{
				R32 remaining_cooldown = cooldown->time_remaining;
				R32 cooldown_ratio = (remaining_cooldown / total_cooldown);
				Assert(IsBetween(cooldown_ratio, 0.0f, 1.0f));

				I32 cooldown_right = (I32)Lerp((R32)left, cooldown_ratio, (R32)right);
				DrawBitmapRect(bitmap, left, cooldown_right, top, bottom, cooldown_color);
			}
		}

		I8 *name = GetItemSlotName(item_id);
		DrawBitmapTextLineCentered(bitmap, name, glyph_data, left, right, top, bottom, item_name_color);
	}
}

static void
func DrawInventorySlotOutline(Canvas *canvas, I32 top, I32 left, V4 color)
{
	Bitmap *bitmap = &canvas->bitmap;
	I32 bottom = top + InventorySlotSide;
	I32 right = left + InventorySlotSide;
	DrawBitmapRectOutline(bitmap, left, right, top, bottom, color);
}

#define InventorySlotPadding 2

static I32
func GetInventoryWidth(Inventory *inventory)
{
	I32 width = InventorySlotPadding + inventory->colN * (InventorySlotSide + InventorySlotPadding);
	return width;
}

static I32
func GetInventoryHeight(Inventory *inventory)
{
	I32 height = InventorySlotPadding + inventory->rowN * (InventorySlotSide + InventorySlotPadding);
	return height;
}

static void
func UpdateAndDrawInventory(Canvas *canvas, CombatLabState *lab_state, Inventory *inventory, IV2 mouse_position)
{
	Bitmap *bitmap = &canvas->bitmap;
	GlyphData *glyph_data = canvas->glyph_data;
	Assert(glyph_data != 0);

	I32 slot_padding = 2;

	I32 width = GetInventoryWidth(inventory);
	I32 height = GetInventoryHeight(inventory);

	I32 left   = inventory->left;
	I32 right  = left + width;
	I32 top    = inventory->top;
	I32 bottom = top + height;

	V4 background_color = MakeColor(0.5f, 0.5f, 0.5f);
	V4 hover_outline_color = MakeColor(1.0f, 1.0f, 0.0f);
	V4 invalid_outline_color = MakeColor(1.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, left, right, top, bottom, background_color);

	InventoryItem *drag_item = &lab_state->drag_item;
	InventoryItem *hover_item = &lab_state->hover_item;

	I32 slot_top = top + slot_padding;
	for(I32 row = 0; row < inventory->row_n; row++)
	{
		I32 slot_left = left + slot_padding;
		for(I32 col = 0; col < inventory->col_n; col++)
		{
			SlotId slot_id = GetInventorySlotId(inventory, row, col);
			ItemId item_id = GetInventoryItemId(inventory, row, col);
			
			ItemId visible_item_id = (item_id == NoItemId) ? NoItemId : GetVisibleItemId(lab_state, item_id);

			B32 is_hover = (IsIntBetween(mouse_position.col, slot_left, slot_left + InventorySlotSide) &&
							   IsIntBetween(mouse_position.row, slot_top, slot_top + InventorySlotSide));
			B32 is_drag = (drag_item->inventory == inventory && 
							  drag_item->slot.row == row && drag_item->slot.col == col);

			if(!is_drag)
			{
				DrawInventorySlot(canvas, lab_state, slot_id, visible_item_id, slot_top, slot_left);
			}
			else
			{
				DrawInventorySlot(canvas, lab_state, slot_id, NoItemId, slot_top, slot_left);
			}

			if(is_hover)
			{
				if(!is_drag && item_id != NoItemId)
				{
					I32 tooltip_bottom = slot_top - UIBoxPadding;
					I32 tooltip_right = IntMin2((slot_left + InventorySlotSide / 2) + TooltipWidth / 2,
												  (bitmap->width - 1) - UIBoxPadding);

					tooltip_right = IntMax2(tooltipRight, UIBoxPadding + TooltipWidth);

					I8 tooltip_buffer[128];
					String tooltip = GetItemTooltipText(visible_item_id, tooltip_buffer, 128);
					DrawBitmapStringTooltipBottomRight(bitmap, tooltip, glyph_data, tooltip_bottom, tooltip_right);
				}

				V4 outline_color = hover_outline_color;
				if(drag_item->inventory && !ItemGoesIntoSlot(drag_item->item_id, slot_id))
				{
					outline_color = invalid_outline_color;
				}

				DrawInventorySlotOutline(canvas, slot_top, slot_left, outline_color);
				hoverItem->inventory = inventory;
				hoverItem->slot_id = slot_id;
				hoverItem->item_id = item_id;
				hoverItem->slot.row = row;
				hoverItem->slot.col = col;
			}

			slot_left += (InventorySlotSide + slot_padding);
		}

		slot_top += (InventorySlotSide + slot_padding);
	}
}

static void
func DrawCharacterInfo(Canvas *canvas, Entity *entity, CombatLabState *lab_state, IV2 mouse_position)
{
	Bitmap *bitmap = &canvas->bitmap;
	
	V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	V4 outline_color = MakeColor(1.0f, 1.0f, 1.0f);

	Inventory *inventory = &lab_state->equip_inventory;
	I32 width = GetInventoryWidth(inventory);
	I32 height = UIBoxPadding + 14 * TextHeightInPixels + UIBoxPadding;

	I32 left   = UIBoxPadding;
	I32 right  = left + width;
	I32 bottom = (bitmap->height - 1) - UIBoxPadding;
	I32 top    = bottom - height;

	inventory->left = right - GetInventoryWidth(inventory);
	inventory->top = top - GetInventoryHeight(inventory);
	UpdateAndDrawInventory(canvas, labState, inventory, mouse_position);

	DrawBitmapRect(bitmap, left, right, top, bottom, background_color);

	GlyphData *glyph_data = canvas->glyph_data;
	Assert(glyph_data != 0);

	V4 text_color  = MakeColor(1.0f, 1.0f, 1.0f);
	V4 title_color = MakeColor(1.0f, 1.0f, 0.0f);
	V4 info_color  = MakeColor(0.3f, 0.3f, 0.3f);

	I32 text_left = left + UIBoxPadding;
	I32 text_top = top + UIBoxPadding;

	DrawBitmapTextLineTopLeft(bitmap, "Character Info", glyph_data, text_left, text_top, title_color);
	text_top += TextHeightInPixels;

	I8 line_buffer[128] = {};
	OneLineString(line_buffer, 128, "Level " + entity->level);
	DrawBitmapTextLineTopLeft(bitmap, line_buffer, glyph_data, text_left, text_top, text_color);
	text_top += TextHeightInPixels;

	text_top += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Attributes", glyph_data, text_left, text_top, title_color);
	text_top += TextHeightInPixels;

	OneLineString(line_buffer, 128, "Constitution: " + entity->constitution);
	DrawBitmapTextLineTopLeft(bitmap, line_buffer, glyph_data, text_left, text_top, text_color);
	text_top += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Increases maximum health.",
							  glyph_data, text_left, text_top, info_color);
	text_top += TextHeightInPixels;

	OneLineString(line_buffer, 128, "Strength: " + entity->strength);
	DrawBitmapTextLineTopLeft(bitmap, line_buffer, glyph_data, text_left, text_top, text_color);
	text_top += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Increases damage of physical attacks.",
							  glyph_data, text_left, text_top, info_color);
	text_top += TextHeightInPixels;

	OneLineString(line_buffer, 128, "Intellect: " + entity->intellect);
	DrawBitmapTextLineTopLeft(bitmap, line_buffer, glyph_data, text_left, text_top, text_color);
	text_top += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Increases damage of magical attacks.",
							  glyph_data, text_left, text_top, info_color);
	text_top += TextHeightInPixels;

	OneLineString(line_buffer, 128, "Dexterity: " + entity->dexterity);
	DrawBitmapTextLineTopLeft(bitmap, line_buffer, glyph_data, text_left, text_top, text_color);
	text_top += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Reduces cast time of abilities.",
							  glyph_data, text_left, text_top, info_color);
	text_top += TextHeightInPixels;

	DrawBitmapTextLineTopLeft(bitmap, "Professions", glyph_data, text_left, text_top, title_color);
	text_top += TextHeightInPixels;

	OneLineString(line_buffer, 128, "Herbalism: " + entity->herbalism);
	DrawBitmapTextLineTopLeft(bitmap, line_buffer, glyph_data, text_left, text_top, text_color);
	text_top += TextHeightInPixels;

	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outline_color);
}

static void
func DrawHateTable(Canvas *canvas, CombatLabState *lab_state)
{
	Bitmap *bitmap = &canvas->bitmap;
	
	Entity *player = &lab_state->entities[0];
	Entity *target = player->target;

	HateTable *hate_table = &lab_state->hate_table;

	if(target != 0 && target->group_id != player->group_id)
	{
		I32 line_n = GetHateTableEntityEntryN(hate_table, target);
		if(line_n > 0)
		{
			I32 width = 200;
			I32 height = UIBoxPadding + TextHeightInPixels + line_n * TextHeightInPixels + UIBoxPadding;
			V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
			V4 outline_color = MakeColor(0.5f, 0.5f, 0.5f);
			V4 title_color = MakeColor(1.0f, 1.0f, 0.0f);
			V4 text_color = MakeColor(1.0f, 1.0f, 1.0f);

			I32 right  = (bitmap->width - 1) - UIBoxPadding - UIBoxSide - UIBoxPadding;
			I32 left   = right - width;
			I32 bottom = (bitmap->height - 1) - UIBoxPadding;
			I32 top    = bottom - height;

			DrawBitmapRect(bitmap, left, right, top, bottom, background_color);
			DrawBitmapRectOutline(bitmap, left, right, top, bottom, outline_color);

			I32 text_left = left + UIBoxPadding;
			I32 text_right = right - UIBoxPadding;
			I32 text_top  = top + UIBoxPadding;

			DrawBitmapTextLineTopLeft(bitmap, "Hate", canvas->glyph_data, text_left, text_top, title_color);
			text_top += TextHeightInPixels;

			for(I32 i = 0; i < hate_table->entryN; i++)
			{
				HateTableEntry* entry = &hate_table->entries[i];
				if(entry->source == target)
				{
					I8 *name = entry->target->name;
					I8 value[8];
					OneLineString(value, 8, entry->value);
					DrawBitmapTextLineTopLeft(bitmap, name, canvas->glyph_data, text_left, text_top, text_color);
					DrawBitmapTextLineTopRight(bitmap, value, canvas->glyph_data, text_right, text_top, text_color);
					text_top += TextHeightInPixels;
				}
			}
		}
	}
}

static void
func UpdateEffects(CombatLabState *lab_state, R32 seconds)
{
	I32 remaining_effect_n = 0;
	for(I32 i = 0; i < lab_state->effect_n; i++)
	{
		Effect *effect = &lab_state->effects[i];
		effect->time_remaining -= seconds;
		if(!EffectHasDuration(effect->effect_id) || effect->time_remaining > 0.0f)
		{
			lab_state->effects[remaining_effect_n] = *effect;
			remaining_effect_n++;
		}
		else if(effect->effect_id == EarthShieldEffectId)
		{
			effect->entity->absorb_damage = 0;
		}
	}
	labState->effect_n = remaining_effect_n;

	for(I32 i = 0; i < lab_state->effect_n; i++)
	{
		Effect *effect = &lab_state->effects[i];
		R32 time = effect->time_remaining;
		R32 previous_time = time + seconds;
		EffectId effect_id = effect->effect_id;
		switch(effect_id)
		{
			case BurningEffectId:
			{
				if(Floor(time) != Floor(previous_time))
				{
					DealDamageFromEffect(lab_state, effect->effect_id, effect->entity, 2);
				}
				break;
			}
			case PoisonedEffectId:
			{
				if(Floor(time * (1.0f / 3.0f)) != Floor(previous_time * (1.0f / 3.0f)))
				{
					DealDamageFromEffect(lab_state, effect->effect_id, effect->entity, 2);
				}
				break;
			}
			case RegenerateEffectId:
			{
				if(Floor(time) != Floor(previous_time))
				{
					AttemptToHeal(lab_state, effect->entity, effect->entity, 5);
				}
				break;
			}
			case HealOverTimeEffectId:
			{
				if(Floor(time * (1.0f / 3.0f)) != Floor(previous_time * (1.0f / 3.0f)))
				{
					AttemptToHeal(lab_state, effect->entity, effect->entity, 5);
				}
				break;
			}
			case BleedingEffectId:
			{
				if(Floor(time * (1.0f / 3.0f)) != Floor(previous_time * (1.0f / 3.0f)))
				{
					DealDamageFromEffect(lab_state, effect->effect_id, effect->entity, 10);
				}
			}
		}
	}
}

static void
func UpdateDamageDisplays(CombatLabState *lab_state, R32 seconds)
{
	R32 scroll_up_speed = 1.0f;

	I32 remaining_display_n = 0;
	for(I32 i = 0; i < lab_state->damage_display_n; i++)
	{
		DamageDisplay *display = &lab_state->damage_displays[i];
		display->position.y -= scoll_up_speed * seconds;

		display->time_remaining -= seconds;
		if(display->time_remaining > 0.0f)
		{
			lab_state->damage_displays[remaining_display_n] = *display;
			remaining_display_n++;
		}
	}

	lab_state->damage_display_n = remaining_display_n;
}

static B32
func CanMove(CombatLabState *lab_State, Entity *entity)
{
	B32 can_move = false;
	if(IsDead(entity))
	{
		can_move = false;
	}
	else if(HasEffect(lab_state, entity, KickedEffectId) || HasEffect(lab_state, entity, RollingEffectId))
	{
		can_move = false;
	}
	else
	{
		can_move = true;
	}
	return can_move;
}

static B32
func CanPickUpItem(CombatLabState *lab_state, Entity *entity, DroppedItem *item)
{
	B32 is_alive = !IsDead(entity);
	B32 has_empty_slot = HasEmptySlot(&lab_state->inventory);
	B32 can_pick_up_item = (is_alive && has_empty_slot);
	return can_pick_up_item;
}

static void
func PickUpItem(CombatLabState *lab_State, Entity *entity, DroppedItem *item)
{
	Assert(item != 0);
	Assert(CanPickUpItem(lab_state, entity, item));

	I32 item_index = 0;
	B32 item_found = false;
	for(I32 i = 0; i < lab_state->dropped_item_n; i++)
	{
		if(item == &lab_state->dropped_items[i])
		{
			item_found = true;
			item_index = i;
			break;
		}
	}

	Assert(item_found);
	lab_state->dropped_item_n--;
	for(I32 i = item_index; i < lab_state->dropped_item_n; i++)
	{
		lab_state->dropped_items[i] = lab_state->dropped_items[i + 1];
	}

	AddItemToInventory(&lab_state->inventory, item->item_id);

	I8 *item_name = GetVisibleItemName(lab_state, item->item_id);
	CombatLog(lab_state, entity->name + " picks up " + item_name + ".");
}

static void
func PickUpFlower(CombatLabState *lab_state, Flower *flower)
{
	Inventory *inventory = &lab_state->inventory;
	Assert(HasEmptySlot(inventory));

	B32 found_flower = false;
	I32 flower_index = 0;
	for(I32 i = 0; i < lab_state->flower_n; i++)
	{
		if(&lab_state->flowers[i] == flower)
		{
			found_flower = true;
			flower_index = i;
			break;
		}
	}
	Assert(found_flower);

	AddItemToInventory(inventory, flower->item_id);

	Entity *player = &lab_state->entities[0];
	I8 *visible_flower_name = GetVisibleItemName(lab_state, flower->item_id);
	CombatLog(lab_state, player->name + " picks up " + visible_flower_name + ".");

	for(I32 i = flower_index + 1; i < lab_state->flower_n; i++)
	{
		lab_state->flowers[i - 1] = lab_state->flowers[i];
	}

	lab_state->flower_n--;
}

static void
func UpdateAndDrawDroppedItems(Canvas *canvas, CombatLabState *lab_state, V2 mouse_position, R32 seconds)
{
	lab_state->hover_dropped_item = 0;

	I32 remaining_n = 0;
	for(I32 i = 0; i < lab_state->dropped_item_n; i++)
	{
		DroppedItem *item = &lab_state->dropped_items[i];
		item->time_left -= seconds;
		if(item->time_left > 0.0f)
		{
			lab_state->dropped_items[remaining_n] = *item;
			remaining_n++;
		}
	}
	lab_state->dropped_item_n = remaining_n;

	for(I32 i = 0; i < lab_state->dropped_item_n; i++)
	{
		DroppedItem *item = &lab_state->dropped_items[i];
		I8 *item_name = GetVisibleItemName(lab_state, item->item_id);

		V4 normal_background_color = MakeColor(0.5f, 0.5f, 0.5f);
		V4 hover_background_color = MakeColor(0.7f, 0.5f, 0.5f);
		V4 text_color = MakeColor(1.0f, 1.0f, 1.0f);

		R32 text_width = GetTextWidth(canvas, item_name);
		R32 text_height = GetTextHeight(canvas, item_name);
		
		Camera *camera = canvas->camera;
		Assert(camera->unit_in_pixels > 0.0f);
		Rect rect = MakeRect(item->position, text_width, text_height);

		V4 background_color = normal_background_color;
		if(IsPointInRect(mouse_position, rect))
		{
			background_color = hover_background_color;
			lab_state->hover_dropped_item = item;
		}

		DrawRect(canvas, rect, background_color);
		DrawTextLineXYCentered(canvas, item_name, item->position.y, item->position.x, text_color);
	}
}

static void
func DrawDamageDisplays(Canvas *canvas, CombatLabState *lab_state)
{
	V4 damage_color = MakeColor(1.0f, 1.0f, 0.0f);
	V4 heal_color = MakeColor(0.0f, 0.5f, 0.0f);
	for(I32 i = 0; i < lab_state->damage_display_n; i++)
	{
		DamageDisplay *display = &lab_state->damage_displays[i];
		I8 text[16] = {};
		V4 text_color = {};
		if(display->damage > 0)
		{
			text_color = damage_color;
			OneLineString(text, 16, display->damage);
		}
		else if(display->damage < 0)
		{
			text_color = heal_color;
			OneLineString(text, 16, -display->damage);
		}
		else
		{
			DebugBreak();
		}
		DrawTextLineXYCentered(canvas, text, display->position.y, display->position.x, text_color);
	}
}

static V2
func GetClosestRectOutlinePosition(Rect rect, V2 point)
{
	V2 result = {};

	Assert(IsPointInRect(point, rect));

	R32 left_distance = (point.x - rect.left);
	Assert(left_distance >= 0.0f);

	R32 min_distance = left_distance;
	result = MakePoint(rect.left, point.y);

	R32 right_distance = (rect.right - point.x);
	Assert(right_distance >= 0.0f);
	if(right_distance < min_distance)
	{
		min_distance = right_distance;
		result = MakePoint(rect.right, point.y);
	}

	R32 top_distance = (point.y - rect.top);
	Assert(top_distance >= 0.0f);
	if(top_distance < min_distance)
	{
		min_distance = top_distance;
		result = MakePoint(point.x, rect.top);
	}

	R32 bottom_distance = (rect.bottom - point.y);
	Assert(bottom_distance >= 0.0f);
	if(bottom_distance < min_distance)
	{
		min_distance = bottom_distance;
		result = MakePoint(point.x, rect.bottom);
	}

	return result;
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
}

static void
func RemoveEffectsOfDeadEntities(CombatLabState *lab_state)
{
	I32 remaining_effect_n = 0;
	for(I32 i = 0; i < lab_state->effect_n; i++)
	{
		Effect *effect = &lab_state->effects[i];
		if(!IsDead(effect->entity))
		{
			lab_state->effects[remaining_effect_n] = *effect;
			remaining_effect_n++;
		}
		else
		{
			Entity *player = &lab_state->entities[0];
			if(effect->entity == player)
			{
				RecalculatePlayerAttributes(lab_state);
			}
		}
	}

	lab_state->effect_n = remaining_effect_n;
}

static void
func DeleteInventoryItem(InventoryItem item)
{
	Inventory* inventory = item.inventory;
	Assert(inventory != 0);

	SetSlotItemId(item.inventory, item.slot, NoItemId);
}

static B32
func ItemIsOnCooldown(CombatLabState *lab_state, Entity *entity, ItemId item_id)
{
	B32 is_on_cooldown = false;

	ItemId cooldown_item_id = GetItemIdForCooldown(item_id);
	Assert(ItemHasOwnCooldown(cooldown_item_id));

	for(I32 i = 0; i < lab_state->item_cooldown_n; i++)
	{
		ItemCooldown *cooldown = &lab_state->item_cooldowns[i];
		Assert(ItemHasOwnCooldown(cooldown->item_id));
		if(cooldown->entity == entity && cooldown->item_id == cooldown_item_id)
		{
			is_on_cooldown = true;
			break;
		}
	}
	return is_on_cooldown;
}

static void
func AddItemCooldown(CombatLabState *lab_state, Entity *entity, ItemId item_id)
{
	Assert(!ItemIsOnCooldown(lab_state, entity, item_id));

	item_id = GetItemIdForCooldown(item_id);
	Assert(ItemHasOwnCooldown(item_id));

	R32 duration = GetItemCooldownDuration(item_id);
	Assert(duration > 0.0f);

	ItemCooldown cooldown = {};
	cooldown.entity = entity;
	cooldown.item_id = item_id;
	cooldown.time_remaining = duration;

	Assert(lab_state->item_cooldown_n < MaxItemCooldownN);
	lab_staet->item_cooldowns[lab_state->item_cooldown_n] = cooldown;
	lab_state->item_cooldown_n++;
}

static B32
func CanUseItem(CombatLabState *lab_state, Entity *entity, ItemId item_id)
{
	B32 can_use = true;
	if(IsDead(entity))
	{
		can_use = false;
	}
	else if(ItemIsOnCooldown(lab_state, entity, item_id))
	{
		can_use = false;
	}
	else
	{
		switch(item_id)
		{
			case HealthPotionItemId:
			{
				can_use = CanBeHealed(entity);
				break;
			}
			case AntiVenomItemId:
			{
				bool has_friendly_target = (entity->target != 0 && entity->target->group_id == entity->group_id);
				bool target_is_alive = (entity->target != 0 && !IsDead(entity->target));
				can_use = (has_friendly_target && target_is_alive);
				break;
			}
			case IntellectPotionItemId:
			{
				can_use = true;
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
				can_use = true;
				break;
			}
			default:
			{
				can_use = false;
			}
		}
	}
	return can_use;
}

static void
func UseItem(CombatLabState *lab_state, Entity *entity, InventoryItem item)
{
	ItemId item_id = item.item_id;
	Assert(CanUseItem(lab_state, entity, item_id));
	switch(item_id)
	{
		case HealthPotionItemId:
		{
			Heal(lab_state, entity, entity, 30);
			break;
		}
		case AntiVenomItemId:
		{
			RemoveEffect(lab_state, entity->target, PoisonedEffectId);
			break;
		}
		case IntellectPotionItemId:
		{
			ResetOrAddEffect(lab_state, entity, IntellectPotionEffectId);
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
			ResetOrAddEffect(lab_state, entity, FeelingSmartEffectId);
			break;
		}
		case BlueFlowerOfHealingItemId:
		{
			ResetOrAddEffect(lab_state, entity, HealOverTimeEffectId);
			break;
		}
		case BlueFlowerOfDampeningItemId:
		{
			ResetOrAddEffect(lab_state, entity, ReducedDamageDoneAndTakenEffectId);
			break;
		}
		case RedFlowerOfStrengthItemId:
		{
			ResetOrAddEffect(lab_state, entity, FeelingStrongEffectId);
			break;
		}
		case RedFlowerOfHealthItemId:
		{
			Heal(lab_state, entity, entity, 30);
			break;
		}
		case RedFlowerOfPoisonItemId:
		{
			ResetOrAddEffect(lab_state, entity, PoisonedEffectId);
			break;
		}
		case YellowFlowerOfDexterityItemId:
		{
			ResetOrAddEffect(lab_state, entity, FeelingQuickEffectId);
			break;
		}
		case YellowFlowerOfAntivenomItemId:
		{
			RemoveEffect(lab_state, entity->target, PoisonedEffectId);
			ResetOrAddEffect(lab_state, entity, ImmuneToPoisonEffectId);
			break;
		}
		case YellowFlowerOfRageItemId:
		{
			ResetOrAddEffect(lab_state, entity, IncreasedDamageDoneAndTakenEffectId);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	I8 *item_name = GetVisibleItemName(lab_state, item_id);
	CombatLog(lab_state, entity->name + " uses item " + item_name + ".");
	DeleteInventoryItem(item);

	if(GetItemCooldownDuration(item_id) > 0.0f)
	{
		AddItemCooldown(lab_state, entity, item_id);
	}
}

static R32
func GetEntityMoveSpeed(CombatLabState *lab_state, Entity *entity)
{
	R32 move_speed = EntityMoveSpeed;
	if(IsDead(entity))
	{
		moveSpeed = 0.0f;
	}
	else
	{
		switch(entity->class_id)
		{
			case SnakeClassId:
			case CrocodileClassId:
			{
				move_speed = AnimalMoveSpeed;
				break;
			}
			default:
			{
				move_speed = EntityMoveSpeed;
			}
		}

		if(HasEffect(lab_state, entity, BittenEffectId))
		{
			move_speed *= 0.5f;
		}

		if(HasEffect(lab_state, entity, EarthShakeEffectId))
		{
			Map* map = &lab_state->map;
			IV2 tile = GetContainingTile(map, entity->position);
			TileId tile_type = GetTileType(map, tile);
		}
	}

	return move_speed;
}

static B32
func IsNeutral(Entity *entity)
{
	Assert(entity != 0);
	B32 neutral = false;
	switch(entity->class_id)
	{
		case SnakeClassId:
		{
			neutral = true;
			break;
		}
	}

	return neutral;
}

static void
func CombatLabUpdate(CombatLabState *lab_state, Canvas *canvas, R32 seconds, UserInput *user_input)
{
	V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, background_color);

	DrawMap(canvas, &labState->map);

	V2 mouse_position = PixelToUnit(canvas->camera, user_input->mouse_pixel_position);
	UpdateAndDrawFlowers(canvas, lab_state, mouse_position);

	R32 map_width  = GetMapWidth(&lab_state->map);
	R32 map_height = GetMapHeight(&lab_state->map);

	Entity *player = &lab_state->entities[0];
	Assert(player->group_id == PlayerGroupId);

	UpdateAbilityCooldowns(lab_state, seconds);
	UpdateItemCooldowns(lab_state, seconds);
	UpdateEffects(lab_state, seconds);
	UpdateDamageDisplays(lab_state, seconds);

	player->input_direction = MakeVector(0.0f, 0.0f);

	Map *map = &lab_state->map;

	B32 input_move_left  = IsKeyDown(user_input, 'A') || IsKeyDown(user_input, VK_LEFT);
	B32 input_move_right = IsKeyDown(user_input, 'D') || IsKeyDown(user_input, VK_RIGHT);
	B32 input_move_up    = IsKeyDown(user_input, 'W') || IsKeyDown(user_input, VK_UP);
	B32 input_move_down  = IsKeyDown(user_input, 'S') || IsKeyDown(user_input, VK_DOWN);	

	IV2 player_tile = GetContainingTile(map, player->position);
	if(input_move_left && input_move_right)
	{
		player->input_diretion.x = 0.0f;
	}
	else if(input_move_left)
	{
		player->input_direction.x = -1.0f;
	}
	else if(input_move_right)
	{
		player->input_direction.x = +1.0f;
	}

	if(input_move_up && input_move_down)
	{
		player->input_direction.y = 0.0f;
	}
	else if(input_move_up)
	{
		player->input_direction.y = -1.0f;
	}
	else if(input_move_down)
	{
		player->input_direction.y = +1.0f;
	}

	if(IsDead(player))
	{
		player->velocity = MakeVector(0.0f, 0.0f);
	}
	if(CanMove(labState, player))
	{
		R32 move_speed = GetEntityMoveSpeed(lab_state, player);
		player->velocity = move_speed * player->input_direction;
	}

	UpdateEntityMovement(player, map, seconds);

	canvas->camera->center = player->position;

	if(player->input_direction.x != 0.0f || player->input_direction.y != 0.0f)
	{
		if(player->casted_ability != NoAbilityId)
		{
			player->casted_ability = NoAbilityId;
		}
	}

	R32 enemy_pull_distance = 30.0f;
	for(I32 i = 0; i < EntityN; i++)
	{
		Entity *enemy = &lab_state->entities[i];
		if (enemy->group_id != EnemyGroupId)
		{
			continue;
		}

		if(enemy->target == 0)
		{
			R32 distance_from_player = MaxDistance(player->position, enemy->position);
			B32 is_neutral = IsNeutral(enemy);
			if(!is_neutral && distance_from_player <= enemy_pull_distance)
			{
				enemy->target = player;
				AddEmptyHateTableEntry(&lab_state->hate_table, enemy, player);
			}
		}

		Entity *target = enemy->target;
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
			IV2 enemy_tile = GetContainingTile(map, enemy->position);
			Assert(enemy->group_id != enemy->target->group_id);
			IV2 target_tile = GetContainingTile(map, enemy->target->position);
			if(enemy_tile == target_tile)
			{
				enemy->velocity = MakeVector(0.0f, 0.0f);
			}
			else
			{
			}
		}

		UpdateEntityMovement(enemy, map, seconds);
	}

	if(WasKeyPressed(user_input, VK_TAB))
	{
		Entity *target = 0;
		R32 target_player_distance = 0.0f;
		for(I32 i = 0; i < EntityN; i++)
		{
			Entity *enemy = &lab_state->entities[i];
			if(enemy->group_id == player->group_id || IsDead(enemy) || enemy == player->target)
			{
				continue;
			}
			Assert(enemy != player);

			R32 enemy_player_distance = Distance(enemy->position, player->position);
			if(target == 0 || enemy_player_distance < target_player_distance)
			{
				target = enemy;
				target_player_distance = enemy_player_distance;
			}
		}

		if(target)
		{
			player->target = target;
		}
	}

	if(WasKeyPressed(user_input, VK_F1))
	{
		player->target = player;
	}

	if(WasKeyReleased(user_input, VK_LBUTTON))
	{
		if(lab_state->hover_ability_id != NoAbilityId)
		{
			AttemptToUseAbility(lab_state, player, lab_state->hover_ability_id);
		}
		else
		{
			for(I32 i = 0; i < EntityN; i++)
			{
				Entity *entity = &lab_state->entities[i];
				R32 distance_from_mouse = Distance(mouse_position, entity->position);
				if(!IsDead(entity) && distance_from_mouse <= EntityRadius)
				{
					player->target = entity;
					break;
				}
			}
		}
	}

	for(I32 i = 0; i < EntityN; i++)
	{
		Entity *entity = &lab_state->entities[i];
		UpdateEntityRecharge(entity, seconds);
	}

	for(I32 i = '1'; i <= '7'; i++)
	{
		if(WasKeyPressed(user_input, i))
		{
			AttemptToUseAbilityAtIndex(lab_state, player, i - '1');
		}
	}

	for(I32 i = 1; i < EntityN; i++)
	{
		Entity *entity = &lab_state->entities[i];
		if(entity->class_id == SnakeClassId)
		{
			AttemptToUseAbility(lab_state, entity, SnakeStrikeAbilityId);
		}
		else if(entity->class_id == CrocodileClassId)
		{
			AttemptToUseAbility(lab_state, entity, CrocodileBiteAbilityId);
			AttemptToUseAbility(lab_state, entity, CrocodileLashAbilityId);
		}
		else if(entity->class_id == TigerClassId)
		{
			AttemptToUseAbility(lab_state, entity, TigerBiteAbilityId);
		}
		else
		{
			DebugBreak();
		}
	}

	for(I32 i = 0; i < EntityN; i++)
	{
		Entity *entity = &lab_state->entities[i];
		if(entity->casted_ability != NoAbilityId)
		{
			if(IsDead(entity))
			{
				entity->casted_ability = NoAbilityId;
			}
			else
			{
				Assert(entity->cast_time_total > 0.0f);
				entity->cast_time_remaining -= seconds;
				if(entity->cast_time_remaining <= 0.0f)
				{
					FinishCasting(lab_state, entity);
				}
			}
		}
	}

	if(WasKeyReleased(user_input, VK_RBUTTON))
	{
		if(lab_state->hoverItem.item_id != NoItemId)
		{
			if(CanUseItem(lab_state, player, lab_state->hover_item.item_id))
			{
				UseItem(lab_state, player, lab_state->hover_item);
			}
		}
	}

	RemoveDeadEntitiesFromHateTable(&lab_state->hate_table);
	SortHateTable(&lab_state->hate_table);
	UpdateEnemyTargets(lab_state);
	RemoveEffectsOfDeadEntities(lab_state);

	V4 player_color = MakeColor(0.0f, 1.0f, 1.0f);
	if(player == player->target)
	{
		DrawSelectedEntity(canvas, player, player_color);
	}
	else
	{
		DrawEntity(canvas, player, player_color);
	}

	V4 neutral_enemy_color = MakeColor(1.0f, 1.0f, 0.0f);
	V4 hostile_enemy_color = MakeColor(1.0f, 0.0f, 0.0f);
	for(I32 i = 0; i < EntityN; i++)
	{
		Entity* enemy = &lab_state->entities[i];
		if(enemy->group_id != EnemyGroupId)
		{
			continue;
		}

		B32 is_neutral = IsNeutral(enemy);
		V4 enemy_color = (is_neutral) ? neutral_enemy_color : hostile_enemy_color;

		if(enemy == player->target)
		{
			DrawSelectedEntity(canvas, enemy, enemy_color);
		}
		else
		{
			DrawEntity(canvas, enemy, enemy_color);
		}
	}

	for(I32 i = 0; i < EntityN; i++)
	{
		Entity *entity = &lab_state->entities[i];
		DrawEntityBars(canvas, entity);
	}

	V4 entity_name_color = MakeColor(1.0f, 1.0f, 1.0f);
	V4 cast_ability_name_color = MakeColor(0.0f, 1.0f, 1.0f);
	for(I32 i = 0; i < EntityN; i++)
	{
		Entity *entity = &lab_state->entities[i];
		R32 text_center_x = entity->position.x;
		R32 text_bottom = entity->position.y - 2.0f * EntityRadius;

		if(entity->casted_ability == NoAbilityId)
		{
			DrawTextLineBottomXCentered(canvas, entity->name, text_bottom, text_center_x, entity_name_color);
		}
		else
		{
			I8 *ability_name = GetAbilityName(entity->casted_ability);
			DrawTextLineBottomXCentered(canvas, ability_name, text_bottom, text_center_x, cast_ability_name_color);
		}
	}

	if(WasKeyReleased(user_input, 'V'))
	{
		lab_state->show_inventory = !lab_state->show_inventory;
	}

	if(WasKeyReleased(user_input, 'C'))
	{
		lab_state->show_character_info = !lab_state->show_character_info;
	}

	if(WasKeyPressed(user_input, VK_LBUTTON))
	{
		if(lab_state->hover_item.inventory != 0 && lab_state->hover_item.item_id != NoItemId)
		{
			lab_state->drag_item = lab_state->hover_item;
		}
	}

	if(WasKeyReleased(user_input, VK_LBUTTON))
	{
		InventoryItem *drag_item = &lab_state->drag_item;
		InventoryItem *hover_item = &lab_state->hover_item;
		if(drag_item->inventory != 0)
		{
			Assert(drag_item->item_id != NoItemId);
			if(hover_item->inventory != 0)
			{
				AttemptToSwapItems(drag_item, hover_item);
			}
			else
			{
				DropItem(lab_state, player, drag_item->item_id);
				SetSlotItemId(drag_item->inventory, drag_item->slot, NoItemId);
			}
		}

		lab_state->drag_item.inventory = 0;
	}

	if(WasKeyPressed(user_input, ' '))
	{
		DroppedItem *item = lab_state->hover_dropped_item;
		if(item != 0)
		{
			if(CanPickUpItem(lab_state, player, item))
			{
				PickUpItem(lab_state, player, item);
			}
		}

		Flower *flower = lab_state->hover_flower;
		if(flower)
		{
			if(HasEmptySlot(&lab_state->inventory))
			{
				PickUpFlower(lab_state, flower);
				player->herbalism++;
			}
		}
	}

	lab_state->hover_item.inventory = 0;
	if(lab_state->show_inventory)
	{
		Inventory *inventory = &lab_state->inventory;
		I32 right = (canvas->bitmap.width - 1) - UIBoxPadding - UIBoxSide - UIBoxPadding;
		I32 bottom = (canvas->bitmap.height - 1) - UIBoxPadding;
		inventory->left = right - GetInventoryWidth(inventory);
		inventory->top = bottom - GetInventoryHeight(inventory);
		UpdateAndDrawInventory(canvas, lab_state, inventory, user_input->mouse_pixel_position);
	}
	else
	{
		DrawHateTable(canvas, lab_state);
	}

	DrawAbilityBar(canvas, lab_state, user_input->mouse_pixel_position);
	DrawHelpBar(canvas, lab_state, user_input->mouse_pixel_position);
	DrawPlayerEffectBar(canvas, lab_state);
	DrawTargetEffectBar(canvas, lab_state);
	DrawDamageDisplays(canvas, lab_state);

	if(lab_state->show_character_info)
	{
		DrawCharacterInfo(canvas, player, lab_state, user_input->mouse_pixel_position);
	}
	else
	{
		DrawCombatLog(canvas, lab_state);
	}

	InventoryItem *drag_item = &lab_state->drag_item;
	if(drag_item->inventory != 0)
	{
		Assert(drag_item->item_id != 0);
		
		I32 slot_top  = user_input->mouse_pixel_position.row - InventorySlotSide / 2;
		I32 slot_left = user_input->mouse_pixel_position.col - InventorySlotSide / 2;

		V4 drag_outline_color = MakeColor(1.0f, 0.5f, 0.0f);

		DrawInventorySlot(canvas, lab_state, drag_item->slot_id, drag_item->item_id, slot_top, slot_left);
		DrawInventorySlotOutline(canvas, slot_top, slot_left, drag_outline_color);
	}

	UpdateAndDrawDroppedItems(canvas, lab_state, mouse_position, seconds);
}

// TODO: Stop spamming combat log when dying next to a tree!
// TODO: Don't log overheal!
// TODO: Mana
// TODO: Offensive and defensive target
// TODO: Invisibility
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