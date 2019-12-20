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
func GetAbilityClass(AbilityId ability_id)
{
	ClassId class_id = NoClassId;
	switch(ability_id)
	{
		case LightningAbilityId:
		case EarthShakeAbilityId:
		case HealAbilityId:
		case EarthShieldAbilityId:
		{
			class_id = DruidClassId;
			break;
		}
		case SmallPunchAbilityId:
		case BigPunchAbilityId:
		case KickAbilityId:
		case SpinningKickAbilityId:
		case RollAbilityId:
		case AvoidanceAbilityId:
		{
			class_id = MonkClassId;
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
			class_id = PaladinClassId;
			break;
		}
		case SnakeStrikeAbilityId:
		{
			class_id = SnakeClassId;
			break;
		}
		case CrocodileBiteAbilityId:
		case CrocodileLashAbilityId:
		{
			class_id = CrocodileClassId;
			break;
		}
		case TigerBiteAbilityId:
		{
			class_id = TigerClassId;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return class_id;
}

static I32
func GetAbilityMinLevel(AbilityId ability_id)
{
	I32 min_level = 0;
	switch(ability_id)
	{
		case EarthShakeAbilityId:
		{
			min_level = 2;
			break;
		}
		case HealAbilityId:
		{
			min_level = 3;
			break;
		}
		case EarthShieldAbilityId:
		{
			min_level = 4;
			break;
		}
		case SwordStabAbilityId:
		{
			min_level = 1;
			break;
		}
		case LightOfTheSunAbilityId:
		{
			min_level = 1;
			break;
		}
		case SwordSwingAbilityId:
		{
			min_level = 2;
			break;
		}
		case RaiseShieldAbilityId:
		{
			min_level = 2;
			break;
		}
		case BurnAbilityId:
		{
			min_level = 3;
			break;
		}
		case BlessingOfTheSunAbilityId:
		{
			min_level = 4;
			break;
		}
		case MercyOfTheSunAbilityId:
		{
			min_level = 5;
			break;
		}
		case SmallPunchAbilityId:
		{
			min_level = 1;
			break;
		}
		case BigPunchAbilityId:
		{
			min_level = 2;
			break;
		}
		case KickAbilityId:
		{
			min_level = 3;
			break;
		}
		case SpinningKickAbilityId:
		{
			min_level = 3;
			break;
		}
		case RollAbilityId:
		{
			min_level = 4;
			break;
		}
		case AvoidanceAbilityId:
		{
			min_level = 5;
			break;
		}
		default:
		{
			min_level = 1;
		}
	}
	return min_level;
}

static R32
func GetAbilityCastDuration(AbilityId ability_id)
{
	Assert(ability_id != NoAbilityId);
	R32 cast_duration = 0.0f;
	switch(ability_id)
	{
		case LightningAbilityId:
		{
			cast_duration = 2.0f;
			break;
		}
		case HealAbilityId:
		{
			cast_duration = 1.5f;
			break;
		}
		case LightOfTheSunAbilityId:
		{
			cast_duration = 1.5f;
			break;
		}
		default:
		{
			cast_duration = 0.0f;
			break;
		}
	}
	return cast_duration;
}

static B32
func AbilityIsCasted(AbilityId ability_id)
{
	R32 cast_duration = GetAbilityCastDuration(ability_id);
	B32 is_casted = (cast_duration > 0.0f);
	return is_casted;
}

static R32
func GetAbilityCooldownDuration(AbilityId ability_id)
{
	R32 cooldown = 0.0f;
	switch(ability_id)
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

static B32
func AbilityHasCooldown(AbilityId ability_id)
{
	R32 cooldown_duration = GetAbilityCooldownDuration(ability_id);
	B32 has_cooldown = (cooldown_duration > 0.0f);
	return has_cooldown;
}

static R32
func GetAbilityRechargeDuration(AbilityId ability_id)
{
	R32 recharge = 0.0f;
	switch(ability_id)
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

static I8 *
func GetAbilityName(AbilityId ability_id)
{
	I8 *name = 0;
	switch(ability_id)
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
