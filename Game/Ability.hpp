#pragma once

#include <Windows.h>

#include "Debug.hpp"

enum AbilityId
{
	NoAbilityId,

	LightningAbilityId,
	EarthShakeAbilityId,
	HealAbilityId,
	EarthShieldAbilityId,

	SmallPunchAbilityId,
	BigPunchAbilityId,
	KickAbilityId,
	SpinningKickAbilityId,
	RollAbilityId,
	AvoidanceAbilityId,

	SwordStabAbilityId,
	LightOfTheSunAbilityId,
	SwordSwingAbilityId,
	RaiseShieldAbilityId,
	BurnAbilityId,
	BlessingOfTheSunAbilityId,
	MercyOfTheSunAbilityId,

	SnakeStrikeAbilityId,

	CrocodileBiteAbilityId,
	CrocodileLashAbilityId,

	TigerBiteAbilityId,

	AbilityN
};

enum ClassId
{
	NoClassId,
	DruidClassId,
	MonkClassId,
	PaladinClassId,
	SnakeClassId,
	CrocodileClassId,
	TigerClassId
};

static ClassId
func GetAbilityClass(AbilityId abilityId)
{
	ClassId classId = NoClassId;
	switch(abilityId)
	{
		case LightningAbilityId:
		case EarthShakeAbilityId:
		case HealAbilityId:
		case EarthShieldAbilityId:
		{
			classId = DruidClassId;
			break;
		}
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
		case LightOfTheSunAbilityId:
		case SwordSwingAbilityId:
		case RaiseShieldAbilityId:
		case BurnAbilityId:
		case BlessingOfTheSunAbilityId:
		case MercyOfTheSunAbilityId:
		{
			classId = PaladinClassId;
			break;
		}
		case SnakeStrikeAbilityId:
		{
			classId = SnakeClassId;
			break;
		}
		case CrocodileBiteAbilityId:
		case CrocodileLashAbilityId:
		{
			classId = CrocodileClassId;
			break;
		}
		case TigerBiteAbilityId:
		{
			classId = TigerClassId;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return classId;
}

static Int32
func GetAbilityMinLevel(AbilityId abilityId)
{
	Int32 minLevel = 0;
	switch(abilityId)
	{
		case EarthShakeAbilityId:
		{
			minLevel = 2;
			break;
		}
		case HealAbilityId:
		{
			minLevel = 3;
			break;
		}
		case EarthShieldAbilityId:
		{
			minLevel = 4;
			break;
		}
		case SwordStabAbilityId:
		{
			minLevel = 1;
			break;
		}
		case LightOfTheSunAbilityId:
		{
			minLevel = 1;
			break;
		}
		case SwordSwingAbilityId:
		{
			minLevel = 2;
			break;
		}
		case RaiseShieldAbilityId:
		{
			minLevel = 2;
			break;
		}
		case BurnAbilityId:
		{
			minLevel = 3;
			break;
		}
		case BlessingOfTheSunAbilityId:
		{
			minLevel = 4;
			break;
		}
		case MercyOfTheSunAbilityId:
		{
			minLevel = 5;
			break;
		}
		case SmallPunchAbilityId:
		{
			minLevel = 1;
			break;
		}
		case BigPunchAbilityId:
		{
			minLevel = 2;
			break;
		}
		case KickAbilityId:
		{
			minLevel = 3;
			break;
		}
		case SpinningKickAbilityId:
		{
			minLevel = 3;
			break;
		}
		case RollAbilityId:
		{
			minLevel = 4;
			break;
		}
		case AvoidanceAbilityId:
		{
			minLevel = 5;
			break;
		}
		default:
		{
			minLevel = 1;
		}
	}
	return minLevel;
}

static Real32
func GetAbilityCastDuration(AbilityId abilityId)
{
	Assert(abilityId != NoAbilityId);
	Real32 castDuration = 0.0f;
	switch(abilityId)
	{
		case LightningAbilityId:
		{
			castDuration = 2.0f;
			break;
		}
		case HealAbilityId:
		{
			castDuration = 1.5f;
			break;
		}
		case LightOfTheSunAbilityId:
		{
			castDuration = 1.5f;
			break;
		}
		default:
		{
			castDuration = 0.0f;
			break;
		}
	}
	return castDuration;
}

static Bool32
func AbilityIsCasted(AbilityId abilityId)
{
	Real32 castDuration = GetAbilityCastDuration(abilityId);
	Bool32 isCasted = (castDuration > 0.0f);
	return isCasted;
}

static Real32
func GetAbilityCooldownDuration(AbilityId abilityId)
{
	Real32 cooldown = 0.0f;
	switch(abilityId)
	{
		case EarthShakeAbilityId:
		{
			cooldown = 10.0f;
			break;
		}
		case HealAbilityId:
		{
			cooldown = 5.0f;
			break;
		}
		case EarthShieldAbilityId:
		{
			cooldown = 30.0f;
			break;
		}
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
		case CrocodileBiteAbilityId:
		{
			cooldown = 5.0f;
			break;
		}
		default:
		{
			cooldown = 0.0f;
		}
	}
	return cooldown;
}

static Bool32
func AbilityHasCooldown(AbilityId abilityId)
{
	Real32 cooldownDuration = GetAbilityCooldownDuration(abilityId);
	Bool32 hasCooldown = (cooldownDuration > 0.0f);
	return hasCooldown;
}

static Real32
func GetAbilityRechargeDuration(AbilityId abilityId)
{
	Real32 recharge = 0.0f;
	switch(abilityId)
	{
		case SnakeStrikeAbilityId:
		{
			recharge = 3.0f;
			break;
		}
		case CrocodileLashAbilityId:
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
		case SmallPunchAbilityId:
		{
			recharge = 1.0f;
			break;
		}
		default:
		{
			recharge = 1.0f;
		}
	}
	return recharge;
}

static Int8 *
func GetAbilityName(AbilityId abilityId)
{
	Int8 *name = 0;
	switch(abilityId)
	{
		case LightningAbilityId:
		{
			name = "Lightning";
			break;
		}
		case EarthShakeAbilityId:
		{
			name = "Earth Shake";
			break;
		}
		case HealAbilityId:
		{
			name = "Heal";
			break;
		}
		case EarthShieldAbilityId:
		{
			name = "Earth Shield";
			break;
		}
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
		case SnakeStrikeAbilityId:
		{
			name = "Snake strike";
			break;
		}
		case CrocodileBiteAbilityId:
		{
			name = "Crocodile bite";
			break;
		}
		case CrocodileLashAbilityId:
		{
			name = "Crocodile Lash";
			break;
		}
		case TigerBiteAbilityId:
		{
			name = "Tiger bite";
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	Assert(name != 0);
	return name;
}
