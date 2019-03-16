#pragma once

#include "Lab.hpp"

#define TileSide 10.0f
#define TileRowN 10
#define TileColN 10

enum AbilityId
{
	NoAbilityId,
	SmallPunchAbilityId,
	BigPunchAbilityId,
	KickAbilityId,
	SpinningKickAbilityId,
	RollAbilityId,
	AvoidanceAbilityId,

	SwordStabAbilityId,
	SwordSwingAbilityId,
	RaiseShieldAbilityId,
	BurnAbilityId,
	LightOfTheSunAbilityId,
	BlessingOfTheSunAbilityId,
	MercyOfTheSunAbilityId,

	EnemyAbilityId,
	AbilityN
};

enum EffectId
{
	NoEffectId,
	KickedEffectId,
	RollingEffectId,
	InvulnerableEffectId,
	ShieldRaisedEffectId,
	BurningEffectId,
	BlessingOfTheSunEffectId,
	BlindEffectId
};

enum ClassId
{
	NoClassId,
	MonkClassId,
	PaladinClassId,
	EnemyClassId
};

struct TileIndex
{
	I32 row;
	I32 col;
};

#define EntityRadius (0.25f * TileSide)
#define EntityMaxHealth 100

enum EntityGroupId
{
	PlayerGroupId,
	EnemyGroupId
};

struct Entity
{
	V2 position;
	V2 velocity;

	F32 recharge;
	F32 rechargeFrom;
	I32 health;

	I32 classId;
	I32 groupId;
	Entity* target;

	V2 inputDirection;
};

struct Cooldown
{
	Entity* entity;
	I32 abilityId;
	F32 timeRemaining;
};

#define DamageDisplayDuration 2.0f
struct DamageDisplay
{
	F32 timeRemaining;
	V2 position;
	I32 damage;
};

struct Effect
{
	Entity* entity;
	I32 effectId;
	F32 timeRemaining;
};

#define EntityN 5
#define MaxCooldownN 64
#define MaxEffectN 64
#define MaxDamageDisplayN 64
struct CombatLabState
{
	Camera camera;
	Canvas canvas;
	B32 running;

	Cooldown cooldowns[MaxCooldownN];
	I32 cooldownN;

	Effect effects[MaxEffectN];
	I32 effectN;

	Entity entities[EntityN];

	DamageDisplay damageDisplays[MaxDamageDisplayN];
	I32 damageDisplayN;

	I32 hoverAbilityId;
};
CombatLabState gCombatLabState;

#define PlayerSpeed 11.0f

#define EnemyAttackRadius 10.0f

#define MaxMeleeAttackDistance (4.0f * EntityRadius)

static void func CombatLabResize(CombatLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 5.0f;
	camera->center = MakePoint(0.0f, 0.0f);
}

static void func CombatLabBlit(Canvas* canvas, HDC context, RECT rect)
{
	I32 width  = rect.right - rect.left;
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

static TileIndex func MakeTileIndex(I32 row, I32 col)
{
	TileIndex index = {};
	index.row = row;
	index.col = col;
	return index;
}

static B32 func IsValidTileIndex(TileIndex index)
{
	B32 result = (IsIntBetween(index.row, 0, TileRowN - 1) &&
				  IsIntBetween(index.col, 0, TileColN - 1));
	return result;
}

static V2 func GetTileCenter(TileIndex index)
{
	Assert(IsValidTileIndex(index));
	F32 x = F32(index.col) * TileSide + TileSide * 0.5f;
	F32 y = F32(index.row) * TileSide + TileSide * 0.5f;
	V2 position = MakePoint(x, y);
	return position;
}

static TileIndex func GetContainingTileIndex(V2 point)
{
	I32 row = Floor(point.y / TileSide);
	I32 col = Floor(point.x / TileSide);
	TileIndex index = MakeTileIndex(row, col);
	return index;
}

static void func CombatLabInit(CombatLabState* labState, I32 windowWidth, I32 windowHeight)
{
	labState->running = true;
	labState->canvas.glyphData = GetGlobalGlyphData();
	CombatLabResize(labState, windowWidth, windowHeight);

	F32 mapWidth = TileRowN * TileSide;
	F32 mapHeight = TileColN * TileSide;
	V2 mapCenter = MakePoint(mapWidth * 0.5f, mapHeight * 0.5f);

	Entity* player = &labState->entities[0];
	player->position = mapCenter;
	player->health = EntityMaxHealth;
	player->inputDirection = MakePoint(0.0f, 0.0f);
	player->classId = MonkClassId;
	player->groupId = PlayerGroupId;

	{
		Entity* enemy = &labState->entities[1];
		enemy->position = MakePoint(EntityRadius, EntityRadius);
		enemy->health = EntityMaxHealth;
		enemy->classId = EnemyClassId;
		enemy->groupId = EnemyGroupId;
	}
	{
		Entity* enemy = &labState->entities[2];
		enemy->position = MakePoint(mapWidth - EntityRadius, EntityRadius);
		enemy->health = EntityMaxHealth;
		enemy->classId = EnemyClassId;
		enemy->groupId = EnemyGroupId;
	}
	{
		Entity* enemy = &labState->entities[3];
		enemy->position = MakePoint(mapWidth - EntityRadius, mapHeight - EntityRadius);
		enemy->health = EntityMaxHealth;
		enemy->classId = EnemyClassId;
		enemy->groupId = EnemyGroupId;
	}
	{
		Entity* enemy = &labState->entities[4];
		enemy->position = MakePoint(EntityRadius, mapHeight - EntityRadius);
		enemy->health = EntityMaxHealth;
		enemy->classId = EnemyClassId;
		enemy->groupId = EnemyGroupId;
	}
}

static I32 func GetTileDistance(TileIndex tileIndex1, TileIndex tileIndex2)
{
	Assert(IsValidTileIndex(tileIndex1));
	Assert(IsValidTileIndex(tileIndex2));
	I32 rowDistance = IntAbs(tileIndex1.row - tileIndex2.row);
	I32 colDistance = IntAbs(tileIndex1.col - tileIndex2.col);
	I32 distance = IntMax2(rowDistance, colDistance);
	return distance;
}

#define TileGridColor MakeColor(0.2f, 0.2f, 0.2f)

static B32 func IsOnCooldown(CombatLabState* labState, Entity* entity, I32 abilityId)
{
	B32 isOnCooldown = false;
	for(I32 i = 0; i < labState->cooldownN; i++)
	{
		Cooldown* cooldown = &labState->cooldowns[i];
		if(cooldown->entity == entity && cooldown->abilityId == abilityId)
		{
			isOnCooldown = true;
			break;
		}
	}
	return isOnCooldown;
}

static B32 func IsDead(Entity* entity)
{
	B32 isDead = (entity->health == 0);
	return isDead;
}

static I32 func GetAbilityClass(I32 abilityId)
{
	I32 classId = NoClassId;
	switch(abilityId)
	{
		case SmallPunchAbilityId:
		case BigPunchAbilityId:
		case KickAbilityId:
		case SpinningKickAbilityId:
		case RollAbilityId:
		case AvoidanceAbilityId:
		{
			classId = MonkClassId;
			break;
		}
		case SwordStabAbilityId:
		case SwordSwingAbilityId:
		case RaiseShieldAbilityId:
		case BurnAbilityId:
		case LightOfTheSunAbilityId:
		case BlessingOfTheSunAbilityId:
		case MercyOfTheSunAbilityId:
		{
			classId = PaladinClassId;
			break;
		}
		case EnemyAbilityId:
		{
			classId = EnemyClassId;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return classId;
}

static B32 func HasEffect(CombatLabState* labState, Entity* entity, I32 effectId)
{
	B32 hasEffect = false;
	for(I32 i = 0; i < labState->effectN; i++)
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

static B32 func AbilityIsEnabled(CombatLabState* labState, Entity* entity, I32 abilityId)
{
	B32 canUse = false;
	Entity* target = entity->target;

	I32 abilityClassId = GetAbilityClass(abilityId);

	if(IsDead(entity))
	{
		canUse = false;
	}
	else if(abilityClassId != entity->classId)
	{
		DebugBreak();
		canUse = false;
	}
	else
	{
		switch(abilityId)
		{
			case SmallPunchAbilityId:
			case BigPunchAbilityId:
			case KickAbilityId:
			case SwordStabAbilityId:
			case EnemyAbilityId:
			{
				canUse = ((target != 0) && (!IsDead(target)) &&
					      (MaxDistance(entity->position, target->position) <= MaxMeleeAttackDistance));
				break;
			}
			case RollAbilityId:
			{
				canUse = (entity->inputDirection.x != 0.0f || entity->inputDirection.y != 0.0f);
				break;
			}
			case BurnAbilityId:
			{
				canUse = ((target != 0) && (!IsDead(target)) &&
						  (MaxDistance(entity->position, target->position) <= 30.0f));
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
				canUse = true;
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
			canUse = false;
		}
	} else if(HasEffect(labState, entity, BlindEffectId))
	{
		canUse = false;
	}

	return canUse;
}

static B32 func CanUseAbility(CombatLabState* labState, Entity* entity, I32 abilityId)
{
	B32 canUseAbility = (entity->recharge == 0.0f && !IsOnCooldown(labState, entity, abilityId) &&
						 AbilityIsEnabled(labState, entity, abilityId));
	return canUseAbility;
}

static void func PutOnRecharge(Entity* entity, F32 rechargeTime)
{
	Assert(rechargeTime > 0.0f);
	Assert(entity->recharge == 0.0f);
	entity->rechargeFrom = rechargeTime;
	entity->recharge = rechargeTime;
}

static B32 func CanDamage(CombatLabState* labState, Entity* entity)
{
	B32 canDamage = false;
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

static void func AddDamageDisplay(CombatLabState* labState, V2 position, I32 damage)
{
	Assert(labState->damageDisplayN < MaxDamageDisplayN);
	DamageDisplay* display = &labState->damageDisplays[labState->damageDisplayN];
	labState->damageDisplayN++;
	display->position = position;
	display->damage = damage;
	display->timeRemaining = DamageDisplayDuration;
}

static void func DealFinalDamage(Entity* entity, I32 damage)
{
	Assert(damage > 0);
	entity->health = IntMax2(entity->health - damage, 0);
}

static void func DealDamage(CombatLabState* labState, Entity* entity, I32 damage)
{
	if(CanDamage(labState, entity))
	{
		I32 finalDamage = damage;
		if(HasEffect(labState, entity, ShieldRaisedEffectId))
		{
			finalDamage = damage / 2;
		}

		DealFinalDamage(entity, finalDamage);
		AddDamageDisplay(labState, entity->position, finalDamage);
	}
}

static void func Heal(CombatLabState* labState, Entity* entity, I32 damage)
{
	Assert(!IsDead(entity));
	entity->health = IntMin2(entity->health + damage, EntityMaxHealth);
	AddDamageDisplay(labState, entity->position, -damage);
}

static F32 func GetAbilityCooldownDuration(I32 abilityId)
{
	F32 cooldown = 0.0f;
	switch(abilityId)
	{
		case BigPunchAbilityId:
		{
			cooldown = 3.0f;
			break;
		}
		case KickAbilityId:
		{
			cooldown = 5.0f;
			break;
		}
		case RollAbilityId:
		{
			cooldown = 5.0f;
			break;
		}
		case AvoidanceAbilityId:
		{
			cooldown = 10.0f;
			break;
		}
		case RaiseShieldAbilityId:
		{
			cooldown = 6.0f;
			break;
		}
		case BurnAbilityId:
		{
			cooldown = 10.0f;
			break;
		}
		case LightOfTheSunAbilityId:
		{
			cooldown = 10.0f;
			break;
		}
		case BlessingOfTheSunAbilityId:
		{
			cooldown = 10.0f;
			break;
		}
		case MercyOfTheSunAbilityId:
		{
			cooldown = 60.0f;
			break;
		}
		case SmallPunchAbilityId:
		case SpinningKickAbilityId:
		case SwordStabAbilityId:
		case SwordSwingAbilityId:
		case EnemyAbilityId:
		{
			cooldown = 0.0f;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return cooldown;
}

static B32 func HasCooldown(I32 abilityId)
{
	F32 cooldownDuration = GetAbilityCooldownDuration(abilityId);
	B32 hasCooldown = (cooldownDuration > 0.0f);
	return hasCooldown;
}

static void func AddCooldown(CombatLabState* labState, Entity* entity, I32 abilityId)
{
	Assert(HasCooldown(abilityId));
	F32 duration = GetAbilityCooldownDuration(abilityId);

	Assert(duration > 0.0f);
	Assert(labState->cooldownN < MaxCooldownN);
	Cooldown* cooldown = &labState->cooldowns[labState->cooldownN];
	labState->cooldownN++;

	cooldown->entity = entity;
	cooldown->abilityId = abilityId;
	cooldown->timeRemaining = duration;
}

static Cooldown* func GetCooldown(CombatLabState* labState, Entity* entity, I32 abilityId)
{
	Cooldown* result = 0;
	for(I32 i = 0; i < labState->cooldownN; i++)
	{
		Cooldown* cooldown = &labState->cooldowns[i];
		if(cooldown->entity == entity && cooldown->abilityId == abilityId)
		{
			result = cooldown;
			break;
		}
	}
	return result;
}

static V2 func GetClosestMoveDirection(V2 distance)
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

static F32 func GetEffectTotalDuration(I32 effectId)
{
	F32 duration = 0.0f;
	switch(effectId)
	{
		case KickedEffectId:
		{
			duration = 1.0f;
			break;
		}
		case RollingEffectId:
		{
			duration = 1.0f;
			break;
		}
		case InvulnerableEffectId:
		{
			duration = 2.0f;
			break;
		}
		case BurningEffectId:
		{
			duration = 8.0f;
			break;
		}
		case ShieldRaisedEffectId:
		{
			duration = 3.0f;
			break;
		}
		case BlessingOfTheSunEffectId:
		{
			duration = 60.0f;
			break;
		}
		case BlindEffectId:
		{
			duration = 5.0f;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	Assert(duration > 0.0f);
	return duration;
}

static void func RemoveEffect(CombatLabState* labState, Entity* entity, I32 effectId)
{
	I32 remainingEffectN = 0;
	for(I32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		if(effect->entity != entity || effect->effectId != effectId)
		{
			labState->effects[remainingEffectN] = *effect;
			remainingEffectN++;
		}
	}
	labState->effectN = remainingEffectN;
}

static void func AddEffect(CombatLabState* labState, Entity* entity, I32 effectId)
{
	Assert(labState->effectN < MaxEffectN);
	Effect* effect = &labState->effects[labState->effectN];
	labState->effectN++;

	effect->entity = entity;
	effect->effectId = effectId;
	
	F32 duration = GetEffectTotalDuration(effectId);
	Assert(duration > 0.0f);
	effect->timeRemaining = duration;
}

static F32 func GetAbilityRechargeDuration(I32 abilityId)
{
	F32 recharge = 0.0f;
	switch(abilityId)
	{
		case SmallPunchAbilityId:
		case BigPunchAbilityId:
		case KickAbilityId:
		case SpinningKickAbilityId:
		case AvoidanceAbilityId:
		case RollAbilityId:
		case SwordStabAbilityId:
		case SwordSwingAbilityId:
		case RaiseShieldAbilityId:
		case LightOfTheSunAbilityId:
		{
			recharge = 1.0f;
			break;
		}
		case EnemyAbilityId:
		{
			recharge = 3.0f;
			break;
		}
		case BurnAbilityId:
		case BlessingOfTheSunAbilityId:
		case MercyOfTheSunAbilityId:
		{
			recharge = 0.0f;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return recharge;
}

static void func UseAbility(CombatLabState* labState, Entity* entity, I32 abilityId)
{
	Assert(CanUseAbility(labState, entity, abilityId));
	Entity* target = entity->target;
	switch(abilityId)
	{
		case SmallPunchAbilityId:
		{
			Assert(target != 0);
			DealDamage(labState, target, 10);
			break;
		}
		case BigPunchAbilityId:
		{
			Assert(target != 0);
			DealDamage(labState, target, 30);
			break;
		}
		case KickAbilityId:
		{
			Assert(target != 0);
			F32 moveSpeed = 20.0f;
			target->velocity = moveSpeed * GetClosestMoveDirection(target->position - entity->position);
			AddEffect(labState, target, KickedEffectId);
			break;
		}
		case SpinningKickAbilityId:
		{
			for(I32 i = 0; i < EntityN; i++)
			{
				Entity* target = &labState->entities[i];
				if(target->groupId != entity->groupId)
				{
					F32 distance = Distance(entity->position, target->position);
					if(distance <= MaxMeleeAttackDistance)
					{
						DealDamage(labState, target, 10);
					}
				}
			}
			break;
		}
		case RollAbilityId:
		{
			F32 moveSpeed = 20.0f;
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
			I32 damage = 15;
			if(HasEffect(labState, entity, BlessingOfTheSunEffectId))
			{
				damage += 2;
			}

			Assert(target != 0);
			DealDamage(labState, target, damage);
			break;
		}
		case SwordSwingAbilityId:
		{
			I32 damage = 10;
			if(HasEffect(labState, entity, BlessingOfTheSunEffectId))
			{
				damage += 2;
			}

			for(I32 i = 0; i < EntityN; i++)
			{
				Entity* target = &labState->entities[i];
				if(target->groupId != entity->groupId)
				{
					F32 distance = Distance(entity->position, target->position);
					if(distance <= MaxMeleeAttackDistance)
					{
						DealDamage(labState, target, damage);
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
				Heal(labState, entity, 10);
			}
			break;
		}
		case BurnAbilityId:
		{
			Assert(target != 0);
			AddEffect(labState, target, BurningEffectId);
			break;
		}
		case LightOfTheSunAbilityId:
		{
			Heal(labState, entity, 20);
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
			Heal(labState, entity, 30);
			for(I32 i = 0; i < EntityN; i++)
			{
				Entity* target = &labState->entities[i];
				if(!IsDead(target) && target->groupId != entity->groupId)
				{
					F32 distance = Distance(entity->position, target->position);
					if(distance <= MaxMeleeAttackDistance)
					{
						AddEffect(labState, target, BlindEffectId);
					}
				}
			}
			break;
		}
		case EnemyAbilityId:
		{
			Assert(target != 0);
			DealDamage(labState, target, 10);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	F32 rechargeDuration = GetAbilityRechargeDuration(abilityId);
	if(rechargeDuration > 0.0f)
	{
		PutOnRecharge(entity, rechargeDuration);
	}

	if(HasCooldown(abilityId))
	{
		AddCooldown(labState, entity, abilityId);
	}
}

static void func AttemptToUseAbility(CombatLabState* labState, Entity* entity, I32 abilityId)
{
	if(CanUseAbility(labState, entity, abilityId))
	{
		UseAbility(labState, entity, abilityId);
	}
}

static I32 func GetClassAbilityOfIndex(I32 classId, I32 abilityIndex)
{
	I32 result = NoAbilityId;
	I32 classAbilityIndex = 0;
	for(I32 abilityId = 1; abilityId < AbilityN; abilityId++)
	{
		if(GetAbilityClass(abilityId) == classId)
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

static void func AttemptToUseAbilityOfIndex(CombatLabState* labState, Entity* entity, I32 abilityIndex)
{
	I32 abilityId = GetClassAbilityOfIndex(entity->classId, abilityIndex);
	if(abilityId != NoAbilityId)
	{
		AttemptToUseAbility(labState, entity, abilityId);
	}
}

static LRESULT CALLBACK func CombatLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	
	CombatLabState* labState = &gCombatLabState;
	Entity* player = &labState->entities[0];
	Assert(player->groupId == PlayerGroupId);
	switch(message)
	{
		case WM_SIZE:
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width  = clientRect.right - clientRect.left;
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
		case WM_KEYDOWN:
		{
			WPARAM keyCode = wparam;
			switch(keyCode)
			{
				case VK_TAB:
				{
					Entity* target = 0;
					F32 targetPlayerDistance = 0.0f;
					for(I32 i = 0; i < EntityN; i++)
					{
						Entity* enemy = &labState->entities[i];
						if(enemy->groupId == player->groupId || IsDead(enemy) || enemy == player->target)
						{
							continue;
						}
						Assert(enemy != player);

						F32 enemyPlayerDistance = Distance(enemy->position, player->position);
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
					break;
				}
				case VK_LEFT:
				case 'A':
				{
					player->inputDirection.x = -1.0f;
					break;
				}
				case VK_RIGHT:
				case 'D':
				{
					player->inputDirection.x = +1.0f;
					break;
				}
				case VK_UP:
				case 'W':
				{
					player->inputDirection.y = -1.0f;
					break;
				}
				case VK_DOWN:
				case 'S':
				{
					player->inputDirection.y = +1.0f;
					break;
				}
				case '1':
				{
					AttemptToUseAbilityOfIndex(labState, player, 0);
					break;
				}
				case '2':
				{
					AttemptToUseAbilityOfIndex(labState, player, 1);
					break;
				}
				case '3':
				{
					AttemptToUseAbilityOfIndex(labState, player, 2);
					break;
				}
				case '4':
				{
					AttemptToUseAbilityOfIndex(labState, player, 3);
					break;
				}
				case '5':
				{
					AttemptToUseAbilityOfIndex(labState, player, 4);
					break;
				}
				case '6':
				{
					AttemptToUseAbilityOfIndex(labState, player, 5);
					break;
				}
				case '7':
				{
					AttemptToUseAbilityOfIndex(labState, player, 6);
					break;
				}
				case 'C':
				{
					if(player->classId == MonkClassId)
					{
						player->classId = PaladinClassId;
					}
					else if(player->classId == PaladinClassId)
					{
						player->classId = MonkClassId;
					}
					else
					{
						DebugBreak();
					}
					break;
				}
			}
			break;
		}
		case WM_KEYUP:
		{
			WPARAM keyCode = wparam;
			switch(keyCode) 
			{
				case VK_LEFT:
				case 'A':
				case VK_RIGHT:
				case 'D':
				{
					player->inputDirection.x = 0.0f;
					break;
				}
				case VK_UP:
				case 'W':
				case VK_DOWN:
				case 'S':
				{
					player->inputDirection.y = 0.0f;
					break;
				}
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if(labState->hoverAbilityId != NoAbilityId)
			{
				AttemptToUseAbility(labState, &labState->entities[0], labState->hoverAbilityId);
			}
			else
			{
				V2 mousePosition = GetMousePosition(&labState->camera, window);
				for(I32 i = 0; i < EntityN; i++)
				{
					Entity* entity = &labState->entities[i];
					if(Distance(mousePosition, entity->position) <= EntityRadius &&
					   !IsDead(entity) && entity->groupId != player->groupId)
					{
						player->target = entity;
						break;
					}
				}
			}
			break;
		}
		case WM_SETCURSOR:
		{
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
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

	if(topPixel > bottomPixel)
	{
		IntSwap(&topPixel, &bottomPixel);
	}

	if(leftPixel > rightPixel)
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

	for(I32 row = topPixel; row < bottomPixel; row++)
	{
		for(I32 col = leftPixel; col < rightPixel; col++)
		{
			U32* pixel = bitmap.memory + row * bitmap.width + col;
			I32 pixelDistanceSquare = IntSquare(row - centerYPixel) + IntSquare(col - centerXPixel);
			if(pixelDistanceSquare <= pixelRadiusSquare)
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
	for(I32 i = 0; i < lineN; i++)
	{
		F32 angle1 = angle;
		F32 angle2 = angle + angleAdvance;
		V2 point1 = center + radius * RotationVector(angle1);
		V2 point2 = center + radius * RotationVector(angle2);
		Bresenham(canvas, point1, point2, color);

		angle += angleAdvance;
	}
}

static void func DrawEntity(Canvas* canvas, Entity* entity, V4 color)
{
	V4 entityOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawCircle(canvas, entity->position, EntityRadius, color);
	DrawCircleOutline(canvas, entity->position, EntityRadius, entityOutlineColor);
}

static void func DrawSelectedEntity(Canvas* canvas, Entity* entity, V4 color)
{
	V4 selectionOutlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawCircle(canvas, entity->position, EntityRadius, color);
	DrawCircleOutline(canvas, entity->position, EntityRadius, selectionOutlineColor);	
}

static void func DrawEntityBars(Canvas* canvas, Entity* entity)
{
	V2 healthBarCenter = entity->position + MakeVector(0.0f, -1.5f * EntityRadius);
	F32 healthBarWidth = 2.0f * EntityRadius;
	F32 healthBarHeight = 0.5f * EntityRadius;
	V4 healthBarBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
	V4 healthBarFilledColor = MakeColor(1.0f, 0.0f, 0.0f);
	V4 healthBarOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);
	F32 healthRatio = F32(entity->health) / F32(EntityMaxHealth);
	Assert(IsBetween(healthRatio, 0.0f, 1.0f));
	Rect healthBarBackgroundRect = MakeRect(healthBarCenter, healthBarWidth, healthBarHeight);
	DrawRect(canvas, healthBarBackgroundRect, healthBarBackgroundColor);
	Rect healthBarFilledRect = healthBarBackgroundRect;
	healthBarFilledRect.right = Lerp(healthBarFilledRect.left, healthRatio, healthBarFilledRect.right);
	DrawRect(canvas, healthBarFilledRect, healthBarFilledColor);
	DrawRectOutline(canvas, healthBarBackgroundRect, healthBarOutlineColor);
}

static I8* func GetAbilityName(I32 abilityId)
{
	I8* name = 0;
	switch(abilityId)
	{
		case SmallPunchAbilityId:
		{
			name = "Small Punch";
			break;
		}
		case BigPunchAbilityId:
		{
			name = "Big Punch";
			break;
		}
		case KickAbilityId:
		{
			name = "Kick";
			break;
		}
		case SpinningKickAbilityId:
		{
			name = "Spinning Kick";
			break;
		}
		case RollAbilityId:
		{
			name = "Roll";
			break;
		}
		case AvoidanceAbilityId:
		{
			name = "Avoidance";
			break;
		}
		case SwordStabAbilityId:
		{
			name = "Sword Stab";
			break;
		}
		case SwordSwingAbilityId:
		{
			name = "Sword Swing";
			break;
		}
		case RaiseShieldAbilityId:
		{
			name = "Raise Shield";
			break;
		}
		case BurnAbilityId:
		{
			name = "Burn";
			break;
		}
		case LightOfTheSunAbilityId:
		{
			name = "Light of the Sun";
			break;
		}
		case BlessingOfTheSunAbilityId:
		{
			name = "Blessing of the Sun";
			break;
		}
		case MercyOfTheSunAbilityId:
		{
			name = "Mercy of the Sun";
			break;
		}
		case EnemyAbilityId:
		default:
		{
			DebugBreak();
		}
	}
	Assert(name != 0);
	return name;
}

static String func GetAbilityTooltipText(I32 abilityId, I8* buffer, I32 bufferSize)
{
	String text = StartString(buffer, bufferSize);

	I8* abilityName = GetAbilityName(abilityId);
	AddLine(text, abilityName);

	F32 rechargeDuration = GetAbilityRechargeDuration(abilityId);
	if(rechargeDuration == 1.0f)
	{
		AddLine(text, "Recharge: 1 second");
	}
	else if(rechargeDuration > 0.0f)
	{
		AddLine(text, "Recharge: " + rechargeDuration + " seconds");
	}

	if(HasCooldown(abilityId))
	{
		F32 cooldownDuration = GetAbilityCooldownDuration(abilityId);
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

	switch(abilityId)
	{
		case SmallPunchAbilityId:
		{
			AddLine(text, "Melee range")
			AddLine(text, "Punch target enemy lightly, dealing");
			AddLine(text, "10 damage.");
			break;
		}
		case BigPunchAbilityId:
		{
			AddLine(text, "Melee range");
			AddLine(text, "Forcefully punch target enemy, dealing");
			AddLine(text, "30 damage.");
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
			AddLine(text, "Deal 5 damage to all enemies within");
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
			AddLine(text, "dealing 15 damage.");
			break;
		}
		case SwordSwingAbilityId:
		{
			AddLine(text, "Deal 10 damage to all enemies in front");
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
			AddLine(text, "Lasts for 60 seconds.");
			break;
		}
		case MercyOfTheSunAbilityId:
		{
			AddLine(text, "Plead for the mercy of the Sun, healing");
			AddLine(text, "you for 30 damage and blinding all");
			AddLine(text, "enemies within your melee range");
			AddLine(text, "for 5 seconds, making them unable to");
			AddLine(text, "use abilities while it lasts.");
			break;
		}
		case EnemyAbilityId:
		default:
		{
			DebugBreak();
		}
	}
	return text;
}

#define UIBoxSide 40
#define UIBoxPadding 5
static void func DrawUIBox(Bitmap* bitmap, I32 left, I32 right, I32 top, I32 bottom, F32 rechargeRatio, V4 color)
{
	V4 outlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	V4 rechargeColor = MakeColor(0.5f, 0.5f, 0.5f);

	DrawBitmapRect(bitmap, left, right, top, bottom, color);

	Assert(IsBetween(rechargeRatio, 0.0f, 1.0f));
	I32 rechargeRight = (I32)Lerp((F32)left, rechargeRatio, (F32)right);
	DrawBitmapRect(bitmap, left, rechargeRight, top, bottom, rechargeColor);

	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);
}

static void func DrawUIBoxWithText(Bitmap* bitmap, I32 left, I32 right, I32 top, I32 bottom, F32 rechargeRatio,
								   I8* text, GlyphData* glyphData, V4 color)
{
	DrawUIBox(bitmap, left, right, top, bottom, rechargeRatio, color);
	V4 textColor = MakeColor(1.0f, 1.0f, 1.0f);
	Assert(text != 0 && glyphData != 0);
	DrawBitmapTextLineCentered(bitmap, text, glyphData, left, right, top, bottom, textColor);
}

static void func DrawEffectUIBox(Bitmap* bitmap, Effect* effect, I32 top, I32 left)
{
	I32 right = left + UIBoxSide;
	I32 bottom = top + UIBoxSide;

	F32 totalDuration = GetEffectTotalDuration(effect->effectId);
	Assert(totalDuration > 0.0f);

	F32 durationRatio = (effect->timeRemaining / totalDuration);
	Assert(IsBetween(durationRatio, 0.0f, 1.0f));

	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawUIBox(bitmap, left, right, top, bottom, durationRatio, backgroundColor);
}

static void func DrawPlayerEffectBar(CombatLabState* labState)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	GlyphData* glyphData = labState->canvas.glyphData;
	Assert(glyphData != 0);

	Entity* player = &labState->entities[0];
	Assert(player->groupId == PlayerGroupId);

	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);

	I32 left = UIBoxPadding;
	I32 top = UIBoxPadding;

	for(I32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		if(effect->entity == player)
		{
			DrawEffectUIBox(bitmap, effect, top, left);
			left += UIBoxSide + UIBoxPadding;
		}
	}
}

static void func DrawTargetEffectBar(CombatLabState* labState)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	GlyphData* glyphData = labState->canvas.glyphData;
	Assert(glyphData != 0);

	Entity* player = &labState->entities[0];
	Assert(player->groupId == PlayerGroupId);

	Entity* target = player->target;

	if(target)
	{
		V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);

		I32 left = (bitmap->width - 1) - UIBoxPadding - UIBoxSide;
		I32 top = UIBoxPadding;

		for(I32 i = 0; i < labState->effectN; i++)
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

static void func DrawHelpBar(CombatLabState* labState, V2 mousePosition)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	GlyphData* glyphData = labState->canvas.glyphData;
	Assert(glyphData != 0);

	Camera* camera = &labState->camera;
	I32 mouseX = UnitXtoPixel(camera, mousePosition.x);
	I32 mouseY = UnitYtoPixel(camera, mousePosition.y);

	I32 right  = (bitmap->width - 1) - UIBoxPadding;
	I32 left   = right - UIBoxSide;
	I32 bottom = (bitmap->height - 1) - UIBoxPadding;
	I32 top    = bottom - UIBoxSide;

	V4 boxColor = MakeColor(0.0f, 0.0f, 0.0f);

	I8 textBuffer[8] = {};
	OneLineString(textBuffer, 8, "Help");

	DrawUIBoxWithText(bitmap, left, right, top, bottom, 0.0f, textBuffer, glyphData, boxColor);

	if(IsIntBetween(mouseX, left, right) && IsIntBetween(mouseY, top, bottom))
	{
		I8 tooltipBuffer[256] = {};
		String tooltip = StartString(tooltipBuffer, 256);
		AddLine(tooltip, "Key binds");
		AddLine(tooltip, "[W][A][S][D] or Arrows - Move");
		AddLine(tooltip, "[Tab] - Target closest enemy");
		AddLine(tooltip, "[1]-[7] - Use Ability");
		AddLine(tooltip, "[C] - Switch between classes Monk/Paladin");

		I32 tooltipBottom = top - UIBoxPadding;
		I32 tooltipRight = (bitmap->width - 1);
		DrawBitmapStringTooltipBottomRight(bitmap, tooltip, glyphData, tooltipBottom, tooltipRight);
	}
}

static void func DrawAbilityBar(CombatLabState* labState, V2 mousePosition)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	GlyphData* glyphData = labState->canvas.glyphData;
	Assert(glyphData != 0);

	Camera* camera = &labState->camera;
	I32 mouseX = UnitXtoPixel(camera, mousePosition.x);
	I32 mouseY = UnitYtoPixel(camera, mousePosition.y);

	Entity* entity = &labState->entities[0];
	Assert(entity->groupId == PlayerGroupId);
	I32 classId = entity->classId;
	Assert(classId != NoClassId);

	I32 abilityN = 0;
	for(I32 abilityId = 1; abilityId < AbilityN; abilityId++)
	{
		if(GetAbilityClass(abilityId) == classId)
		{
			abilityN++;
		}
	}

	I32 centerX = (bitmap->width - 1) / 2;
	I32 width = UIBoxSide * abilityN + UIBoxPadding * (abilityN - 1);

	I32 left   = centerX - width / 2;
	I32 right  = centerX + width / 2;
	I32 bottom = (bitmap->height - 1) - 30;
	I32 top    = bottom - UIBoxSide;

	I32 boxLeft = left;
	I32 abilityIndex = 0;

	labState->hoverAbilityId = NoAbilityId;

	for(I32 abilityId = 1; abilityId < AbilityN; abilityId++)
	{
		if(GetAbilityClass(abilityId) == classId)
		{
			I32 boxRight = boxLeft + UIBoxSide;
			I32 boxTop = top;
			I32 boxBottom = bottom;

			V4 boxBackgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
			V4 boxCannotUseColor = MakeColor(0.2f, 0.0f, 0.0f);

			V4 color = boxBackgroundColor;

			F32 recharge = 0.0f;
			F32 rechargeFrom = 0.0f;
			Cooldown* cooldown = GetCooldown(labState, entity, abilityId);
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

			F32 rechargeRatio = (rechargeFrom > 0.0f) ? (recharge / rechargeFrom) : 0.0f;

			I8 name[8] = {};
			OneLineString(name, 8, (abilityIndex + 1));

			DrawUIBoxWithText(bitmap, boxLeft, boxRight, boxTop, boxBottom, rechargeRatio, name, glyphData, color);

			if(IsIntBetween(mouseX, boxLeft, boxRight) && IsIntBetween(mouseY, boxTop, boxBottom))
			{
				I32 tooltipLeft = (boxLeft + boxRight) / 2 - TooltipWidth / 2;
				I32 tooltipBottom = boxTop - 5;

				static I8 tooltipBuffer[512] = {};
				String tooltipText = GetAbilityTooltipText(abilityId, tooltipBuffer, 512);
				DrawBitmapStringTooltipBottom(bitmap, tooltipText, glyphData, tooltipBottom, tooltipLeft);

				labState->hoverAbilityId = abilityId;
			}

			boxLeft = boxRight + UIBoxPadding;
			abilityIndex++;
		}
	}
}

static void func UpdateEntityRecharge(Entity* entity, F32 seconds)
{
	entity->recharge = Max2(entity->recharge - seconds, 0.0f);
}

static void func UpdateCooldowns(CombatLabState* labState, F32 seconds)
{
	I32 remainingCooldownN = 0;
	for(I32 i = 0; i < labState->cooldownN; i++)
	{
		Cooldown* cooldown = &labState->cooldowns[i];
		cooldown->timeRemaining -= seconds;
		if(cooldown->timeRemaining > 0.0f)
		{
			labState->cooldowns[remainingCooldownN] = *cooldown;
			remainingCooldownN++;
		}
	}
	labState->cooldownN = remainingCooldownN;
}

static void func UpdateEffects(CombatLabState* labState, F32 seconds)
{
	I32 remainingEffectN = 0;
	for(I32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		effect->timeRemaining -= seconds;
		if(effect->timeRemaining > 0.0f)
		{
			labState->effects[remainingEffectN] = *effect;
			remainingEffectN++;
		}
	}
	labState->effectN = remainingEffectN;

	for(I32 i = 0; i < labState->effectN; i++)
	{
		Effect* effect = &labState->effects[i];
		F32 time = effect->timeRemaining;
		F32 previousTime = time + seconds;
		I32 effectId = effect->effectId;
		switch(effectId)
		{
			case BurningEffectId:
			{
				if(Floor(time) != Floor(previousTime))
				{
					DealDamage(labState, effect->entity, 2);
				}
				break;
			}
		}
	}
}

static void func UpdateDamageDisplays(CombatLabState* labState, F32 seconds)
{
	F32 scrollUpSpeed = 1.0f;

	I32 remainingDisplayN = 0;
	for(I32 i = 0; i < labState->damageDisplayN; i++)
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

static B32 func CanMove(CombatLabState* labState, Entity* entity)
{
	B32 canMove = false;
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

static void func DrawDamageDisplays(CombatLabState* labState)
{
	V4 damageColor = MakeColor(1.0f, 1.0f, 0.0f);
	V4 healColor = MakeColor(0.0f, 0.5f, 0.0f);
	Canvas* canvas = &labState->canvas;
	for(I32 i = 0; i < labState->damageDisplayN; i++)
	{
		DamageDisplay* display = &labState->damageDisplays[i];
		I8 text[16] = {};
		V4 textColor = {};
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

static void func CombatLabUpdate(CombatLabState* labState, V2 mousePosition, F32 seconds)
{
	Canvas* canvas = &labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 tileColor = MakeColor(0.5f, 0.5f, 0.5f);
	V4 hoverTileColor = MakeColor(0.6f, 0.6f, 0.6f);

	V4 moveTileColor = MakeColor(0.8f, 0.8f, 0.8f);
	V4 hoverMoveTileColor = MakeColor(0.9f, 0.9f, 0.9f);

	V4 attackTileColor = MakeColor(0.8f, 0.5f, 0.5f);
	V4 hoverAttackTileColor = MakeColor(0.9f, 0.6f, 0.6f);

	for(I32 row = 0; row < TileRowN; row++)
	{
		for(I32 col = 0; col < TileColN; col++)
		{
			TileIndex tileIndex = MakeTileIndex(row, col);
			V4 color = tileColor;
			V2 tileCenter = GetTileCenter(tileIndex);
			Rect tileRect = MakeSquareRect(tileCenter, TileSide);
			DrawRect(canvas, tileRect, color);
			DrawRectOutline(canvas, tileRect, TileGridColor);
		}
	}

	F32 mapWidth = TileColN * TileSide;
	F32 mapHeight = TileRowN * TileSide;

	Entity* player = &labState->entities[0];
	Assert(player->groupId == PlayerGroupId);

	UpdateCooldowns(labState, seconds);
	UpdateEffects(labState, seconds);
	UpdateDamageDisplays(labState, seconds);

	if(IsDead(player))
	{
		player->velocity = MakeVector(0.0f, 0.0f);
	}
	if(CanMove(labState, player))
	{
		player->velocity = PlayerSpeed * player->inputDirection;
	}
	player->position = player->position + seconds * player->velocity;
	player->position.x = Clip(player->position.x, EntityRadius, mapWidth - EntityRadius);
	player->position.y = Clip(player->position.y, EntityRadius, mapHeight - EntityRadius);

	V4 playerColor = MakeColor(0.0f, 1.0f, 1.0f);
	DrawEntity(canvas, player, playerColor);

	V4 enemyColor = MakeColor(1.0f, 0.0f, 0.0f);

	for(I32 i = 0; i < EntityN; i++)
	{
		Entity* enemy = &labState->entities[i];
		if (enemy->groupId != EnemyGroupId)
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

	F32 enemyPullDistance = 30.0f;
	for(I32 i = 0; i < EntityN; i++)
	{
		Entity* enemy = &labState->entities[i];
		if (enemy->groupId != EnemyGroupId)
		{
			continue;
		}

		if(enemy->target == 0)
		{
			F32 distance = MaxDistance(player->position, enemy->position);
			if(distance <= enemyPullDistance)
			{
				enemy->target = player;
			}
		}

		Entity* target = enemy->target;
		if(target == 0)
		{
			if(CanMove(labState, enemy))
			{
				enemy->velocity = MakeVector(0.0f, 0.0f);
			}
		}
		else
		{
			B32 foundTargetTile = false;
			F32 enemyTargetDistance = 0.0f;

			V2 targetDirection = {};
			V2 targetPosition = {};
			TileIndex playerTileIndex = GetContainingTileIndex(target->position);

			for(I32 row = playerTileIndex.row - 1; row <= playerTileIndex.row + 1; row++)
			{
				V2 direction = {};
				direction.y = (F32)(row - playerTileIndex.row);

				for(I32 col = playerTileIndex.col - 1; col <= playerTileIndex.col + 1; col++)
				{
					direction.x = (F32)(col - playerTileIndex.col);

					if(row == playerTileIndex.row && col == playerTileIndex.col)
					{
						continue;
					}

					F32 attackDistance = 2.5f * EntityRadius;
					TileIndex tileIndex = MakeTileIndex(row, col);
					if(IsValidTileIndex(tileIndex))
					{
						V2 position = target->position + attackDistance * direction;
						F32 enemyDistance = MaxDistance(enemy->position, position);
						if(!foundTargetTile || enemyDistance < enemyTargetDistance)
						{
							targetDirection = direction;
							targetPosition = position;
							enemyTargetDistance = enemyDistance;
							foundTargetTile = true;
						}
					}

				}
			}

			Assert(foundTargetTile);
			F32 enemyMoveSpeed = 10.0f;

			if(IsDead(enemy))
			{
				enemy->velocity = MakeVector(0.0f, 0.0f);
			}
			if(CanMove(labState, enemy))
			{
				enemy->velocity = MakeVector(0.0f, 0.0f);
				if(enemy->position.x < targetPosition.x)
				{
					enemy->velocity.x = Min2(+enemyMoveSpeed, (targetPosition.x - enemy->position.x) / seconds);
				}
				else
				{
					enemy->velocity.x = Max2(-enemyMoveSpeed, (targetPosition.x - enemy->position.x) / seconds);
				}

				if(enemy->position.y < targetPosition.y)
				{
					enemy->velocity.y = Min2(+enemyMoveSpeed, (targetPosition.y - enemy->position.y) / seconds);
				}
				else
				{
					enemy->velocity.y = Max2(-enemyMoveSpeed, (targetPosition.y - enemy->position.y) / seconds);
				}
			}
		}
		enemy->position = enemy->position + seconds * enemy->velocity;
		enemy->position.x = Clip(enemy->position.x, EntityRadius, mapWidth - EntityRadius);
		enemy->position.y = Clip(enemy->position.y, EntityRadius, mapHeight - EntityRadius);
	}

	for(I32 i = 0; i < EntityN; i++)
	{
		Entity* entity = &labState->entities[i];
		UpdateEntityRecharge(entity, seconds);
	}

	for(I32 i = 0; i < EntityN; i++)
	{
		Entity* enemy = &labState->entities[i];
		if(enemy->groupId != EnemyGroupId)
		{
			continue;
		}
		AttemptToUseAbility(labState, enemy, EnemyAbilityId);
	}

	for(I32 i = 0; i < EntityN; i++)
	{
		Entity* entity = &labState->entities[i];
		DrawEntityBars(canvas, entity);
	}

	V2 mapCenter = MakePoint(mapWidth * 0.5f, mapHeight * 0.5f);
	Camera* camera = &labState->camera;
	camera->center = mapCenter;

	DrawAbilityBar(labState, mousePosition);
	DrawHelpBar(labState, mousePosition);
	DrawPlayerEffectBar(labState);
	DrawTargetEffectBar(labState);
	DrawDamageDisplays(labState);
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
	I32 width  = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	CombatLabInit(labState, width, height);

	LARGE_INTEGER counterFrequency;
	QueryPerformanceFrequency(&counterFrequency);

	LARGE_INTEGER lastCounter;
	QueryPerformanceCounter(&lastCounter);

	MSG message = {};
	while(labState->running)
	{
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		I64 microSeconds = counter.QuadPart - lastCounter.QuadPart;
		F32 milliSeconds = (F32(microSeconds) * 1000.0f) / F32(counterFrequency.QuadPart);
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

// TODO: Computer controlled enemies shouldn't stack upon each other
// TODO: Momentum and acceleration
// TODO: Remove limitation of 8 directions!
// TODO: Move with mouse?
// TODO: Heal others
// TODO: Target friendly entity
// TODO: Damage types
// TODO: Text display for effects
// TODO: Sword swing should only damage enemies in front of the paladin!
// TODO: Entity facing direction!
// TODO: Combat log