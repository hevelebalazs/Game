#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Geometry.hpp"

struct Ability
{
	F32 radius;
	F32 totalDuration;
	F32 durationLeft;

	F32 totalCooldown;
	F32 cooldownRemaining;

	F32 angle;
	F32 angleRange;
	F32 damage;
};

struct Entity
{
	V2 position;
	V2 velocity;
	F32 radius;

	F32 health;
	F32 maxHealth;

	Ability ability;

	F32 attackFromAngle;
};

enum CombatLabMode 
{
	FixedDirectionMode,
	ChangingDirectionMode,
	TargetSelectionMode
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

	CombatLabMode mode;
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
	player->position = MakePoint(0.0f, 0.0f);
	player->radius = 1.0f;

	player->maxHealth = maxHealth;
	player->health = maxHealth;

	for (I32 i = 0; i < CombatLabEnemyN; ++i) 
	{
		Entity* enemy = &labState->enemies[i];
		enemy->position.x = player->position.x + RandomBetween(-20.0f, +20.0f);
		enemy->position.y = player->position.y + RandomBetween(-20.0f, +20.0f);
		enemy->radius = player->radius;

		enemy->maxHealth = maxHealth;
		enemy->health = maxHealth;
	}
	
	Ability* ability = &labState->player.ability;
	ability->totalDuration = 0.5f;
	ability->radius = 5.0f;
	ability->angleRange = 0.25f * PI;
	ability->damage = 30.0f;
	ability->totalCooldown = 1.0f;
	ability->cooldownRemaining = 0.0f;

	for (I32 i = 0; i < CombatLabEnemyN; ++i)
	{
		Ability* ability = &labState->enemies[i].ability;
		ability->totalDuration = 1.0f;
		ability->radius = 5.0f;
		ability->angleRange = 0.5f * PI;
		if (labState->mode == TargetSelectionMode)
		{
			ability->damage = 5.0f;
		}
		else
		{
			ability->damage = 30.0f;
		}
		ability->totalCooldown = 1.5f;
		ability->cooldownRemaining = 0.0f;
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
					labState->mode = FixedDirectionMode;
					CombatLabReset(labState);
					break;
				}
				case '2':
				{
					labState->mode = TargetSelectionMode;
					CombatLabReset(labState);
					break;
				}
				case '3':
				{
					labState->mode = ChangingDirectionMode;
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
	labState->mode = FixedDirectionMode;

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
		IntSwap(&topPixel, &bottomPixel);

	if (leftPixel > rightPixel)
		IntSwap(&leftPixel, &rightPixel);

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
				*pixel = colorCode;
		}
	}
}

static void DrawSliceOutline(Canvas canvas, V2 center, F32 radius, F32 minAngle, F32 maxAngle, V4 color)
{
	if (minAngle > maxAngle)
		minAngle -= TAU;
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
		IntSwap(&topPixel, &bottomPixel);

	if (leftPixel > rightPixel)
		IntSwap(&leftPixel, &rightPixel);

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
				*pixel = colorCode;
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

static void DrawEntityHover(Canvas canvas, Entity* entity, V4 color)
{
	F32 hoverWidth = 0.1f;
	DrawCircle(canvas, entity->position, entity->radius + hoverWidth, color);
}

static void DrawCooldown(Canvas canvas, Entity* entity, V4 color)
{
	Ability* ability = &entity->ability;
	if (ability->cooldownRemaining > 0.0f)
	{
		Assert(ability->totalCooldown > 0.0f);
		F32 radius = entity->radius * (ability->cooldownRemaining / ability->totalCooldown);
		DrawCircle(canvas, entity->position, radius, color);
	}
}

static void DrawEntity(Canvas canvas, Entity* entity, V4 color)
{
	DrawCircle(canvas, entity->position, entity->radius, color);

	V4 healthBarBackgroundColor = {};
	if (entity->health > 0.0f)
		healthBarBackgroundColor = MakeColor(0.5f, 0.0f, 0.0f);
	else
		healthBarBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);

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
			DoDamage(enemy, damage);
	}
}

static void DamageEnemiesInCircle(CombatLabState* labState, V2 center, F32 radius, F32 damage)
{
	for (I32 i = 0; i < CombatLabEnemyN; ++i) 
	{
		Entity* enemy = &labState->enemies[i];
		F32 enemyCircleDistance = Distance(enemy->position, center);
		if (enemyCircleDistance < radius + enemy->radius)
			DoDamage(enemy, damage);
	}
}

static void DrawEntityAbility(Canvas canvas, Entity* entity, V4 color)
{
	Ability* ability = &entity->ability;
	if (ability->durationLeft > 0.0f)
	{
		F32 durationRatio = 1.0f - ability->durationLeft / ability->totalDuration;
		F32 fillRadius = entity->radius + (durationRatio) * (ability->radius - entity->radius);

		F32 minAngle = NormalizeAngle(ability->angle - ability->angleRange * 0.5f);
		F32 maxAngle = NormalizeAngle(ability->angle + ability->angleRange * 0.5f);
		DrawSliceOutline(canvas, entity->position, ability->radius, minAngle, maxAngle, color);
		DrawSlice(canvas, entity->position, fillRadius, minAngle, maxAngle, color);
	}
}

static void CombatLabUpdate(CombatLabState* labState, V2 mousePosition, F32 seconds)
{
	Entity* player = &labState->player;
	Ability* playerAbility = &player->ability;

	F32 maxAttackDistance = 5.0f;

	player->velocity = MakeVector(0.0f, 0.0f);
	if (labState->followMouse)
	{
		F32 playerMoveSpeed = 10.0f;
		V2 targetPosition = mousePosition;
		V2 moveDirection = PointDirection(player->position, targetPosition);
		player->velocity = playerMoveSpeed * moveDirection;
	}

	if (player->health > 0.0f && playerAbility->durationLeft == 0.0f)
		player->position = player->position + seconds * player->velocity;

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

	V4 playerAbilityColor = MakeColor(1.0f, 1.0f, 0.0f);
	DrawEntityAbility(canvas, &labState->player, playerAbilityColor);

	V4 enemyAbilityColor = MakeColor(1.0f, 0.5f, 0.0f);
	for (I32 i = 0; i < CombatLabEnemyN; ++i)
	{
		DrawEntityAbility(canvas, &labState->enemies[i], enemyAbilityColor);
	}

	if (labState->mode == TargetSelectionMode)
	{
		for (I32 i = 0; i < CombatLabEnemyN; ++i)
		{
			Entity* enemy = &labState->enemies[i];
			Ability* ability = &enemy->ability;
			if (enemy->health == 0.0f)
			{
				ability->cooldownRemaining = 0.0f;
			}
			else
			{
				ability->cooldownRemaining = Max2(0.0f, ability->cooldownRemaining - seconds);
			}
		}
	}

	if (player->health > 0.0f)
	{
		if (labState->mode == FixedDirectionMode || labState->mode == ChangingDirectionMode)
		{
			if (playerAbility->durationLeft > 0.0f)
			{
				if (labState->mode == ChangingDirectionMode)
				{
					playerAbility->angle = LineAngle(player->position, mousePosition);
				}

				playerAbility->durationLeft = Max2(0.0f, playerAbility->durationLeft - seconds);
				if (playerAbility->durationLeft == 0.0f)
				{
					F32 minAngle = NormalizeAngle(playerAbility->angle - playerAbility->angleRange * 0.5f);
					F32 maxAngle = NormalizeAngle(playerAbility->angle + playerAbility->angleRange * 0.5f);
					DamageEnemiesInSlice(labState, labState->player.position, playerAbility->radius, minAngle, maxAngle, playerAbility->damage);
				}
			}
	
			Assert(playerAbility->durationLeft >= 0.0f);
			if (playerAbility->durationLeft == 0.0f)
			{
				if (labState->useAbility)
				{
					playerAbility->angle = LineAngle(labState->player.position, mousePosition);
					playerAbility->durationLeft = playerAbility->totalDuration;
				}
			}
		}
		else if (labState->mode == TargetSelectionMode)
		{
			Assert(playerAbility->cooldownRemaining >= 0.0f);
			if (playerAbility->cooldownRemaining > 0.0f)
			{
				playerAbility->cooldownRemaining = Max2(0.0f, playerAbility->cooldownRemaining - seconds);
			}
			else if (labState->useAbility)
			{
				Entity* targetEnemy = GetFirstEnemyAtPosition(labState, mousePosition);
				if (targetEnemy && Distance(player->position, targetEnemy->position) < maxAttackDistance)
				{
					DoDamage(targetEnemy, playerAbility->damage);
					playerAbility->cooldownRemaining = playerAbility->totalCooldown;
				}
			}
		}
	}

	if (player->health > 0.0f)
	{
		F32 enemyMoveSpeed = 5.0f;
		F32 attackDistance = 4.0f;
		F32 followDistance = 10.0f;

		Entity* attackingEnemies[CombatLabEnemyN];
		I32 attackingEnemyN = 0;
		for (I32 i = 0; i < CombatLabEnemyN; ++i) 
		{
			Entity* enemy = &labState->enemies[i];

			if (enemy->health > 0.0f)
			{
				F32 distanceFromPlayer = Distance(enemy->position, player->position);
				if (distanceFromPlayer <= followDistance)
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

			V2 targetPosition = player->position + attackDistance * RotationVector(enemy->attackFromAngle);
			F32 distanceFromTarget = Distance(enemy->position, targetPosition);

			Ability* ability = &enemy->ability;
			if (labState->mode == FixedDirectionMode || labState->mode == ChangingDirectionMode)
			{
				if (ability->durationLeft > 0.0f)
				{
					if (labState->mode == ChangingDirectionMode)
					{
						ability->angle = LineAngle(enemy->position, player->position);
					}

					ability->durationLeft = Max2(0.0f, ability->durationLeft - seconds);
					if (ability->durationLeft == 0.0f)
					{
						F32 minAngle = NormalizeAngle(ability->angle - ability->angleRange * 0.5f);
						F32 maxAngle = NormalizeAngle(ability->angle + ability->angleRange * 0.5f);
						if (IsEntityInSlice (player, enemy->position, ability->radius, minAngle, maxAngle))
						{
							DoDamage(player, ability->damage);
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
						ability->angle = LineAngle(enemy->position, player->position);
						ability->durationLeft = ability->totalDuration;
					}
				}
			}
			else if (labState->mode == TargetSelectionMode)
			{
				Assert(ability->cooldownRemaining >= 0.0f);
				if (distanceFromTarget > 0.1f)
				{
					V2 moveDirection = PointDirection(enemy->position, targetPosition);
					enemy->position = enemy->position + seconds * enemyMoveSpeed * moveDirection;
				}
				else if (ability->cooldownRemaining == 0.0f)
				{
					DoDamage(player, ability->damage);
					ability->cooldownRemaining = ability->totalCooldown;
				}
			}
		}
	}

	V4 cooldownColor = MakeColor(0.5f, 0.5f, 0.5f);

	V4 playerColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawEntity(canvas, &labState->player, playerColor);
	if (labState->mode == TargetSelectionMode)
	{
		DrawCooldown(canvas, player, cooldownColor);
	}

	V4 enemyColor = MakeColor(0.5f, 0.0f, 0.0f);
	V4 enemyHoverEnabledColor = MakeColor(1.0f, 1.0f, 1.0f);
	V4 enemyHoverDisabledColor = MakeColor(1.0f, 0.5f, 0.0f);

	if (labState->mode == TargetSelectionMode)
	{
		Entity* hoverEnemy = GetFirstEnemyAtPosition(labState, mousePosition);
		if (hoverEnemy)
		{
			F32 distanceFromPlayer = Distance(hoverEnemy->position, player->position);
			V4 hoverColor = (distanceFromPlayer > maxAttackDistance) ? enemyHoverDisabledColor : enemyHoverEnabledColor;
			DrawEntityHover(canvas, hoverEnemy, hoverColor);
		}
	}

	for (I32 i = 0; i < CombatLabEnemyN; i++)
	{
		Entity* enemy = &labState->enemies[i];
		DrawEntity(canvas, enemy, enemyColor);

		if (labState->mode == TargetSelectionMode)
		{
			DrawCooldown(canvas, enemy, cooldownColor);
		}
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

// [TODO: When multiple enemies are attacking the player, they shouldn't stack upon each other!]
// TODO: At this point separating player and enemy structures seems like a good idea!
// TODO: Multiple player abilities!