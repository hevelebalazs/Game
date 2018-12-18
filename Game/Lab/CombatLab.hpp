#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Geometry.hpp"

#define CombatLabEnemyN 20

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
#define RedAbility1Damage 20

#define RedAbility2Radius 5.0f
#define RedAbility2TotalCastTime 0.5f
#define RedAbility2AngleRange (0.25f * PI)
#define RedAbility2Damage 50

#define BlueAbility1Radius 3.0f
#define BlueAbility1TotalCastTime 1.0f
#define BlueAbility1Damage 20

#define BlueAbility2Radius 3.0f
#define BlueAbility2TotalCastTime 1.0f
#define BlueAbility2Damage 20
#define BlueAbility2SlowTime 3.0f
#define BlueAbility2ResourceCost 50

#define BlueAbility3Radius 1.5f
#define BlueAbility3TotalCastTime 1.0f
#define BlueAbility3Damage 40
#define BlueAbility3SlowTime 6.0f
#define BlueAbility3ResourceCost 100

#define MaxLevel 5

static int gExpNeededForNextLevel[] = 
{
	0,
	100,
	150,
	250,
	500
};

static int gEnemyHPOnLevel[] =
{
	0,
	100,
	200,
	350,
	550,
	800
};

enum AbilityID
{
	NoAbilityID,
	WhiteAbility1ID,
	WhiteAbility2ID,
	WhiteAbility3ID,
	RedAbility1ID,
	RedAbility2ID,
	BlueAbility1ID,
	BlueAbility2ID,
	BlueAbility3ID
};

struct Ability
{
	I32 id;
	F32 castTimeLeft;

	V2 position;
	F32 angle;
};

enum EntityType
{
	WhiteEntity,
	RedEntity,
	BlueEntity
};

struct Entity
{
	EntityType type;
	I32 level;

	V2 position;
	V2 velocity;
	F32 radius;

	I32 health;
	I32 maxHealth;

	Ability ability;

	B32 isAttacking;
	F32 attackFromAngle;

	B32 hasSpecialResource;
	I32 maxSpecialResource;
	I32 specialResource;

	F32 slowTimeLeft;
};

struct CombatLabState
{
	Camera camera;
	Canvas canvas;
	B32 running;

	Entity player;
	Entity enemies[CombatLabEnemyN];

	B32 followMouse;
	I32 useAbilityID;

	I32 expGainedThisLevel;

	F32 secondsFraction;
};
static CombatLabState gCombatLabState;

static void CombatLabResize(CombatLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 20.0f;
}

static void CombatLabBlit(Canvas canvas, HDC context, RECT rect)
{
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	Bitmap bitmap = canvas.bitmap;
	BITMAPINFO bitmapInfo = GetBitmapInfo(&bitmap);
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
				  0, 0, width, height,
				  bitmap.memory,
				  &bitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static void GainSpecialResource(Entity* entity, I32 amount)
{
	Assert(entity->hasSpecialResource);
	Assert(amount >= 0);
	entity->specialResource = IntMin2(entity->specialResource + amount, entity->maxSpecialResource);
}

static void DoDamage(Entity* entity, I32 damage)
{
	entity->health = IntMax2(entity->health - damage, 0);
	if (entity->health == 0)
	{
		Ability* ability = &entity->ability;
		ability->castTimeLeft = 0.0f;
	}
}

static void InterruptAbility(Entity* entity)
{
	Ability* ability = &entity->ability;
	ability->id = NoAbilityID;
	ability->castTimeLeft = 0.0f;
}

static void CombatLabReset(CombatLabState* labState)
{
	InitRandom();

	labState->running = true;

	Entity* player = &labState->player;
	player->type = WhiteEntity;
	player->level = 1;

	player->position = MakePoint(0.0f, 0.0f);
	player->radius = 1.0f;

	player->maxHealth = 100;
	player->health = player->maxHealth;

	player->hasSpecialResource = true;
	player->specialResource = 0;
	player->maxSpecialResource = 100;

	player->slowTimeLeft = 0.0f;

	player->ability.id = WhiteAbility1ID;

	for (I32 i = 0; i < CombatLabEnemyN; ++i) 
	{
		Entity* enemy = &labState->enemies[i];
		if (i < CombatLabEnemyN / 2)
		{
			enemy->type = RedEntity;
			enemy->ability.id = RedAbility1ID;
		}
		else
		{
			enemy->type = BlueEntity;
			enemy->ability.id = BlueAbility1ID;
		}

		enemy->level = IntRandom(1, 3);
		enemy->position.x = player->position.x + RandomBetween(-50.0f, +50.0f);
		enemy->position.y = player->position.y + RandomBetween(-50.0f, +50.0f);
		enemy->radius = player->radius;

		enemy->maxHealth = gEnemyHPOnLevel[enemy->level];
		enemy->health = enemy->maxHealth;

		enemy->hasSpecialResource = false;
		enemy->isAttacking = false;

		if (enemy->level >= 2)
		{
			enemy->hasSpecialResource = true;
			enemy->maxSpecialResource = 100;
			enemy->specialResource = 0;
		}
	}

	labState->expGainedThisLevel = 0;
}

static void CombatLabResetAtLevel(CombatLabState* labState, I32 level)
{
	Assert(IsIntBetween(level, 1, MaxLevel));
	CombatLabReset(labState);
	labState->player.level = level;
}

static Entity* GetFirstEnemyAtPosition(CombatLabState* labState, V2 position)
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

static B32 IsEntityOnLine(Entity* entity, V2 point1, V2 point2)
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

static void DamageEnemiesOnLine(CombatLabState* labState, V2 point1, V2 point2, I32 damage)
{
	for (I32 i = 0; i < CombatLabEnemyN; ++i)
	{
		Entity* enemy = &labState->enemies[i];
		if (IsEntityOnLine(enemy, point1, point2))
		{
			DoDamage(enemy, damage);
		}
	}
}

static LRESULT CALLBACK CombatLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
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

			CombatLabBlit(labState->canvas, context, clientRect);

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
			labState->useAbilityID = WhiteAbility1ID;
			break;
		}
		case WM_LBUTTONUP:
		{
			labState->useAbilityID = NoAbilityID;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			labState->followMouse = true;
			break;
		}
		case WM_RBUTTONUP:
		{
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
					labState->useAbilityID = WhiteAbility2ID;
					break;
				}
				case 'W':
				{
					labState->useAbilityID = WhiteAbility3ID;
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
					labState->useAbilityID = NoAbilityID;
					break;
				}
				case 'W':
				{
					labState->useAbilityID = NoAbilityID;
					break;
				}
				case 'B':
				{
					Entity* player = &labState->player;
					Assert(player->hasSpecialResource);
					player->specialResource = player->maxSpecialResource;
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

static void CombatLabInit(CombatLabState* labState, I32 windowWidth, I32 windowHeight)
{
	CombatLabReset(labState);

	CombatLabResize(labState, windowWidth, windowHeight);
}

static void DrawSlice(Canvas canvas, V2 center, F32 radius, F32 minAngle, F32 maxAngle, V4 color)
{
	U32 colorCode = GetColorCode(color);

	Camera *camera = canvas.camera;
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

	Bitmap bitmap = canvas.bitmap;
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

static void DrawSliceOutline(Canvas canvas, V2 center, F32 radius, F32 minAngle, F32 maxAngle, V4 color)
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

static void DrawCircle(Canvas canvas, V2 center, F32 radius, V4 color)
{
	U32 colorCode = GetColorCode(color);

	Camera *camera = canvas.camera;

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

	Bitmap bitmap = canvas.bitmap;
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

static void DrawCircleOutline(Canvas canvas, V2 center, F32 radius, V4 color)
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

static void DrawEntity(Canvas canvas, Entity* entity)
{
	V4 color = {};
	switch (entity->type)
	{
		case WhiteEntity:
			color = MakeColor(1.0f, 1.0f, 1.0f);
			break;
		case RedEntity:
			color = MakeColor(1.0f, 0.0f, 0.0f);
			break;
		case BlueEntity:
			color = MakeColor(0.0f, 0.0f, 1.0f);
			break;
		default:
			DebugBreak ();
	}

	DrawCircle(canvas, entity->position, entity->radius, color);

	V4 healthBarBackgroundColor = {};
	if (entity->health > 0.0f)
	{
		healthBarBackgroundColor = MakeColor(0.5f, 0.0f, 0.0f);
	}
	else
	{
		healthBarBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
	}

	V4 healthBarColor = MakeColor(1.0f, 0.0f, 0.0f);

	F32 healthBarWidth = 2.0f * entity->radius;
	F32 healthBarHeight = 0.3f * healthBarWidth;

	F32 healthBarX = entity->position.x;
	F32 healthBarY = entity->position.y - entity->radius - healthBarHeight - healthBarHeight * 0.5f;

	F32 healthBarLeft   = healthBarX - 0.5f * healthBarWidth;
	F32 healthBarRight  = healthBarX + 0.5f * healthBarWidth;
	F32 healthBarTop    = healthBarY - 0.5f * healthBarHeight;
	F32 healthBarBottom = healthBarY + 0.5f * healthBarHeight;
	DrawRect(canvas, healthBarLeft, healthBarRight, healthBarTop, healthBarBottom, healthBarBackgroundColor);

	F32 healthRatio = F32(entity->health) / F32(entity->maxHealth);
	Assert(IsBetween(healthRatio, 0.0f, 1.0f));
	F32 healthBarFilledX = healthBarLeft + healthRatio * (healthBarRight - healthBarLeft);
	DrawRect(canvas, healthBarLeft, healthBarFilledX, healthBarTop, healthBarBottom, healthBarColor);

	Assert(IsIntBetween(entity->level, 1, MaxLevel));
	V4 levelBoxColor = MakeColor(1.0f, 1.0f, 1.0f);

	F32 spaceBetweenLevelBoxes = 0.05f;
	F32 levelBoxBarWidth = healthBarWidth;
	F32 levelBoxSize = (levelBoxBarWidth - (MaxLevel - 1) * spaceBetweenLevelBoxes) / MaxLevel;
	F32 levelBoxBottom = healthBarTop - 0.05f;
	F32 levelBoxTop = levelBoxBottom - levelBoxSize;
	for (I32 i = 1; i <= entity->level; i++)
	{
		F32 spaceBoxLeft = healthBarLeft + (i - 1) * (levelBoxSize + spaceBetweenLevelBoxes);
		F32 spaceBoxRight = spaceBoxLeft + levelBoxSize;
		DrawRect(canvas, spaceBoxLeft, spaceBoxRight, levelBoxTop, levelBoxBottom, levelBoxColor);
	}

	if (entity->hasSpecialResource)
	{
		V4 filledColor = MakeColor(1.0f, 1.0f, 0.0f);
		V4 emptyColor = MakeColor(0.3f, 0.3f, 0.3f);
		F32 barTop = healthBarBottom + 0.05f;
		F32 barBottom = barTop - 0.2f;
		F32 barLeft = healthBarLeft;
		F32 barRight = healthBarRight;
		DrawRect(canvas, barLeft, barRight, barTop, barBottom, emptyColor);
		
		Assert(entity->maxSpecialResource > 0);
		Assert(IsIntBetween(entity->specialResource, 0, entity->maxSpecialResource));
		F32 filledRatio = F32(entity->specialResource) / F32(entity->maxSpecialResource);
		F32 barFilledX = barLeft + (barRight - barLeft) * filledRatio;
		DrawRect(canvas, barLeft, barFilledX, barTop, barBottom, filledColor);
	}
}

static B32 IsEntityInSlice(Entity* entity, V2 center, F32 radius, F32 minAngle, F32 maxAngle)
{
	F32 distance = Distance(entity->position, center);
	F32 angle = LineAngle(center, entity->position);
	B32 result = (distance < radius + entity->radius && IsAngleBetween(minAngle, angle, maxAngle));
	return result;
}

static void GainExp(CombatLabState* labState, I32 exp)
{
	I32 level = labState->player.level;
	if (level < MaxLevel)
	{
		labState->expGainedThisLevel += exp;
		if (labState->expGainedThisLevel >= gExpNeededForNextLevel[level])
		{
			labState->player.level++;
			labState->expGainedThisLevel -= gExpNeededForNextLevel[level];
		}
	}
}

static B32 IsEntityInCircle(Entity* entity, V2 center, F32 radius)
{
	B32 result = false;
	F32 distanceFromCenter = Distance(entity->position, center);
	if (distanceFromCenter < radius + entity->radius)
	{
		result = true;
	}
	return result;
}

static void DrawWhiteAbility1(Canvas canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability)
	Assert(ability->id == WhiteAbility1ID);
	V4 color = MakeColor(0.8f, 0.8f, 0.8f);
	F32 radius = entity->radius + WhiteAbility1Radius;

	if (ability->castTimeLeft > 0.0f)
	{
		Assert(IsBetween(ability->castTimeLeft, 0.0f, WhiteAbility1TotalCastTime));
		F32 durationRatio = 1.0f - ability->castTimeLeft / WhiteAbility1TotalCastTime;
		F32 fillRadius = entity->radius + (durationRatio) * (radius - entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - WhiteAbility1AngleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + WhiteAbility1AngleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void DrawWhiteAbility3(Canvas canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == WhiteAbility3ID);
	V4 color = MakeColor(0.8f, 0.8f, 0.8f);
	F32 radius = entity->radius + WhiteAbility3Radius;

	if (ability->castTimeLeft > 0.0f)
	{
		Assert(IsBetween(ability->castTimeLeft, 0.0f, WhiteAbility3TotalCastTime));
		F32 durationRatio = 1.0f - ability->castTimeLeft / WhiteAbility3TotalCastTime;
		F32 fillRadius = entity->radius + (durationRatio) * (radius - entity->radius);
		DrawCircleOutline(canvas, entity->position, radius, color);
		DrawCircle(canvas, entity->position, fillRadius, color);
	}
}

static void DrawRedAbility1(Canvas canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == RedAbility1ID);
	V4 color = MakeColor(0.8f, 0.0f, 0.0f);
	F32 radius = entity->radius + RedAbility1Radius;

	if (ability->castTimeLeft > 0.0f)
	{
		Assert(IsBetween(ability->castTimeLeft, 0.0f, RedAbility1TotalCastTime));
		F32 durationRatio = 1.0f - ability->castTimeLeft / RedAbility1TotalCastTime;
		F32 fillRadius = entity->radius + (durationRatio) * (radius - entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - RedAbility1AngleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + RedAbility1AngleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void DrawRedAbility3(Canvas canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == RedAbility2ID);

	Assert(entity->level >= 2);
	V4 color = MakeColor(0.8f, 0.0f, 0.0f);
	F32 radius = entity->radius + RedAbility2Radius;

	if (ability->castTimeLeft > 0.0f)
	{
		Assert(IsBetween(ability->castTimeLeft, 0.0f, RedAbility2TotalCastTime));
		F32 durationRatio = 1.0f - ability->castTimeLeft / RedAbility2TotalCastTime;
		F32 fillRadius = entity->radius + (durationRatio) * (radius - entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - RedAbility2AngleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + RedAbility2AngleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void DrawBlueAbility1(Canvas canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability)
	Assert(ability->id == BlueAbility1ID);

	V4 color = MakeColor(0.0f, 0.0f, 0.8f);
	if (ability->castTimeLeft > 0.0f)
	{
		Assert(IsBetween(ability->castTimeLeft, 0.0f, BlueAbility1TotalCastTime));
		F32 durationRatio = 1.0f - ability->castTimeLeft / BlueAbility1TotalCastTime;
		F32 fillRadius = durationRatio * BlueAbility1Radius;

		DrawCircleOutline(canvas, ability->position, BlueAbility1Radius, color);
		DrawCircle(canvas, ability->position, fillRadius, color);
	}
}

static void DrawBlueAbility2(Canvas canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == BlueAbility2ID);

	V4 color = MakeColor(0.0f, 0.0f, 0.8f);
	if (ability->castTimeLeft > 0.0f)
	{
		Assert(IsBetween(ability->castTimeLeft, 0.0f, BlueAbility2TotalCastTime));
		F32 durationRatio = 1.0f - ability->castTimeLeft / BlueAbility2TotalCastTime;
		F32 fillRadius = entity->radius + durationRatio * BlueAbility2Radius;
		
		DrawCircleOutline(canvas, entity->position, entity->radius + BlueAbility2Radius, color);
		DrawCircle(canvas, entity->position, fillRadius, color);
	}
}

static void DrawBlueAbility3(Canvas canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	Assert(ability->id == BlueAbility3ID);

	V4 color = MakeColor(0.0f, 0.0f, 0.8f);
	if (ability->castTimeLeft > 0.0f)
	{
		Assert(IsBetween(ability->castTimeLeft, 0.0f, BlueAbility3TotalCastTime));
		F32 durationRatio = 1.0f - ability->castTimeLeft / BlueAbility3TotalCastTime;
		F32 fillRadius = durationRatio * BlueAbility3Radius;
		
		DrawCircleOutline(canvas, ability->position, BlueAbility3Radius, color);
		DrawCircle(canvas, ability->position, fillRadius, color);
	}
}

static void DrawAbility(Canvas canvas, Entity* entity, Ability* ability)
{
	Assert(ability == &entity->ability);
	if (ability->id == WhiteAbility1ID)
	{
		DrawWhiteAbility1(canvas, entity, ability);
	}
	else if (ability->id == WhiteAbility2ID)
	{
	}
	else if (ability->id == WhiteAbility3ID)
	{
		DrawWhiteAbility3(canvas, entity, ability);
	}
	else if (ability->id == BlueAbility1ID)
	{
		DrawBlueAbility1(canvas, entity, ability);
	}
	else if (ability->id == BlueAbility2ID)
	{
		DrawBlueAbility2(canvas, entity, ability);
	}
	else if (ability->id == BlueAbility3ID)
	{
		DrawBlueAbility3(canvas, entity, ability);
	}
	else if (ability->id == RedAbility1ID)
	{
		DrawRedAbility1(canvas, entity, ability);
	}
	else if (ability->id == RedAbility2ID)
	{
		DrawRedAbility3(canvas, entity, ability);
	}
	else if (ability->id == NoAbilityID)
	{
	}
	else
	{
		DebugBreak();
	}
}

static void DrawExpBar(Canvas canvas, CombatLabState* labState)
{
	I32 level = labState->player.level;
	if (level < MaxLevel)
	{
		Bitmap* bitmap = &canvas.bitmap;
		I32 left = 0;
		I32 right = bitmap->width - 1;
		I32 bottom = bitmap->height - 1;
		I32 top = bottom - 20;
		V4 unfilledColor = MakeColor(0.3f, 0.3f, 0.3f);
		V4 filledColor = MakeColor(0.6f, 0.6f, 0.6f);
		DrawBitmapRect(bitmap, left, right, top, bottom, unfilledColor);

		I32 expGained = labState->expGainedThisLevel;
		Assert(IsIntBetween(level, 1, MaxLevel - 1));
		I32 expNeeded = gExpNeededForNextLevel[level];
		I32 rightFilled = left + Floor(F32(right - left) * F32(expGained) / F32(expNeeded));
		DrawBitmapRect(bitmap, left, rightFilled, top, bottom, filledColor);
	}
}

static F32 GetEntityDistance(Entity* entity1, Entity* entity2)
{
	F32 distance = Distance(entity1->position, entity2->position) - entity1->radius - entity2->radius;
	return distance;
}

static void CombatLabUpdate(CombatLabState* labState, V2 mousePosition, F32 seconds)
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

	player->velocity = MakeVector(0.0f, 0.0f);
	if (labState->followMouse)
	{
		F32 playerMoveSpeed = 10.0f;
		if (player->slowTimeLeft > 0.0f)
		{
			playerMoveSpeed *= 0.5f;
			player->slowTimeLeft = Max2(0.0f, player->slowTimeLeft - seconds);
		}
		Assert(player->slowTimeLeft >= 0.0f);

		V2 targetPosition = mousePosition;
		V2 moveDirection = PointDirection(player->position, targetPosition);
		player->velocity = playerMoveSpeed * moveDirection;
	}

	if (player->health > 0.0f && playerAbility->castTimeLeft == 0.0f)
	{
		player->position = player->position + seconds * player->velocity;
	}

	Canvas canvas = labState->canvas;
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
		if (playerAbility->castTimeLeft > 0.0f)
		{
			playerAbility->castTimeLeft = Max2(0.0f, playerAbility->castTimeLeft - seconds);
			if (playerAbility->id == WhiteAbility1ID)
			{
				if (playerAbility->castTimeLeft == 0.0f)
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
							DoDamage(enemy, 30);
							B32 enemyIsDead = (enemy->health == 0);

							if (enemyWasAlive)
							{
								hitAnyEnemy = true;
							}

							if (enemyWasAlive && enemyIsDead)
							{
								GainExp(labState, 20);
							}
						}
					}

					if (hitAnyEnemy)
					{
						GainSpecialResource(player, 20);
					}
				}
			}
			else if (playerAbility->id == WhiteAbility2ID)
			{
				V2 moveDirection = RotationVector(playerAbility->angle);
				player->position = player->position + seconds * WhiteAbility2MoveSpeed * moveDirection;
				if (playerAbility->castTimeLeft == 0.0f)
				{
					V2 oldPosition = player->position - WhiteAbility2TotalCastTime * WhiteAbility2MoveSpeed * moveDirection;
					DamageEnemiesOnLine(labState, oldPosition, player->position, WhiteAbility2Damage);
				}
			}
			else if (playerAbility->id == WhiteAbility3ID)
			{
				if (playerAbility->castTimeLeft == 0.0f)
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
							DoDamage(enemy, WhiteAbility3Damage);
						}
					}
				}
			}
			else
			{
				DebugBreak();
			}
		}
	
		Assert(playerAbility->castTimeLeft >= 0.0f);
		if (playerAbility->castTimeLeft == 0.0f)
		{
			if (labState->useAbilityID == WhiteAbility1ID)
			{
				playerAbility->id = WhiteAbility1ID;
				playerAbility->angle = LineAngle(labState->player.position, mousePosition);
				playerAbility->castTimeLeft = WhiteAbility1TotalCastTime;
			}
			else if (labState->useAbilityID == WhiteAbility2ID)
			{
				if (player->level >= 2)
				{
					Assert(player->hasSpecialResource);
					if (player->specialResource >= WhiteAbility2ResourceCost)
					{
						player->specialResource -= WhiteAbility2ResourceCost;
						playerAbility->id = WhiteAbility2ID;
						playerAbility->angle = LineAngle(labState->player.position, mousePosition);
						playerAbility->castTimeLeft = WhiteAbility2TotalCastTime;
					}
				}
			}
			else if (labState->useAbilityID == WhiteAbility3ID)
			{
				if (player->level >= 3)
				{
					Assert(player->hasSpecialResource);
					if (player->specialResource >= WhiteAbility3ResourceCost)
					{
						player->specialResource -= WhiteAbility3ResourceCost;
						playerAbility->id = WhiteAbility3ID;
						playerAbility->castTimeLeft = WhiteAbility3TotalCastTime;
					}
				}
			}
			else
			{
				Assert(labState->useAbilityID == NoAbilityID);
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
					ability->castTimeLeft = 0.0f;
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

			if (enemy->type == RedEntity && enemy->level >= 2)
			{
				GainSpecialResource(enemy, 10 * secondsPassed);
			}

			if (enemy->type == BlueEntity)
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
			if (enemy->type == RedEntity)
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
			else if (enemy->type == BlueEntity)
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
			if (ability->castTimeLeft > 0.0f)
			{
				ability->castTimeLeft = Max2(0.0f, ability->castTimeLeft - seconds);

				if (ability->id == RedAbility2ID)
				{
					Assert(enemy->level >= 2);
					ability->angle = LineAngle(enemy->position, player->position);
				}
				else if (ability->id == BlueAbility3ID)
				{
					Assert(enemy->level >= 3);
					ability->position = player->position;
				}

				if (ability->castTimeLeft == 0.0f)
				{
					if (ability->id == RedAbility1ID)
					{
						F32 minAngle = NormalizeAngle(ability->angle - RedAbility1AngleRange * 0.5f);
						F32 maxAngle = NormalizeAngle(ability->angle + RedAbility1AngleRange * 0.5f);
						if (IsEntityInSlice(player, enemy->position, RedAbility1Radius, minAngle, maxAngle))
						{
							DoDamage(player, RedAbility1Damage);
						}
					}
					else if (ability->id == RedAbility2ID)
					{
						F32 minAngle = NormalizeAngle(ability->angle - RedAbility2AngleRange * 0.5f);
						F32 maxAngle = NormalizeAngle(ability->angle + RedAbility2AngleRange * 0.5f);
						if (IsEntityInSlice(player, enemy->position, RedAbility2Radius, minAngle, maxAngle))
						{
							DoDamage(player, RedAbility2Damage);
						}
					}
					else if (ability->id == BlueAbility1ID)
					{
						Assert(enemy->type == BlueEntity);
						if (IsEntityInCircle(player, ability->position, BlueAbility1Radius))
						{
							DoDamage(player, BlueAbility1Damage);
						}
					}
					else if (ability->id == BlueAbility2ID)
					{
						Assert(enemy->type == BlueEntity);
						F32 distanceFromPlayer = GetEntityDistance(enemy, player);
						if (distanceFromPlayer <= BlueAbility2Radius)
						{
							DoDamage(player, BlueAbility2Damage);
							player->slowTimeLeft += BlueAbility2SlowTime;
						}
					}
					else if (ability->id == BlueAbility3ID)
					{
						Assert(enemy->type == BlueEntity);
						if (IsEntityInCircle(player, ability->position, BlueAbility3Radius))
						{
							DoDamage(player, BlueAbility3Damage);
							player->slowTimeLeft += BlueAbility3SlowTime;
						}
					}
					else
					{
						DebugBreak();
					}
				}
			}

			B32 canMove = false;
			if (ability->castTimeLeft == 0.0f)
			{
				canMove = true;
			}
			else if (ability->id == RedAbility2ID && enemy->level >= 3)
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
	
			if (ability->castTimeLeft == 0.0f)
			{
				if (enemy->type == BlueEntity && enemy->level == 2)
				{
					F32 distanceFromPlayer = GetEntityDistance(enemy, player);
					Assert(enemy->hasSpecialResource);
					if (distanceFromPlayer <= BlueAbility2Radius && enemy->specialResource >= BlueAbility2ResourceCost)
					{
						ability->id = BlueAbility2ID;
						ability->castTimeLeft = BlueAbility2TotalCastTime;
						enemy->specialResource -= BlueAbility2ResourceCost;
					}
				}
				else if (enemy->type == BlueEntity && enemy->level == 3)
				{	
					Assert(enemy->hasSpecialResource);
					if (enemy->specialResource >= BlueAbility3ResourceCost)
					{
						ability->id = BlueAbility3ID;
						ability->position = player->position;
						ability->castTimeLeft = BlueAbility3TotalCastTime;
						enemy->specialResource -= BlueAbility3ResourceCost;
					}
				}

				if (ability->castTimeLeft == 0.0f && distanceFromTarget < 0.1f)
				{
					if (enemy->type == RedEntity)
					{
						bool useRedAbility = true;
						if (enemy->level >= 2)
						{
							Assert(enemy->hasSpecialResource);
							if (enemy->specialResource >= 100)
							{
								ability->id = RedAbility2ID;
								ability->angle = LineAngle(enemy->position, player->position);
								ability->castTimeLeft = RedAbility2TotalCastTime;
								enemy->specialResource -= 100;
								useRedAbility = false;
							}
						}

						if (useRedAbility)
						{
							ability->id = RedAbility1ID;
							ability->angle = LineAngle(enemy->position, player->position);
							ability->castTimeLeft = RedAbility1TotalCastTime;
						}
					}
					else if (enemy->type == BlueEntity)
					{
						ability->id = BlueAbility1ID;
						ability->position = player->position;
						ability->castTimeLeft = BlueAbility1TotalCastTime;
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
			if (enemy->ability.castTimeLeft > 0.0f)
			{
				InterruptAbility(enemy);
			}
		}
	}

	DrawEntity(canvas, &labState->player);
	for (I32 i = 0; i < CombatLabEnemyN; i++)
	{
		Entity* enemy = &labState->enemies[i];
		DrawEntity(canvas, enemy);
	}

	DrawExpBar(canvas, labState);
}

static void CombatLab(HINSTANCE instance)
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
		CombatLabBlit(labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}