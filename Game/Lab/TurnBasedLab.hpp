#pragma once

#include "Lab.hpp"

#define TileSide 10.0f
#define TileRowN 10
#define TileColN 10

struct TileIndex
{
	I32 row;
	I32 col;
};

#define TurnBasedEntityN 4
#define TurnBasedTeamN 2
#define MaxActionPoints 5

V4 TeamColors[TurnBasedTeamN] =
{
	MakeColor (0.8f, 0.0f, 0.0f),
	MakeColor (0.0f, 0.0f, 0.8f)
};

V4 HoverTeamColors[TurnBasedTeamN] = 
{
	MakeColor (0.9f, 0.1f, 0.1f),
	MakeColor (0.1f, 0.0f, 0.9f)
};

struct TurnBasedEntity
{
	TileIndex tileIndex;
	I32 teamIndex;
	I32 actionPoints;
	I32 healthPoints;
	I32 maxHealthPoints;
};

enum TurnBasedAbilityId
{
	SmallPunchAbilityId,
	BigPunchAbilityId,
	KickAbilityId,
	SpinningKickAbilityId,
	AvoidanceAbilityId,
	RollAbilityId,
	TurnBasedAbilityN
};

struct TurnBasedAbility
{
	I32 id;
	I32 maxDistance;
	I32 damage;
	I32 actionPoints;
	I32 cooldown;
};
TurnBasedAbility gTurnBasedAbilities[TurnBasedAbilityN];

enum TurnBasedEffectId
{
	InvulnerableEffectId,
};

struct TurnBasedEffect
{
	TurnBasedEntity* entity;
	I32 id;
	I32 turns;
};
#define TurnBasedEffectN 64

struct TurnBasedCooldown
{
	TurnBasedEntity* entity;
	TurnBasedAbility* ability;
	I32 turns;
};
#define TurnBasedCooldownN 64

struct TurnBasedLabState
{
	Camera camera;
	Canvas canvas;
	B32 running;

	TurnBasedEntity entities[TurnBasedEntityN];
	TileIndex hoverTileIndex;

	TurnBasedEntity* selectedEntity;
	I32 activeTeamIndex;
	TurnBasedAbility* selectedAbility;

	TurnBasedAbility* hoverAbility;

	TurnBasedCooldown cooldowns[TurnBasedCooldownN];
	I32 cooldownN;

	TurnBasedEffect effects[TurnBasedEffectN];
	I32 effectN;
};
TurnBasedLabState gTurnBasedLabState;

static void func TurnBasedLabResize (TurnBasedLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera (camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap (&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 5.0f;
	camera->center = MakePoint (0.0f, 0.0f);
}

static void func TurnBasedLabBlit (Canvas* canvas, HDC context, RECT rect)
{
	I32 width  = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	Bitmap* bitmap = &canvas->bitmap;
	BITMAPINFO bitmapInfo = GetBitmapInfo (bitmap);
	StretchDIBits (context,
				   0, 0, bitmap->width, bitmap->height,
				   0, 0, width, height,
				   bitmap->memory,
				   &bitmapInfo,
				   DIB_RGB_COLORS,
				   SRCCOPY
	);
}

B32 operator== (TileIndex index1, TileIndex index2)
{
	B32 result = (index1.row == index2.row && 
				  index1.col == index2.col);
	return result;
}

static TileIndex func MakeTileIndex (I32 row, I32 col)
{
	TileIndex index = {};
	index.row = row;
	index.col = col;
	return index;
}

static B32 func IsValidTileIndex (TileIndex index)
{
	B32 result = (IsIntBetween (index.row, 0, TileRowN - 1) &&
				  IsIntBetween (index.col, 0, TileColN - 1));
	return result;
}

static V2 func GetTileCenter (TileIndex index)
{
	Assert (IsValidTileIndex (index));
	F32 x = F32 (index.col) * TileSide + TileSide * 0.5f;
	F32 y = F32 (index.row) * TileSide + TileSide * 0.5f;
	V2 position = MakePoint (x, y);
	return position;
}

static TileIndex func GetContainingTileIndex (V2 point)
{
	I32 row = Floor (point.y / TileSide);
	I32 col = Floor (point.x / TileSide);
	TileIndex index = MakeTileIndex (row, col);
	return index;
}

static B32 func IsValidTeamIndex(I32 index)
{
	B32 isValid = IsIntBetween (index, 0, TurnBasedTeamN - 1);
	return isValid;
}

static void func EndTurn (TurnBasedLabState* labState)
{
	for (I32 i = 0; i < TurnBasedEntityN; i++)
	{
		TurnBasedEntity* entity = &labState->entities[i];
		if (entity->teamIndex == labState->activeTeamIndex)
		{
			entity->actionPoints = MaxActionPoints;
		}
	}

	labState->activeTeamIndex++;
	if (labState->activeTeamIndex == TurnBasedTeamN)
	{
		labState->activeTeamIndex = 0;
	}
	Assert (IsValidTeamIndex (labState->activeTeamIndex));
}

static void func TurnBasedLabInit (TurnBasedLabState* labState, I32 windowWidth, I32 windowHeight)
{
	labState->running = true;
	labState->canvas.glyphData = GetGlobalGlyphData ();
	TurnBasedLabResize (labState, windowWidth, windowHeight);

	TurnBasedEntity* entities = labState->entities;
	entities[0].tileIndex = MakeTileIndex (0, 0);
	entities[1].tileIndex = MakeTileIndex (0, TileColN - 1);
	entities[2].tileIndex = MakeTileIndex (TileRowN - 1, 0);
	entities[3].tileIndex = MakeTileIndex (TileRowN - 1, TileColN - 1);

	entities[0].teamIndex = 0;
	entities[1].teamIndex = 0;
	entities[2].teamIndex = 1;
	entities[3].teamIndex = 1;

	labState->activeTeamIndex = 0;
	labState->selectedEntity = 0;
	labState->selectedAbility = 0;

	labState->hoverAbility = 0;

	for (I32 i = 0; i < TurnBasedEntityN; i++)
	{
		TurnBasedEntity* entity = &labState->entities[i];

		entity->actionPoints = MaxActionPoints;

		entity->healthPoints = 10;
		entity->maxHealthPoints = 10;
	}

	{
		TurnBasedAbility* smallPunch = &gTurnBasedAbilities[SmallPunchAbilityId];
		smallPunch->id = SmallPunchAbilityId;
		smallPunch->actionPoints = 1;
		smallPunch->damage = 1;
		smallPunch->maxDistance = 1;
	}
	{
		TurnBasedAbility* bigPunch = &gTurnBasedAbilities[BigPunchAbilityId];
		bigPunch->id = BigPunchAbilityId;
		bigPunch->actionPoints = 3;
		bigPunch->damage = 5;
		bigPunch->maxDistance = 1;
		bigPunch->cooldown = 1;
	}
	{
		TurnBasedAbility* kick = &gTurnBasedAbilities[KickAbilityId];
		kick->id = KickAbilityId;
		kick->actionPoints = 3;
		kick->damage = 2;
		kick->maxDistance = 1;
		kick->cooldown = 3;
	}
	{
		TurnBasedAbility* spinningKick = &gTurnBasedAbilities[SpinningKickAbilityId];
		spinningKick->id = SpinningKickAbilityId;
		spinningKick->actionPoints = 2;
		spinningKick->damage = 1;
	}
	{
		TurnBasedAbility* avoidance = &gTurnBasedAbilities[AvoidanceAbilityId];
		avoidance->id = AvoidanceAbilityId;
		avoidance->actionPoints = 5;
		avoidance->cooldown = 5;
		// TODO: become invulnerable for 1 turn
	}
	{
		TurnBasedAbility* roll = &gTurnBasedAbilities[RollAbilityId];
		roll->id = RollAbilityId;
		roll->actionPoints = 3;
		roll->cooldown = 3;
		roll->maxDistance = 1;
		roll->damage = 1;
	}

	labState->cooldownN = 0;
}

static I32 func GetEffectTurns (I32 effectId)
{
	I32 turns = 0;
	switch (effectId)
	{
		case InvulnerableEffectId:
		{
			turns = 1;
			break;
		}
		default:
		{
			DebugBreak ();
		}
	}
	return turns;
}

static B32 func IsDead (TurnBasedEntity* entity)
{
	Assert (entity != 0);
	Assert (entity->healthPoints >= 0);
	B32 isDead = (entity->healthPoints == 0);
	return isDead;
}

static void func AddEffect (TurnBasedLabState* labState, TurnBasedEntity* entity, I32 effectId)
{
	Assert (!IsDead (entity));
	TurnBasedEffect effect = {};
	effect.entity = entity;
	effect.id = effectId;
	effect.turns = GetEffectTurns (effectId);
	
	Assert (labState->effectN < TurnBasedEffectN);
	labState->effects[labState->effectN] = effect;
	labState->effectN++;
}

static TurnBasedEntity* func GetEntityAtTile (TurnBasedLabState* labState, TileIndex index)
{
	TurnBasedEntity* result = 0;
	if (IsValidTileIndex (index))
	{
		for (I32 i = 0; i < TurnBasedEntityN; i++)
		{
			TurnBasedEntity* entity = &labState->entities[i];
			if (entity->tileIndex == index)
			{
				result = entity;
				break;
			}
		}
	}
	return result;
}

static B32 func IsValidEntityIndex (I32 index)
{
	B32 isValid = IsIntBetween (index, 0, TurnBasedEntityN - 1);
	return isValid;
}

static I32 func GetTileDistance (TileIndex tileIndex1, TileIndex tileIndex2)
{
	Assert (IsValidTileIndex (tileIndex1));
	Assert (IsValidTileIndex (tileIndex2));
	I32 rowDistance = IntAbs (tileIndex1.row - tileIndex2.row);
	I32 colDistance = IntAbs (tileIndex1.col - tileIndex2.col);
	I32 distance = IntMax2 (rowDistance, colDistance);
	return distance;
}

static B32 func EntityCanMoveTo (TurnBasedLabState* labState, TurnBasedEntity* entity, TileIndex tileIndex)
{
	B32 canMove = false;
	if (entity != 0 && IsValidTileIndex (tileIndex) && !IsDead (entity))
	{
		if (GetTileDistance (entity->tileIndex, tileIndex) <= entity->actionPoints)
		{
			canMove = (GetEntityAtTile (labState, tileIndex) == 0);
		}
	}
	return canMove;
}

static void func MoveEntityTo (TurnBasedLabState* labState, TurnBasedEntity* entity, TileIndex tileIndex)
{
	Assert (EntityCanMoveTo (labState, entity, tileIndex));

	I32 moveDistance = GetTileDistance (entity->tileIndex, tileIndex);
	entity->tileIndex = tileIndex;

	Assert (entity->actionPoints >= moveDistance);
	entity->actionPoints -= moveDistance;
}

static B32 func IsOnCooldown (TurnBasedLabState* labState, TurnBasedEntity* entity, TurnBasedAbility* ability)
{
	B32 isOnCooldown = false;
	for (I32 i = 0; i < labState->cooldownN; i++)
	{
		TurnBasedCooldown* cooldown = &labState->cooldowns[i];
		if (cooldown->entity == entity && cooldown->ability == ability)
		{
			isOnCooldown = true;
			break;
		}
	}
	return isOnCooldown;
}

static B32 func CanRepeatAbility (TurnBasedAbility* ability)
{
	Assert (ability != 0);
	B32 canRepeat = (ability->id == SmallPunchAbilityId);
	return canRepeat;
}

static void func AddCooldown (TurnBasedLabState* labState, TurnBasedEntity* entity, TurnBasedAbility* ability)
{
	Assert (!IsOnCooldown(labState, entity, ability));
	Assert (!CanRepeatAbility(ability));

	Assert (labState->cooldownN < TurnBasedCooldownN);
	TurnBasedCooldown cooldown = {};
	cooldown.entity = entity;
	cooldown.ability = ability;
	cooldown.turns = ability->cooldown;
	labState->cooldowns[labState->cooldownN] = cooldown;
	labState->cooldownN++;
}

static TileIndex func GetLastEmptyTileAtMaxDistance (TurnBasedLabState* labState,
													 TileIndex startIndex, 
													 I32 directionRow, I32 directionCol, 
													 I32 maxDistance)
{
	Assert (IsIntBetween (directionRow, -1, +1));
	Assert (IsIntBetween (directionCol, -1, +1));

	TileIndex tileIndex = startIndex;
	for (I32 i = 0; i < maxDistance; i++)
	{
		TileIndex nextIndex = MakeTileIndex (tileIndex.row + directionRow, tileIndex.col + directionCol);
		if (!IsValidTileIndex (nextIndex))
		{
			continue;
		}
		if (GetEntityAtTile (labState, nextIndex) != 0)
		{
			continue;
		}
		tileIndex = nextIndex;
	}
	return tileIndex;
}

static B32 func AbilityRequiresTileSelection (TurnBasedAbility* ability)
{
	B32 requiresTileSelection = false;

	Assert (ability != 0);
	switch (ability->id)
	{
		case SmallPunchAbilityId:
		case BigPunchAbilityId:
		case KickAbilityId:
		case RollAbilityId:
		{
			requiresTileSelection = true;
			break;
		}
		case SpinningKickAbilityId:
		{
			requiresTileSelection = false;
			break;
		}
	}

	return requiresTileSelection;
}

static B32 func CanUseAbility (TurnBasedLabState* labState, TurnBasedEntity* entity, TurnBasedAbility* ability)
{
	Assert (entity != 0);
	Assert (ability != 0);
	Assert (!AbilityRequiresTileSelection(ability));

	B32 canUse = false;
	if (IsOnCooldown (labState, entity, ability) ||
		entity->actionPoints < ability->actionPoints ||
		IsDead (entity))
	{
		canUse = false;
	}
	else
	{
		switch (ability->id)
		{
			case SpinningKickAbilityId:
			case AvoidanceAbilityId:
			{
				canUse = true;
				break;
			}
			default:
			{
				DebugBreak ();
			}
		}
	}
	return canUse;
}

static B32 func IsInvulnerable (TurnBasedLabState* labState, TurnBasedEntity* entity)
{
	B32 isInvulnerable = false;
	for (I32 i = 0; i < labState->effectN; i++)
	{
		TurnBasedEffect* effect = &labState->effects[i];
		Assert (effect->turns >= 0);
		if (effect->entity == entity && effect->id == InvulnerableEffectId)
		{
			isInvulnerable = true;
			break;
		}
	}
	return isInvulnerable;
}

static B32 func CanDamage (TurnBasedLabState* labState, TurnBasedEntity* target)
{
	B32 canDamage = false;

	if (target == 0 || IsDead (target))
	{
		canDamage = false;
	}
	else if (IsInvulnerable (labState, target))
	{
		canDamage = false;
	}
	else
	{
		canDamage = true;
	}
	return canDamage;
}

static void func DoDamage (TurnBasedLabState* labState, TurnBasedEntity* entity, I32 damage)
{
	Assert (CanDamage(labState, entity));
	Assert (damage > 0);
	entity->healthPoints = IntMax2 (0, entity->healthPoints - damage);
}

static void func UseAbility (TurnBasedLabState* labState, TurnBasedEntity* entity, TurnBasedAbility* ability)
{
	Assert (CanUseAbility (labState, entity, ability));

	switch (ability->id)
	{
		case SpinningKickAbilityId:
		{
			TileIndex center = entity->tileIndex;
			for (I32 row = center.row - 1; row <= center.row + 1; row++)
			{
				for (I32 col = center.col - 1; col <= center.col + 1; col++)
				{
					TileIndex tileIndex = MakeTileIndex (row, col);
					if (IsValidTileIndex (tileIndex))
					{
						TurnBasedEntity* target = GetEntityAtTile (labState, tileIndex);
						if (target && entity->teamIndex != target->teamIndex &&
							CanDamage (labState, target))
						{
							DoDamage (labState, target, ability->damage);
						}
					}
				}
			}
			break;
		}
		case AvoidanceAbilityId:
		{
			AddEffect (labState, entity, InvulnerableEffectId);
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	entity->actionPoints -= ability->actionPoints;
	Assert (entity->actionPoints >= 0);
	
	if (!CanRepeatAbility (ability))
	{
		AddCooldown (labState, entity, ability);
	}
}

static B32 func CanUseAbilityOnTile (TurnBasedLabState* labState,
									 TurnBasedEntity* entity, TurnBasedAbility* ability,
									 TileIndex tileIndex)
{
	Assert (entity != 0);
	Assert (ability != 0);
	Assert (AbilityRequiresTileSelection(ability));

	B32 canUse = false;
	if (IsOnCooldown (labState, entity, ability) ||
		entity->actionPoints < ability->actionPoints ||
		IsDead (entity))
	{
		canUse = false;
	}
	else
	{
		TurnBasedEntity* targetEntity = GetEntityAtTile (labState, tileIndex);
		switch (ability->id)
		{
			case SmallPunchAbilityId:
			case BigPunchAbilityId:
			case KickAbilityId:
			{
				canUse = (targetEntity->teamIndex != entity->teamIndex &&
						  CanDamage (labState, targetEntity) &&
						  (GetTileDistance (entity->tileIndex, tileIndex) <= ability->maxDistance));
				break;
			}
			case RollAbilityId:
			{
				canUse = (GetTileDistance (entity->tileIndex, tileIndex) == 1);
				break;
			}
			default:
			{
				DebugBreak ();
			}
		}
	}
	return canUse;
}

static void func UseAbilityOnTile (TurnBasedLabState* labState,
								   TurnBasedEntity* entity, TurnBasedAbility* ability,
								   TileIndex tileIndex)
{
	Assert (CanUseAbilityOnTile (labState, entity, ability, tileIndex));
	TurnBasedEntity* targetEntity = GetEntityAtTile (labState, tileIndex);
	switch (ability->id)
	{
		case SmallPunchAbilityId:
		case BigPunchAbilityId:
		{
			DoDamage (labState, targetEntity, ability->damage);
			break;
		}
		case KickAbilityId:
		{
			DoDamage (labState, targetEntity, ability->damage);
			I32 knockDirectionRow = IntSign (tileIndex.row - entity->tileIndex.row);
			I32 knockDirectionCol = IntSign (tileIndex.col - entity->tileIndex.col);
			targetEntity->tileIndex = 
				GetLastEmptyTileAtMaxDistance (labState, tileIndex, 
											   knockDirectionRow, knockDirectionCol, 5);
			break;
		}
		case RollAbilityId:
		{
			I32 rollDirectionRow = IntSign (tileIndex.row - entity->tileIndex.row);
			I32 rollDirectionCol = IntSign (tileIndex.col - entity->tileIndex.col);
			TileIndex rollTileIndex =
				GetLastEmptyTileAtMaxDistance (labState, entity->tileIndex,
											   rollDirectionRow, rollDirectionCol, 5);
			I32 distanceRolled = GetTileDistance (entity->tileIndex, rollTileIndex);
			if (distanceRolled < 5)
			{
				TileIndex damagedTileIndex = MakeTileIndex (rollTileIndex.row + rollDirectionRow,
															rollTileIndex.col + rollDirectionCol);
				TurnBasedEntity* damagedEntity = GetEntityAtTile (labState, damagedTileIndex);
				if (damagedEntity && entity->teamIndex != damagedEntity->teamIndex &&
					CanDamage (labState, damagedEntity))
				{
					DoDamage (labState, damagedEntity, ability->damage);
				}
			}

			entity->tileIndex = rollTileIndex;
			break;
		}
	}

	entity->actionPoints -= ability->actionPoints;
	Assert (entity->actionPoints >= 0);
	
	if (!CanRepeatAbility (ability))
	{
		AddCooldown (labState, entity, ability);
	}
}

static void func RemoveExpiredEffects (TurnBasedLabState* labState)
{
	I32 remainingN = 0;
	for (I32 i = 0; i < labState->effectN; i++)
	{
		TurnBasedEffect* effect = &labState->effects[i];
		Assert (effect->turns >= 0);
		if (effect->turns > 0)
		{
			labState->effects[remainingN] = *effect;
			remainingN++;
		}
	}
	labState->effectN = remainingN;
}

static void func UpdateTeamEffectTurns (TurnBasedLabState* labState, I32 teamIndex)
{
	for (I32 i = 0; i < labState->effectN; i++)
	{
		TurnBasedEffect* effect = &labState->effects[i];
		if (effect->entity->teamIndex == teamIndex)
		{
			effect->turns--;
			Assert(effect->turns >= 0);
		}
	}
	RemoveExpiredEffects (labState);
}

static void func RemoveExpiredCooldowns (TurnBasedLabState* labState)
{
	I32 remainingN = 0;
	for (I32 i = 0; i < labState->cooldownN; i++)
	{
		TurnBasedCooldown* cooldown = &labState->cooldowns[i];
		Assert (cooldown->turns >= 0);
		if (cooldown->turns > 0)
		{
			labState->cooldowns[remainingN] = *cooldown;
			remainingN++;
		}
	}
	labState->cooldownN = remainingN;
}

static void func UpdateTeamCooldowns (TurnBasedLabState* labState, I32 teamIndex)
{
	RemoveExpiredCooldowns (labState);

	for (I32 i = 0; i < labState->cooldownN; i++)
	{
		TurnBasedCooldown* cooldown = &labState->cooldowns[i];
		if (cooldown->entity->teamIndex == teamIndex)
		{
			cooldown->turns--;
			Assert (cooldown->turns >= 0);
		}
	}
}

static I32 func GetCooldownTurns (TurnBasedLabState* labState, TurnBasedEntity* entity, TurnBasedAbility* ability)
{
	TurnBasedCooldown* cooldown = 0;
	for (I32 i = 0; i < labState->cooldownN; i++)
	{
		TurnBasedCooldown* testCooldown = &labState->cooldowns[i];
		if (testCooldown->entity == entity && testCooldown->ability == ability)
		{
			cooldown = testCooldown;
			break;
		}
	}

	Assert (cooldown != 0);
	I32 turns = cooldown->turns;
	return turns;
}

static B32 func CanSelectAbility (TurnBasedLabState* labState, TurnBasedAbility* ability)
{
	Assert (AbilityRequiresTileSelection (ability));
	Assert (ability != 0);
	B32 canSelect = false;
	TurnBasedEntity* entity = labState->selectedEntity;
	if (entity)
	{
		B32 hasEnoughActionPoints = (entity->actionPoints >= ability->actionPoints);
		B32 isOnCooldown = IsOnCooldown (labState, entity, ability);
		canSelect = (hasEnoughActionPoints && !isOnCooldown);
	}
	return canSelect;
}

static void func ToggleAbility (TurnBasedLabState* labState, I32 abilityId)
{
	Assert (IsIntBetween (abilityId, 0, TurnBasedAbilityN - 1));
	TurnBasedEntity* entity = labState->selectedEntity;
	TurnBasedAbility* ability = &gTurnBasedAbilities[abilityId];
	if (entity != 0)
	{
		if (AbilityRequiresTileSelection (ability))
		{
			if (labState->selectedAbility == ability)
			{
				labState->selectedAbility = 0;
			}
			else if (CanSelectAbility (labState, ability))
			{
				labState->selectedAbility = ability;
			}
		}
		else
		{
			if (CanUseAbility (labState, entity, ability))
			{
				UseAbility(labState, entity, ability);
			}
		}
	}
}

static void func SelectEntity (TurnBasedLabState* labState, TurnBasedEntity* entity)
{
	labState->selectedEntity = entity;
	labState->selectedAbility = 0;
}

static LRESULT CALLBACK func TurnBasedLabCallback (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	
	TurnBasedLabState* labState = &gTurnBasedLabState;
	switch (message)
	{
		case WM_SIZE:
		{
			RECT clientRect = {};
			GetClientRect (window, &clientRect);
			I32 width  = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			TurnBasedLabResize (labState, width, height);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint (window, &paint);

			RECT clientRect = {};
			GetClientRect (window, &clientRect);

			TurnBasedLabBlit (&labState->canvas, context, clientRect);

			EndPaint (window, &paint);
			break;
		}
		case WM_SETCURSOR:
		{
			HCURSOR cursor = LoadCursor (0, IDC_ARROW);
			SetCursor (cursor);
			break;
		}
		case WM_KEYUP:
		{
			WPARAM keyCode = wparam;
			switch (keyCode)
			{
				case '1':
				{
					ToggleAbility (labState, SmallPunchAbilityId);
					break;
				}
				case '2':
				{
					ToggleAbility (labState, BigPunchAbilityId);
					break;
				}
				case '3':
				{
					ToggleAbility (labState, KickAbilityId);
					break;
				}
				case '4':
				{
					ToggleAbility (labState, SpinningKickAbilityId);
					break;
				}
				case '5':
				{
					ToggleAbility (labState, AvoidanceAbilityId);
					break;
				}
				case '6':
				{
					ToggleAbility (labState, RollAbilityId);
					break;
				}
				case 'E':
				{
					EndTurn (labState);
					SelectEntity (labState, 0);
					UpdateTeamCooldowns (labState, labState->activeTeamIndex);
					UpdateTeamEffectTurns (labState, labState->activeTeamIndex);
					break;
				}
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			V2 mousePosition = GetMousePosition (&labState->camera, window);
			TurnBasedEntity* entity = labState->selectedEntity;
			TurnBasedAbility* ability = labState->selectedAbility;
			TurnBasedAbility* hoverAbility = labState->hoverAbility;
			TileIndex tileIndex = GetContainingTileIndex (mousePosition);
			if (hoverAbility)
			{
				ToggleAbility (labState, hoverAbility->id);
			}
			else if (entity)
			{
				if (ability)
				{
					if (CanUseAbilityOnTile (labState, entity, ability, tileIndex))
					{
						UseAbilityOnTile (labState, entity, ability, tileIndex);
						if (!CanUseAbilityOnTile (labState, entity, ability, tileIndex))
						{
							labState->selectedAbility = 0;
						}
					}
					else
					{
						labState->selectedAbility = 0;
					}
				}
				else if (EntityCanMoveTo (labState, entity, tileIndex))
				{
					MoveEntityTo(labState, entity, tileIndex);
				}
				else
				{
					labState->selectedEntity = 0;
				}
			}
			else
			{
				TurnBasedEntity* hoverEntity = GetEntityAtTile (labState, tileIndex);
				if (hoverEntity && hoverEntity->teamIndex == labState->activeTeamIndex)
				{
					labState->selectedEntity = hoverEntity;
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
			result = DefWindowProc (window, message, wparam, lparam);
			break;
		}
	}
	return result;
}

static I8* func GetStatusText (TurnBasedLabState* labState)
{
	I8* text = 0;

	if (labState->selectedEntity == 0)
	{
		TurnBasedEntity* hoverEntity = GetEntityAtTile (labState, labState->hoverTileIndex);
		if (hoverEntity)
		{
			if (hoverEntity->teamIndex == labState->activeTeamIndex)
			{
				if (IsDead (hoverEntity))
				{
					text = "Entity is dead.";
				}
				else
				{
					text = "Click to select entity.";
				}
			}
			else
			{
				text = "It's not this entity's turn.";
			}
		}
		else
		{
			text = "Click on an entity to select it or press E to end turn.";
		}
	}
	else
	{
		text = "An entity is selected. Not yet implemented.";
	}

	Assert (text != 0);
	return text;
}

static void func DrawStatusBar (TurnBasedLabState* labState)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	I32 height = 10 + TextHeightInPixels;

	I32 left = 0;
	I32 right = bitmap->width - 1;
	I32 bottom = bitmap->height - 1;
	I32 top = bottom - height;
	V4 backgroundColor = MakeColor (0.1f, 0.1f, 0.1f);
	DrawBitmapRect (bitmap, left, right, top, bottom, backgroundColor);

	GlyphData* glyphData = labState->canvas.glyphData;
	Assert (glyphData != 0);

	V4 textColor = MakeColor (1.0f, 1.0f, 1.0f);
	I8* text = GetStatusText (labState);
	Assert (text != 0);
	DrawBitmapTextLineCentered (bitmap, text, glyphData, left, right, top, bottom, textColor);
}

static void func DrawHealthBar (TurnBasedLabState* labState, TurnBasedEntity* entity)
{
	Canvas* canvas = &labState->canvas;
	Assert (entity->maxHealthPoints > 0);
	V2 tileCenter = GetTileCenter (entity->tileIndex);

	F32 left   = tileCenter.x - TileSide * 0.5f;
	F32 right  = tileCenter.x + TileSide * 0.5f;
	F32 top    = tileCenter.y - TileSide * 0.5f;
	F32 bottom = top + TileSide * 0.2f;

	V4 backgroundColor = MakeColor (0.0f, 0.3f, 0.0f);
	V4 filledColor = MakeColor (0.0f, 1.0f, 0.0f);
	if (IsDead(entity))
	{
		backgroundColor = MakeColor (0.3f, 0.3f, 0.3f);
	}
	else if (IsInvulnerable(labState, entity))
	{
		backgroundColor = MakeColor (0.5f, 0.5f, 0.5f);
		filledColor = MakeColor (0.8f, 0.8f, 0.8f);
	}

	DrawRectLRTB (canvas, left, right, top, bottom, backgroundColor);

	F32 healthRatio = F32 (entity->healthPoints) / F32 (entity->maxHealthPoints);
	F32 filledX = Lerp(left, healthRatio, right);
	DrawRectLRTB (canvas, left, filledX, top, bottom, filledColor);
}

static String func GetEffectTooltipText (I32 effectId, I8* buffer, I32 bufferSize)
{
	String text = StartString (buffer, bufferSize);
	switch (effectId)
	{
		case InvulnerableEffectId:
		{
			AddLine (text, "Invulnerable");
			AddLine (text, "Cannot be damaged.");
			break;
		}
		default:
		{
			DebugBreak ();
		}
	}
	return text;
}

static void func DrawEffectsBar (TurnBasedLabState* labState, V2 mousePosition)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	GlyphData* glyphData = labState->canvas.glyphData;
	Assert (glyphData != 0);

	Camera* camera = &labState->camera;
	I32 mouseX = UnitXtoPixel (camera, mousePosition.x);
	I32 mouseY = UnitYtoPixel (camera, mousePosition.y);

	TurnBasedEntity* entity = labState->selectedEntity;
	I32 effectN = 0;
	for (I32 i = 0; i < labState->effectN; i++)
	{
		TurnBasedEffect* effect = &labState->effects[i];
		if (effect->entity == entity)
		{
			effectN++;
		}
	}

	if (effectN > 0)
	{
		I32 boxSide = 40;
		I32 padding = 5;
	
		I32 barWidth = padding + effectN * boxSide + (effectN - 1) * padding + padding;
		I32 barHeight = padding + boxSide + padding;

		I32 top = 0;
		I32 bottom = top + barHeight;
		I32 right = (bitmap->width - 1);
		I32 left = right - barWidth;

		V4 backgroundColor = MakeColor (0.0f, 0.0f, 0.0f);
		V4 outlineColor = MakeColor (1.0f, 1.0f, 1.0f);
		V4 textColor = MakeColor (1.0f, 1.0f, 1.0f);

		I32 boxLeft   = left + padding;
		I32 boxTop    = top + padding;
		I32 boxBottom = boxTop + boxSide;
		for (I32 i = 0; i < labState->effectN; i++)
		{
			TurnBasedEffect* effect = &labState->effects[i];
			if (effect->entity == entity)
			{
				I32 boxRight = boxLeft + boxSide;
				DrawBitmapRect (bitmap, boxLeft, boxRight, boxTop, boxBottom, backgroundColor);
				DrawBitmapRectOutline (bitmap, boxLeft, boxRight, boxTop, boxBottom, outlineColor);

				I8 textBuffer[8] = {};
				OneLineString (textBuffer, 8, effect->turns);
				DrawBitmapTextLineCentered (bitmap, textBuffer, glyphData,
											boxLeft, boxRight, boxTop, boxBottom, textColor);

				if (IsIntBetween (mouseX, boxLeft, boxRight) && IsIntBetween (mouseY, boxTop, boxBottom))
				{
					I32 tooltipTop = boxBottom + padding;
					I32 tooltipLeft = (boxLeft + boxRight) / 2 - TooltipWidth / 2;
					tooltipLeft = IntMin2 (tooltipLeft, (bitmap->width - 1) - padding - TooltipWidth);

					I8 tooltipBuffer[256] = {};
					String tooltipText = GetEffectTooltipText (effect->id, tooltipBuffer, 256);

					DrawBitmapStringTooltip (bitmap, tooltipText, glyphData, tooltipTop, tooltipLeft);
				}

				boxLeft = boxRight + padding;
			}
		}
	}
}

static String func GetAbilityTooltipText (I32 abilityId, I8* buffer, I32 bufferSize)
{
	String text = StartString (buffer, bufferSize);
	switch (abilityId)
	{
		case SmallPunchAbilityId:
		{
			AddLine (text, "Small Punch");
			AddLine (text, "Uses 1 Action Point");
			AddLine (text, "Punch an adjacent enemy lightly,");
			AddLine (text, "dealing 1 damage.");
			AddLine (text, "Can be used multiple times per turn.");
			break;
		}
		case BigPunchAbilityId:
		{
			AddLine (text, "Big Punch");
			AddLine (text, "Uses 3 Action Points");
			AddLine (text, "1 Turn cooldown");
			AddLine (text, "Forcefully punch an adjacent enemy,");
			AddLine (text, "dealing 5 damage.");
			break;
		}
		case KickAbilityId:
		{
			AddLine (text, "Kick");
			AddLine (text, "Uses 3 Action Points");
			AddLine (text, "3 Turns cooldown");
			AddLine (text, "Kick an adjacent enemy, dealing 2 damage");
			AddLine (text, "and knocking them away by up to 5 tiles.");
			break;
		}
		case SpinningKickAbilityId:
		{
			AddLine (text, "Spinning kick");
			AddLine (text, "Uses 2 Action Points");
			AddLine (text, "Deal 1 damage to all adjacent enemies.");
			break;
		}
		case AvoidanceAbilityId:
		{
			AddLine (text, "Avoidance");
			AddLine (text, "Uses 5 Action Points");
			AddLine (text, "5 Turns cooldown");
			AddLine (text, "Become invulnerable until your next turn.");
			break;
		}
		case RollAbilityId:
		{
			AddLine (text, "Roll");
			AddLine (text, "Uses 3 Action Points");
			AddLine (text, "3 Turns cooldown");
			AddLine (text, "Roll up to 5 tiles in a direction.");
			AddLine (text, "Deal 1 damage to an enemy on hit.");
			break;
		}
		default:
		{
			DebugBreak ();
		}
	}
	return text;
}

static void func DrawAbilityBar (TurnBasedLabState* labState, V2 mousePosition)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	GlyphData* glyphData = labState->canvas.glyphData;
	Assert (glyphData != 0);

	Camera* camera = &labState->camera;
	I32 mouseX = UnitXtoPixel (camera, mousePosition.x);
	I32 mouseY = UnitYtoPixel (camera, mousePosition.y);

	labState->hoverAbility = 0;
	TurnBasedEntity* entity = labState->selectedEntity;
	if (entity)
	{
		I32 boxSide = 40;
		I32 padding = 5;

		I32 abilityN = TurnBasedAbilityN;
		I32 centerX = (bitmap->width - 1) / 2;
		I32 width = boxSide * abilityN + padding * (abilityN - 1);

		I32 left = centerX - width / 2;
		I32 right = centerX + width / 2;
		I32 bottom = (bitmap->height - 1) - 30;
		I32 top = bottom - boxSide;

		I32 boxLeft = left;
		for (I32 i = 0; i < abilityN; i++)
		{
			TurnBasedAbility* ability = &gTurnBasedAbilities[i];

			I32 boxRight  = boxLeft + boxSide;
			I32 boxTop    = top;
			I32 boxBottom = bottom;

			V4 boxOutlineColor = MakeColor (1.0f, 1.0f, 1.0f);

			I32 infoBoxSide = TextHeightInPixels + 4;
			I32 infoBottom = boxTop;
			I32 infoTop    = infoBottom - infoBoxSide;
			I32 infoLeft   = boxLeft;
			I32 infoRight  = boxRight;

			I8 infoText[8] = {};
			V4 infoTextColor = {};

			if (IsOnCooldown (labState, entity, ability))
			{
				V4 infoBackgroundColor = MakeColor (0.0f, 0.0f, 0.6f);
				V4 infoOutlineColor = MakeColor (0.0f, 0.0f, 1.0f);
				DrawBitmapRect (bitmap, infoLeft, infoRight, infoTop, infoBottom, infoBackgroundColor);
				DrawBitmapRectOutline (bitmap, infoLeft, infoRight, infoTop, infoBottom, infoOutlineColor);

				I32 cooldownTurns = GetCooldownTurns (labState, entity, ability);
				OneLineString (infoText, 8, "CD+" + (cooldownTurns + 1));
				infoTextColor = MakeColor (0.0f, 0.8f, 0.8f);

				boxOutlineColor = infoOutlineColor;
			}
			else if (entity->actionPoints < ability->actionPoints)
			{
				OneLineString (infoText, 8, ability->actionPoints + " AP");
				infoTextColor = MakeColor (1.0f, 0.0f, 0.0f);
				boxOutlineColor = MakeColor (1.0f, 0.0f, 0.0f);
			}
			else
			{
				OneLineString (infoText, 8, ability->actionPoints + " AP");
				infoTextColor = MakeColor (1.0f, 1.0f, 1.0f);
			}

			DrawBitmapTextLineCentered (bitmap, infoText, glyphData, 
										infoLeft, infoRight, infoTop, infoBottom,
										infoTextColor);

			V4 boxBackgroundColor = MakeColor (0.0f, 0.0f, 0.0f);
			V4 textColor = MakeColor (1.0f, 1.0f, 1.0f);

			if (labState->selectedAbility == ability)
			{
				boxOutlineColor = MakeColor (1.0f, 0.5f, 0.0f);
				textColor = MakeColor (1.0f, 0.5f, 0.0f);
			}

			DrawBitmapRect (bitmap, boxLeft, boxRight, boxTop, boxBottom, boxBackgroundColor);

			char name[8];
			OneLineString (name, 8, (i + 1));
			DrawBitmapTextLineCentered (bitmap, name, glyphData, 
										boxLeft, boxRight, boxTop, boxBottom, 
										textColor);

			DrawBitmapRectOutline (bitmap, boxLeft, boxRight, boxTop, boxBottom, boxOutlineColor);

			if (IsIntBetween (mouseX, boxLeft, boxRight) && IsIntBetween (mouseY, boxTop, boxBottom))
			{
				labState->hoverAbility = ability;

				I32 tooltipLeft = (boxLeft + boxRight) / 2 - TooltipWidth / 2;
				I32 tooltipBottom = infoTop - 5;

				static I8 tooltipBuffer[256] = {};
				String tooltipText = GetAbilityTooltipText (ability->id, tooltipBuffer, 256);

				DrawBitmapStringTooltipBottom (bitmap, tooltipText, glyphData, tooltipBottom, tooltipLeft);
			}

			boxLeft = boxRight + padding;
		}
	}
}

static void func TurnBasedLabUpdate (TurnBasedLabState* labState, V2 mousePosition, F32 seconds)
{
	Canvas* canvas = &labState->canvas;
	V4 backgroundColor = MakeColor (0.0f, 0.0f, 0.0f);
	ClearScreen (canvas, backgroundColor);

	V4 tileGridColor = MakeColor (0.2f, 0.2f, 0.2f);

	V4 tileColor = MakeColor (0.5f, 0.5f, 0.5f);
	V4 hoverTileColor = MakeColor (0.6f, 0.6f, 0.6f);

	V4 moveTileColor = MakeColor (0.8f, 0.8f, 0.8f);
	V4 hoverMoveTileColor = MakeColor (0.9f, 0.9f, 0.9f);

	V4 attackTileColor = MakeColor (0.8f, 0.5f, 0.5f);
	V4 hoverAttackTileColor = MakeColor (0.9f, 0.6f, 0.6f);

	labState->hoverTileIndex = GetContainingTileIndex (mousePosition);

	TurnBasedEntity* selectedEntity = labState->selectedEntity;

	for (I32 row = 0; row < TileRowN; row++)
	{
		for (I32 col = 0; col < TileColN; col++)
		{
			TileIndex tileIndex = MakeTileIndex (row, col);
			TurnBasedEntity* entity = GetEntityAtTile (labState, tileIndex);
			TurnBasedAbility* ability = labState->selectedAbility;

			B32 isEntity = (entity != 0);
			B32 isHover = (tileIndex == labState->hoverTileIndex);
			B32 canMove = (ability == 0) && EntityCanMoveTo (labState, selectedEntity, tileIndex);
			B32 canAttack = ((ability != 0) &&
							 GetTileDistance (selectedEntity->tileIndex, tileIndex) <= ability->maxDistance);

			V4 color = tileColor;
			if (isEntity)
			{
				Assert  (entity != 0);
				I32 teamIndex = entity->teamIndex;
				Assert (IsValidTeamIndex (teamIndex));

				color = (isHover) ? HoverTeamColors[teamIndex] : TeamColors[teamIndex];
			}
			else if (canMove)
			{
				color = (isHover) ? hoverMoveTileColor : moveTileColor;
			}
			else if (canAttack)
			{
				color = (isHover) ? hoverAttackTileColor : attackTileColor;
			}
			else
			{
				color = (isHover) ? hoverTileColor : tileColor;
			}

			V2 tileCenter = GetTileCenter (tileIndex);
			Rect tileRect = MakeSquareRect (tileCenter, TileSide);
			DrawRect (canvas, tileRect, color);

			if (entity != 0)
			{
				DrawHealthBar(labState, entity);
			}

			DrawRectOutline (canvas, tileRect, tileGridColor);

			if (entity != 0 && entity->teamIndex == labState->activeTeamIndex && !IsDead(entity))
			{
				I8 actionPointsText[8];
				String actionPointsString = StartString (actionPointsText, 8);
				AddInt (&actionPointsString, entity->actionPoints);
				V2 tileCenter = GetTileCenter (tileIndex);
				V4 textColor = MakeColor (1.0f, 1.0f, 1.0f);
				DrawTextLineXYCentered (canvas, actionPointsText, tileCenter.y, tileCenter.x, textColor);
			}
		}
	}

	Camera* camera = &labState->camera;
	camera->center = MakePoint (TileColN * TileSide * 0.5f, TileRowN * TileSide * 0.5f);

	DrawStatusBar(labState);
	DrawAbilityBar(labState, mousePosition);
	DrawEffectsBar(labState, mousePosition);
}

static void func TurnBasedLab (HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = TurnBasedLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "TurnBasedLabWindowClass";

	Verify (RegisterClass (&winClass));
	HWND window = CreateWindowEx (
		0,
		winClass.lpszClassName,
		"TurnBasedLab",
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
	Assert (window != 0);

	TurnBasedLabState* labState = &gTurnBasedLabState;

	RECT rect = {};
	GetClientRect (window, &rect);
	I32 width  = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	TurnBasedLabInit (labState, width, height);

	LARGE_INTEGER counterFrequency;
	QueryPerformanceFrequency (&counterFrequency);

	LARGE_INTEGER lastCounter;
	QueryPerformanceCounter (&lastCounter);

	MSG message = {};
	while (labState->running)
	{
		while (PeekMessage (&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage (&message);
			DispatchMessage (&message);
		}

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		I64 microSeconds = counter.QuadPart - lastCounter.QuadPart;
		F32 milliSeconds = (F32 (microSeconds) * 1000.0f) / F32 (counterFrequency.QuadPart);
		F32 seconds = 0.001f * milliSeconds;
		lastCounter = counter;

		V2 mousePosition = GetMousePosition (&labState->camera, window);
		TurnBasedLabUpdate (labState, mousePosition, seconds);

		RECT rect = {};
		GetClientRect (window, &rect);

		HDC context = GetDC (window);
		TurnBasedLabBlit (&labState->canvas, context, rect);
		ReleaseDC (window, context);
	}
}

// TODO: Better status texts!
	// TODO: When an entity is selected
// TODO: Disable selection of dead entities!
// TODO: Better ability outcome visualization!
// TODO: Remove ability structure, use only ids?