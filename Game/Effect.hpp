#pragma once

#include "Debug.hpp"

enum EffectId
{
	NoEffectId,
	KickedEffectId,
	RollingEffectId,
	InvulnerableEffectId,
	ShieldRaisedEffectId,
	BurningEffectId,
	BlessingOfTheSunEffectId,
	BlindEffectId,
	PoisonedEffectId,
	BittenEffectId,
	EarthShakeEffectId,
	EarthShieldEffectId,
	RegenerateEffectId,
	IntellectPotionEffectId
};

static Int8*
func GetEffectName(Int32 effectId)
{
	Int8* name = 0;
	switch(effectId)
	{
		case KickedEffectId:
		{
			name = "Kicked";
			break;
		}
		case RollingEffectId:
		{
			name = "Rolling";
			break;
		}
		case InvulnerableEffectId:
		{
			name = "Invulnerable";
			break;
		}
		case ShieldRaisedEffectId:
		{
			name = "Shield raised";
			break;
		}
		case BurningEffectId:
		{
			name = "Burning";
			break;
		}
		case BlessingOfTheSunEffectId:
		{
			name = "Sun's Blessing";
			break;
		}
		case BlindEffectId:
		{
			name = "Blind";
			break;
		}
		case PoisonedEffectId:
		{
			name = "Poisoned";
			break;
		}
		case BittenEffectId:
		{
			name = "Bitten";
			break;
		}
		case EarthShakeEffectId:
		{
			name = "Earth Shake";
			break;
		}
		case EarthShieldEffectId:
		{
			name = "Earth Shield";
			break;
		}
		case RegenerateEffectId:
		{
			name = "Regenerate";
			break;
		}
		case IntellectPotionEffectId:
		{
			name = "Intellect Potion";
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return name;
}static Real32
func EffectHasDuration(Int32 effectId)
{
	bool hasDuration = true;
	switch(effectId)
	{
		case RegenerateEffectId:
		{
			hasDuration = false;
			break;
		}
		default:
		{
			hasDuration = true;
		}
	}
	return hasDuration;
}

static Real32
func GetEffectDuration(Int32 effectId)
{
	Assert(EffectHasDuration(effectId));
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
			duration = 5 * 60.0f;
			break;
		}
		case BlindEffectId:
		{
			duration = 5.0f;
			break;
		}
		case PoisonedEffectId:
		{
			duration = 60.0f;
			break;
		}
		case BittenEffectId:
		{
			duration = 5.0f;
			break;
		}
		case EarthShakeEffectId:
		{
			duration = 6.0f;
			break;
		}
		case EarthShieldEffectId:
		{
			duration = 5.0f;
			break;
		}
		case IntellectPotionEffectId:
		{
			duration = 10 * 60.0f;
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
