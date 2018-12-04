#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Geometry.hpp"

#define WhiteAbilityRadius 5.0f
#define WhiteAbilityTotalDuration 0.5f
#define WhiteAbilityAngleRange (0.25f * PI)
#define WhiteAbilityDamage 30.0f

#define RedAbilityRadius 5.0f
#define RedAbilityTotalDuration 1.0f
#define RedAbilityAngleRange (0.5f * PI)
#define RedAbilityDamage 30.0f

#define BlueAbilityRadius 3.0f
#define BlueAbilityTotalDuration 1.0f
#define BlueAbilityDamage 20.0f

struct Ability
{
	F32 durationLeft;

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

	V2 position;
	V2 velocity;
	F32 radius;

	F32 health;
	F32 maxHealth;

	Ability ability;

	B32 isAttacking;
	F32 attackFromAngle;
};

#define CombatLabEnemyN 10
struct CombatLabState
{
	Camera camera;
	Canvas canvas;
	B32 running;

	Entity player;
	Entity enemies[CombatLabEnemyN];

	B32 followMouse;
	B32 useAbility;
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
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
				  0, 0, width, height,
				  bitmap.memory,
				  &bitmap.info,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static void DoDamage(Entity* entity, F32 damage)
{
	entity->health = Max2(entity->health - damage, 0.0f);
	if (entity->health == 0.0f)
	{
		Ability* ability = &entity->ability;
		ability->durationLeft = 0.0f;
	}
}

static void CombatLabReset(CombatLabState* labState)
{
	InitRandom();
	F32 maxHealth = 100.0f;

	labState->running = true;

	Entity* player = &labState->player;
	player->type = WhiteEntity;
	player->position = MakePoint(0.0f, 0.0f);
	player->radius = 1.0f;

	player->maxHealth = maxHealth;
	player->health = maxHealth;

	for (I32 i = 0; i < CombatLabEnemyN; ++i) 
	{
		Entity* enemy = &labState->enemies[i];

		if (i < CombatLabEnemyN / 2)
		{
			enemy->type = RedEntity;
		}
		else
		{
			enemy->type = BlueEntity;
		}

		enemy->position.x = player->position.x + RandomBetween(-20.0f, +20.0f);
		enemy->position.y = player->position.y + RandomBetween(-20.0f, +20.0f);
		enemy->radius = player->radius;

		enemy->maxHealth = maxHealth;
		enemy->health = maxHealth;
	}
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
			labState->useAbility = true;
			break;
		}
		case WM_LBUTTONUP:
		{
			labState->useAbility = false;
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
		case WM_KEYUP: 
		{
			WPARAM keyCode = wparam;

			switch (keyCode) 
			{
				case '1':
				{
					CombatLabReset(labState);
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

	F32 healthRatio = entity->health / entity->maxHealth;
	Assert(IsBetween(healthRatio, 0.0f, 1.0f));
	F32 healthBarFilledX = healthBarLeft + healthRatio * (healthBarRight - healthBarLeft);
	DrawRect(canvas, healthBarLeft, healthBarFilledX, healthBarTop, healthBarBottom, healthBarColor);
}

static B32 IsEntityInSlice(Entity* entity, V2 center, F32 radius, F32 minAngle, F32 maxAngle)
{
	F32 distance = Distance(entity->position, center);
	F32 angle = LineAngle(center, entity->position);
	B32 result = (distance < radius && IsAngleBetween(minAngle, angle, maxAngle));
	return result;
}

static void DamageEnemiesInSlice(CombatLabState* labState, V2 center, F32 radius, F32 minAngle, F32 maxAngle, F32 damage)
{
	for (I32 i = 0; i < CombatLabEnemyN; ++i)
	{
		Entity* enemy = &labState->enemies[i];
		if (IsEntityInSlice(enemy, center, radius, minAngle, maxAngle))
		{
			DoDamage(enemy, damage);
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

static void DamageEnemiesInCircle(CombatLabState* labState, V2 center, F32 radius, F32 damage)
{
	for (I32 i = 0; i < CombatLabEnemyN; ++i) 
	{
		Entity* enemy = &labState->enemies[i];
		if (IsEntityInCircle(enemy, center, radius))
		{
			DoDamage(enemy, damage);
		}
	}
}

static void DrawWhiteAbility(Canvas canvas, Entity* entity)
{
	Assert(entity->type == WhiteEntity);
	V4 color = MakeColor(0.8f, 0.8f, 0.8f);
	F32 radius = entity->radius + WhiteAbilityRadius;

	Ability* ability = &entity->ability;
	if (ability->durationLeft > 0.0f)
	{
		Assert(IsBetween(ability->durationLeft, 0.0f, WhiteAbilityTotalDuration));
		F32 durationRatio = 1.0f - ability->durationLeft / WhiteAbilityTotalDuration;
		F32 fillRadius = entity->radius + (durationRatio) * (radius - entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - WhiteAbilityAngleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + WhiteAbilityAngleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void DrawRedAbility(Canvas canvas, Entity* entity)
{
	Assert(entity->type == RedEntity);
	V4 color = MakeColor(0.8f, 0.0f, 0.0f);
	F32 radius = entity->radius + RedAbilityRadius;

	Ability* ability = &entity->ability;
	if (ability->durationLeft > 0.0f)
	{
		Assert(IsBetween(ability->durationLeft, 0.0f, RedAbilityTotalDuration));
		F32 durationRatio = 1.0f - ability->durationLeft / RedAbilityTotalDuration;
		F32 fillRadius = entity->radius + (durationRatio) * (radius - entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - RedAbilityAngleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + RedAbilityAngleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void DrawBlueAbility(Canvas canvas, Entity* entity)
{
	Assert(entity->type == BlueEntity);
	V4 color = MakeColor(0.0f, 0.0f, 0.8f);
	
	Ability* ability = &entity->ability;
	if (ability->durationLeft > 0.0f)
	{
		Assert(IsBetween(ability->durationLeft, 0.0f, BlueAbilityTotalDuration));
		F32 durationRatio = 1.0f - ability->durationLeft / BlueAbilityTotalDuration;
		F32 fillRadius = durationRatio * BlueAbilityRadius;

		DrawCircleOutline(canvas, ability->position, BlueAbilityRadius, color);
		DrawCircle(canvas, ability->position, fillRadius, color);
	}
}

static void CombatLabUpdate(CombatLabState* labState, V2 mousePosition, F32 seconds)
{
	Entity* player = &labState->player;
	Ability* playerAbility = &player->ability;

	player->velocity = MakeVector(0.0f, 0.0f);
	if (labState->followMouse)
	{
		F32 playerMoveSpeed = 10.0f;
		V2 targetPosition = mousePosition;
		V2 moveDirection = PointDirection(player->position, targetPosition);
		player->velocity = playerMoveSpeed * moveDirection;
	}

	if (player->health > 0.0f && playerAbility->durationLeft == 0.0f)
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

	DrawWhiteAbility(canvas, &labState->player);

	V4 enemyAbilityColor = MakeColor(1.0f, 0.5f, 0.0f);
	for (I32 i = 0; i < CombatLabEnemyN; ++i)
	{
		Entity* enemy = &labState->enemies[i];
		if (enemy->type == RedEntity)
		{
			DrawRedAbility(canvas, &labState->enemies[i]);
		}
		else if (enemy->type == BlueEntity)
		{
			DrawBlueAbility(canvas, &labState->enemies[i]);
		}
		else
		{
			DebugBreak();
		}
	}

	if (player->health > 0.0f)
	{
		if (playerAbility->durationLeft > 0.0f)
		{
			playerAbility->durationLeft = Max2(0.0f, playerAbility->durationLeft - seconds);
			if (playerAbility->durationLeft == 0.0f)
			{
				F32 minAngle = NormalizeAngle(playerAbility->angle - WhiteAbilityAngleRange * 0.5f);
				F32 maxAngle = NormalizeAngle(playerAbility->angle + WhiteAbilityAngleRange * 0.5f);
				DamageEnemiesInSlice(labState, labState->player.position, WhiteAbilityRadius, minAngle, maxAngle, WhiteAbilityDamage);
			}
		}
	
		Assert(playerAbility->durationLeft >= 0.0f);
		if (playerAbility->durationLeft == 0.0f)
		{
			if (labState->useAbility)
			{
				playerAbility->angle = LineAngle(labState->player.position, mousePosition);
				playerAbility->durationLeft = WhiteAbilityTotalDuration;
			}
		}
	}

	if (player->health > 0.0f)
	{
		F32 enemyMoveSpeed = 5.0f;

		Entity* attackingEnemies[CombatLabEnemyN];
		I32 attackingEnemyN = 0;
		for (I32 i = 0; i < CombatLabEnemyN; ++i) 
		{
			Entity* enemy = &labState->enemies[i];

			F32 followDistance = 10.0f;

			if (enemy->health > 0.0f)
			{
				F32 distanceFromPlayer = Distance(enemy->position, player->position);
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
					ability->durationLeft = 0.0f;
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

			F32 attackDistance = 0.0f;
			if (enemy->type == RedEntity)
			{
				attackDistance = 5.0f;
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
			if (ability->durationLeft > 0.0f)
			{
				ability->durationLeft = Max2(0.0f, ability->durationLeft - seconds);
				if (ability->durationLeft == 0.0f)
				{
					if (enemy->type == RedEntity)
					{
						F32 minAngle = NormalizeAngle(ability->angle - RedAbilityAngleRange * 0.5f);
						F32 maxAngle = NormalizeAngle(ability->angle + RedAbilityAngleRange * 0.5f);
						if (IsEntityInSlice(player, enemy->position, RedAbilityRadius, minAngle, maxAngle))
						{
							DoDamage(player, RedAbilityDamage);
						}
					}
					else if (enemy->type == BlueEntity)
					{
						if (IsEntityInCircle(player, ability->position, BlueAbilityRadius))
						{
							DoDamage(player, BlueAbilityDamage);
						}
					}
					else
					{
						DebugBreak();
					}
				}
			}
	
			if (ability->durationLeft == 0.0f)
			{
				if (distanceFromTarget > 0.1f)
				{
					V2 moveDirection = PointDirection(enemy->position, targetPosition);
					enemy->position = enemy->position + seconds * enemyMoveSpeed * moveDirection;
				}
				else
				{
					if (enemy->type == RedEntity)
					{
						ability->angle = LineAngle(enemy->position, player->position);
						ability->durationLeft = RedAbilityTotalDuration;
					}
					else if (enemy->type == BlueEntity)
					{
						ability->position = player->position;
						ability->durationLeft = BlueAbilityTotalDuration;
					}
					else
					{
						DebugBreak();
					}
				}
			}
		}
	}

	DrawEntity(canvas, &labState->player);
	for (I32 i = 0; i < CombatLabEnemyN; i++)
	{
		Entity* enemy = &labState->enemies[i];
		DrawEntity(canvas, enemy);
	}
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

// TODO: Blue ability should slow the player down!
// TODO: Green entities and abilities?
// TODO: Rename Duration to CastTime?
// TODO: Multiple player abilities!