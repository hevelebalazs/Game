#pragma once

#include "../Map.hpp"
#include "../UserInput.hpp"

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

#define EntityRadius (2.0f)
#define EntityMaxHealth 100

enum EntityGroupId
{
	PlayerGroupId,
	EnemyGroupId
};

struct Entity
{
	Vec2 position;
	Vec2 velocity;

	Real32 recharge;
	Real32 rechargeFrom;
	Int32 health;

	Int32 classId;
	Int32 groupId;
	Entity* target;

	Vec2 inputDirection;
};

struct Cooldown
{
	Entity* entity;
	Int32 abilityId;
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

#define EntityN 5
#define MaxCooldownN 64
#define MaxEffectN 64
#define MaxDamageDisplayN 64
#define CombatLabArenaSize (1 * MegaByte)
struct CombatLabState
{
	Int8 arenaMemory[CombatLabArenaSize];
	MemArena arena;

	Map map;

	Cooldown cooldowns[MaxCooldownN];
	Int32 cooldownN;

	Effect effects[MaxEffectN];
	Int32 effectN;

	Entity entities[EntityN];

	DamageDisplay damageDisplays[MaxDamageDisplayN];
	Int32 damageDisplayN;

	Int32 hoverAbilityId;
};

#define PlayerSpeed 11.0f

#define EnemyAttackRadius 10.0f

#define MaxMeleeAttackDistance (4.0f * EntityRadius)

static void func CombatLabInit(CombatLabState* labState, Canvas* canvas)
{
	labState->arena = CreateMemArena(labState->arenaMemory, CombatLabArenaSize);
	labState->map = GenerateForestMap(&labState->arena);

	canvas->glyphData = GetGlobalGlyphData();

	Camera* camera = canvas->camera;
	camera->unitInPixels = 5.0f;
	camera->center = MakePoint(0.0f, 0.0f);

	Real32 mapWidth  = GetMapWidth(&labState->map);
	Real32 mapHeight = GetMapHeight(&labState->map);
	Vec2 mapCenter = MakePoint(mapWidth * 0.5f, mapHeight * 0.5f);

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

#define TileGridColor MakeColor(0.2f, 0.2f, 0.2f)

static Bool32 func IsOnCooldown(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Bool32 isOnCooldown = false;
	for(Int32 i = 0; i < labState->cooldownN; i++)
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

static Bool32 func IsDead(Entity* entity)
{
	Bool32 isDead = (entity->health == 0);
	return isDead;
}

static Int32 func GetAbilityClass(Int32 abilityId)
{
	Int32 classId = NoClassId;
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

static Bool32 func HasEffect(CombatLabState* labState, Entity* entity, Int32 effectId)
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

static Bool32 func AbilityIsEnabled(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Bool32 canUse = false;
	Entity* target = entity->target;

	Int32 abilityClassId = GetAbilityClass(abilityId);

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
	}
	else if(HasEffect(labState, entity, BlindEffectId))
	{
		canUse = false;
	}

	return canUse;
}

static Bool32 func CanUseAbility(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Bool32 canUseAbility = (entity->recharge == 0.0f && !IsOnCooldown(labState, entity, abilityId) &&
							AbilityIsEnabled(labState, entity, abilityId));
	return canUseAbility;
}

static void func PutOnRecharge(Entity* entity, Real32 rechargeTime)
{
	Assert(rechargeTime > 0.0f);
	Assert(entity->recharge == 0.0f);
	entity->rechargeFrom = rechargeTime;
	entity->recharge = rechargeTime;
}

static Bool32 func CanDamage(CombatLabState* labState, Entity* entity)
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

static void func AddDamageDisplay(CombatLabState* labState, Vec2 position, Int32 damage)
{
	Assert(labState->damageDisplayN < MaxDamageDisplayN);
	DamageDisplay* display = &labState->damageDisplays[labState->damageDisplayN];
	labState->damageDisplayN++;
	display->position = position;
	display->damage = damage;
	display->timeRemaining = DamageDisplayDuration;
}

static void func DealFinalDamage(Entity* entity, Int32 damage)
{
	Assert(damage > 0);
	entity->health = IntMax2(entity->health - damage, 0);
}

static void func DealDamage(CombatLabState* labState, Entity* entity, Int32 damage)
{
	if(CanDamage(labState, entity))
	{
		Int32 finalDamage = damage;
		if(HasEffect(labState, entity, ShieldRaisedEffectId))
		{
			finalDamage = damage / 2;
		}

		DealFinalDamage(entity, finalDamage);
		AddDamageDisplay(labState, entity->position, finalDamage);
	}
}

static void func Heal(CombatLabState* labState, Entity* entity, Int32 damage)
{
	Assert(!IsDead(entity));
	entity->health = IntMin2(entity->health + damage, EntityMaxHealth);
	AddDamageDisplay(labState, entity->position, -damage);
}

static Real32 func GetAbilityCooldownDuration(Int32 abilityId)
{
	Real32 cooldown = 0.0f;
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

static Bool32 func HasCooldown(Int32 abilityId)
{
	Real32 cooldownDuration = GetAbilityCooldownDuration(abilityId);
	Bool32 hasCooldown = (cooldownDuration > 0.0f);
	return hasCooldown;
}

static void func AddCooldown(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Assert(HasCooldown(abilityId));
	Real32 duration = GetAbilityCooldownDuration(abilityId);

	Assert(duration > 0.0f);
	Assert(labState->cooldownN < MaxCooldownN);
	Cooldown* cooldown = &labState->cooldowns[labState->cooldownN];
	labState->cooldownN++;

	cooldown->entity = entity;
	cooldown->abilityId = abilityId;
	cooldown->timeRemaining = duration;
}

static Cooldown* func GetCooldown(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	Cooldown* result = 0;
	for(Int32 i = 0; i < labState->cooldownN; i++)
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

static Vec2 func GetClosestMoveDirection(Vec2 distance)
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

static Real32 func GetEffectTotalDuration(Int32 effectId)
{
	Real32 duration = 0.0f;
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

static void func RemoveEffect(CombatLabState* labState, Entity* entity, Int32 effectId)
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
}

static void func AddEffect(CombatLabState* labState, Entity* entity, Int32 effectId)
{
	Assert(labState->effectN < MaxEffectN);
	Effect* effect = &labState->effects[labState->effectN];
	labState->effectN++;

	effect->entity = entity;
	effect->effectId = effectId;
	
	Real32 duration = GetEffectTotalDuration(effectId);
	Assert(duration > 0.0f);
	effect->timeRemaining = duration;
}

static Real32 func GetAbilityRechargeDuration(Int32 abilityId)
{
	Real32 recharge = 0.0f;
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

static void func UseAbility(CombatLabState* labState, Entity* entity, Int32 abilityId)
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
						DealDamage(labState, target, 10);
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
			Int32 damage = 15;
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
			Int32 damage = 10;
			if(HasEffect(labState, entity, BlessingOfTheSunEffectId))
			{
				damage += 2;
			}

			for(Int32 i = 0; i < EntityN; i++)
			{
				Entity* target = &labState->entities[i];
				if(target->groupId != entity->groupId)
				{
					Real32 distance = Distance(entity->position, target->position);
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

	Real32 rechargeDuration = GetAbilityRechargeDuration(abilityId);
	if(rechargeDuration > 0.0f)
	{
		PutOnRecharge(entity, rechargeDuration);
	}

	if(HasCooldown(abilityId))
	{
		AddCooldown(labState, entity, abilityId);
	}
}

static void func AttemptToUseAbility(CombatLabState* labState, Entity* entity, Int32 abilityId)
{
	if(CanUseAbility(labState, entity, abilityId))
	{
		UseAbility(labState, entity, abilityId);
	}
}

static Int32 func GetClassAbilityOfIndex(Int32 classId, Int32 abilityIndex)
{
	Int32 result = NoAbilityId;
	Int32 classAbilityIndex = 0;
	for(Int32 abilityId = 1; abilityId < AbilityN; abilityId++)
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

static void func AttemptToUseAbilityOfIndex(CombatLabState* labState, Entity* entity, Int32 abilityIndex)
{
	Int32 abilityId = GetClassAbilityOfIndex(entity->classId, abilityIndex);
	if(abilityId != NoAbilityId)
	{
		AttemptToUseAbility(labState, entity, abilityId);
	}
}

static void func DrawCircle(Canvas* canvas, Vec2 center, Real32 radius, Vec4 color)
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

static void func DrawCircleOutline(Canvas* canvas, Vec2 center, Real32 radius, Vec4 color)
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

static void func DrawEntity(Canvas* canvas, Entity* entity, Vec4 color)
{
	Vec4 entityOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawCircle(canvas, entity->position, EntityRadius, color);
	DrawCircleOutline(canvas, entity->position, EntityRadius, entityOutlineColor);
}

static void func DrawSelectedEntity(Canvas* canvas, Entity* entity, Vec4 color)
{
	Vec4 selectionOutlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawCircle(canvas, entity->position, EntityRadius, color);
	DrawCircleOutline(canvas, entity->position, EntityRadius, selectionOutlineColor);	
}

static void func DrawEntityBars(Canvas* canvas, Entity* entity)
{
	Vec2 healthBarCenter = entity->position + MakeVector(0.0f, -1.5f * EntityRadius);
	Real32 healthBarWidth = 2.0f * EntityRadius;
	Real32 healthBarHeight = 0.5f * EntityRadius;
	Vec4 healthBarBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
	Vec4 healthBarFilledColor = MakeColor(1.0f, 0.0f, 0.0f);
	Vec4 healthBarOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);
	Real32 healthRatio = Real32(entity->health) / Real32(EntityMaxHealth);
	Assert(IsBetween(healthRatio, 0.0f, 1.0f));
	Rect healthBarBackgroundRect = MakeRect(healthBarCenter, healthBarWidth, healthBarHeight);
	DrawRect(canvas, healthBarBackgroundRect, healthBarBackgroundColor);
	Rect healthBarFilledRect = healthBarBackgroundRect;
	healthBarFilledRect.right = Lerp(healthBarFilledRect.left, healthRatio, healthBarFilledRect.right);
	DrawRect(canvas, healthBarFilledRect, healthBarFilledColor);
	DrawRectOutline(canvas, healthBarBackgroundRect, healthBarOutlineColor);
}

static Int8* func GetAbilityName(Int32 abilityId)
{
	Int8* name = 0;
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

static String func GetAbilityTooltipText(Int32 abilityId, Int8* buffer, Int32 bufferSize)
{
	String text = StartString(buffer, bufferSize);

	Int8* abilityName = GetAbilityName(abilityId);
	AddLine(text, abilityName);

	Real32 rechargeDuration = GetAbilityRechargeDuration(abilityId);
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
static void func DrawUIBox(Bitmap* bitmap, Int32 left, Int32 right, Int32 top, Int32 bottom, Real32 rechargeRatio, Vec4 color)
{
	Vec4 outlineColor = MakeColor(1.0f, 1.0f, 1.0f);
	Vec4 rechargeColor = MakeColor(0.5f, 0.5f, 0.5f);

	DrawBitmapRect(bitmap, left, right, top, bottom, color);

	Assert(IsBetween(rechargeRatio, 0.0f, 1.0f));
	Int32 rechargeRight = (Int32)Lerp((Real32)left, rechargeRatio, (Real32)right);
	DrawBitmapRect(bitmap, left, rechargeRight, top, bottom, rechargeColor);

	DrawBitmapRectOutline(bitmap, left, right, top, bottom, outlineColor);
}

static void func DrawUIBoxWithText(Bitmap* bitmap, Int32 left, Int32 right, Int32 top, Int32 bottom, Real32 rechargeRatio,
								   Int8* text, GlyphData* glyphData, Vec4 color)
{
	DrawUIBox(bitmap, left, right, top, bottom, rechargeRatio, color);
	Vec4 textColor = MakeColor(1.0f, 1.0f, 1.0f);
	Assert(text != 0 && glyphData != 0);
	DrawBitmapTextLineCentered(bitmap, text, glyphData, left, right, top, bottom, textColor);
}

static void func DrawEffectUIBox(Bitmap* bitmap, Effect* effect, Int32 top, Int32 left)
{
	Int32 right = left + UIBoxSide;
	Int32 bottom = top + UIBoxSide;

	Real32 totalDuration = GetEffectTotalDuration(effect->effectId);
	Assert(totalDuration > 0.0f);

	Real32 durationRatio = (effect->timeRemaining / totalDuration);
	Assert(IsBetween(durationRatio, 0.0f, 1.0f));

	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawUIBox(bitmap, left, right, top, bottom, durationRatio, backgroundColor);
}

static void func DrawPlayerEffectBar(Canvas* canvas, CombatLabState* labState)
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

static void func DrawTargetEffectBar(Canvas* canvas, CombatLabState* labState)
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

static void func DrawHelpBar(Canvas* canvas, CombatLabState* labState, IntVec2 mousePosition)
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

	if(IsIntBetween(mousePosition.x, left, right) && IsIntBetween(mousePosition.y, top, bottom))
	{
		Int8 tooltipBuffer[256] = {};
		String tooltip = StartString(tooltipBuffer, 256);
		AddLine(tooltip, "Key binds");
		AddLine(tooltip, "[W][A][S][D] or Arrows - Move");
		AddLine(tooltip, "[Tab] - Target closest enemy");
		AddLine(tooltip, "[1]-[7] - Use Ability");
		AddLine(tooltip, "[C] - Switch between classes Monk/Paladin");

		Int32 tooltipBottom = top - UIBoxPadding;
		Int32 tooltipRight = (bitmap->width - 1);
		DrawBitmapStringTooltipBottomRight(bitmap, tooltip, glyphData, tooltipBottom, tooltipRight);
	}
}

static void func DrawAbilityBar(Canvas* canvas, CombatLabState* labState, IntVec2 mousePosition)
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
		if(GetAbilityClass(abilityId) == classId)
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
		if(GetAbilityClass(abilityId) == classId)
		{
			Int32 boxRight = boxLeft + UIBoxSide;
			Int32 boxTop = top;
			Int32 boxBottom = bottom;

			Vec4 boxBackgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
			Vec4 boxCannotUseColor = MakeColor(0.2f, 0.0f, 0.0f);

			Vec4 color = boxBackgroundColor;

			Real32 recharge = 0.0f;
			Real32 rechargeFrom = 0.0f;
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

			Real32 rechargeRatio = (rechargeFrom > 0.0f) ? (recharge / rechargeFrom) : 0.0f;

			Int8 name[8] = {};
			OneLineString(name, 8, (abilityIndex + 1));

			DrawUIBoxWithText(bitmap, boxLeft, boxRight, boxTop, boxBottom, rechargeRatio, name, glyphData, color);

			if(IsIntBetween(mousePosition.x, boxLeft, boxRight) && IsIntBetween(mousePosition.y, boxTop, boxBottom))
			{
				Int32 tooltipLeft = (boxLeft + boxRight) / 2 - TooltipWidth / 2;
				Int32 tooltipBottom = boxTop - 5;

				static Int8 tooltipBuffer[512] = {};
				String tooltipText = GetAbilityTooltipText(abilityId, tooltipBuffer, 512);
				DrawBitmapStringTooltipBottom(bitmap, tooltipText, glyphData, tooltipBottom, tooltipLeft);

				labState->hoverAbilityId = abilityId;
			}

			boxLeft = boxRight + UIBoxPadding;
			abilityIndex++;
		}
	}
}

static void func UpdateEntityRecharge(Entity* entity, Real32 seconds)
{
	entity->recharge = Max2(entity->recharge - seconds, 0.0f);
}

static void func UpdateCooldowns(CombatLabState* labState, Real32 seconds)
{
	Int32 remainingCooldownN = 0;
	for(Int32 i = 0; i < labState->cooldownN; i++)
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

static void func UpdateEffects(CombatLabState* labState, Real32 seconds)
{
	Int32 remainingEffectN = 0;
	for(Int32 i = 0; i < labState->effectN; i++)
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
					DealDamage(labState, effect->entity, 2);
				}
				break;
			}
		}
	}
}

static void func UpdateDamageDisplays(CombatLabState* labState, Real32 seconds)
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

static Bool32 func CanMove(CombatLabState* labState, Entity* entity)
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

static void func DrawDamageDisplays(Canvas* canvas, CombatLabState* labState)
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

static void func CombatLabUpdate(CombatLabState* labState, Canvas* canvas, Real32 seconds, UserInput* userInput)
{
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	DrawMap(canvas, &labState->map);

	Real32 mapWidth  = GetMapWidth(&labState->map);
	Real32 mapHeight = GetMapHeight(&labState->map);

	Entity* player = &labState->entities[0];
	Assert(player->groupId == PlayerGroupId);

	if(WasKeyReleased(userInput, 'C'))
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
	}

	UpdateCooldowns(labState, seconds);
	UpdateEffects(labState, seconds);
	UpdateDamageDisplays(labState, seconds);

	player->inputDirection = MakeVector(0.0f, 0.0f);
	Bool32 inputMoveLeft  = IsKeyDown(userInput, 'A') || IsKeyDown(userInput, VK_LEFT);
	Bool32 inputMoveRight = IsKeyDown(userInput, 'D') || IsKeyDown(userInput, VK_RIGHT);
	Bool32 inputMoveUp    = IsKeyDown(userInput, 'W') || IsKeyDown(userInput, VK_UP);
	Bool32 inputMoveDown  = IsKeyDown(userInput, 'S') || IsKeyDown(userInput, VK_DOWN);

	Map* map = &labState->map;

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
		player->velocity = PlayerSpeed * player->inputDirection;
	}
	player->position = player->position + seconds * player->velocity;
	player->position.x = Clip(player->position.x, EntityRadius, mapWidth - EntityRadius);
	player->position.y = Clip(player->position.y, EntityRadius, mapHeight - EntityRadius);

	Vec4 playerColor = MakeColor(0.0f, 1.0f, 1.0f);
	DrawEntity(canvas, player, playerColor);

	Vec4 enemyColor = MakeColor(1.0f, 0.0f, 0.0f);

	for(Int32 i = 0; i < EntityN; i++)
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
			Real32 distance = MaxDistance(player->position, enemy->position);
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
			Bool32 foundTargetTile = false;
			Real32 enemyTargetDistance = 0.0f;

			Vec2 targetDirection = {};
			Vec2 targetPosition = {};
			TileIndex playerTileIndex = GetContainingTileIndex(map, target->position);

			for(Int32 row = playerTileIndex.row - 1; row <= playerTileIndex.row + 1; row++)
			{
				Vec2 direction = {};
				direction.y = (Real32)(row - playerTileIndex.row);

				for(Int32 col = playerTileIndex.col - 1; col <= playerTileIndex.col + 1; col++)
				{
					direction.x = (Real32)(col - playerTileIndex.col);

					if(row == playerTileIndex.row && col == playerTileIndex.col)
					{
						continue;
					}

					Real32 attackDistance = 2.5f * EntityRadius;
					TileIndex tileIndex = MakeTileIndex(row, col);
					if(IsValidTileIndex(map, tileIndex))
					{
						Vec2 position = target->position + attackDistance * direction;
						Real32 enemyDistance = MaxDistance(enemy->position, position);
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
			Real32 enemyMoveSpeed = 10.0f;

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

	if(WasKeyReleased(userInput, VK_LBUTTON))
	{
		if(labState->hoverAbilityId != NoAbilityId)
		{
			AttemptToUseAbility(labState, player, labState->hoverAbilityId);
		}
		else
		{
			Vec2 mousePosition = PixelToUnit(canvas->camera, userInput->mousePixelPosition);
			for(Int32 i = 0; i < EntityN; i++)
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
			AttemptToUseAbilityOfIndex(labState, player, i - '1');
		}
	}

	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* enemy = &labState->entities[i];
		if(enemy->groupId != EnemyGroupId)
		{
			continue;
		}
		AttemptToUseAbility(labState, enemy, EnemyAbilityId);
	}

	for(Int32 i = 0; i < EntityN; i++)
	{
		Entity* entity = &labState->entities[i];
		DrawEntityBars(canvas, entity);
	}

	Vec2 mapCenter = MakePoint(mapWidth * 0.5f, mapHeight * 0.5f);
	canvas->camera->center = player->position;

	DrawAbilityBar(canvas, labState, userInput->mousePixelPosition);
	DrawHelpBar(canvas, labState, userInput->mousePixelPosition);
	DrawPlayerEffectBar(canvas, labState);
	DrawTargetEffectBar(canvas, labState);
	DrawDamageDisplays(canvas, labState);
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