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

#define EntityRadius (0.25f * TileSide)
#define EntityMaxHealth 100
struct Entity
{
	V2 position;
	F32 recharge;
	F32 rechargeFrom;
	I32 health;
};

struct CombatLabState
{
	Camera camera;
	Canvas canvas;
	B32 running;

	V2 playerMoveDirection;

	Entity player;
	Entity enemy;
};
CombatLabState gCombatLabState;

#define PlayerSpeed 20.0f

#define EnemyAttackRadius 10.0f

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

	Entity* player = &labState->player;
	player->position = mapCenter;
	player->health = EntityMaxHealth;
	labState->playerMoveDirection = MakePoint(0.0f, 0.0f);

	Entity* enemy = &labState->enemy;
	enemy->position = MakePoint(mapWidth - EntityRadius, mapHeight - EntityRadius);
	enemy->health = EntityMaxHealth;
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

static LRESULT CALLBACK func CombatLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	
	CombatLabState* labState = &gCombatLabState;
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
				case 'A':
				{
					labState->playerMoveDirection.x = -1.0f;
					break;
				}
				case 'D':
				{
					labState->playerMoveDirection.x = +1.0f;
					break;
				}
				case 'W':
				{
					labState->playerMoveDirection.y = -1.0f;
					break;
				}
				case 'S':
				{
					labState->playerMoveDirection.y = +1.0f;
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
				case 'A':
				case 'D':
				{
					labState->playerMoveDirection.x = 0.0f;
					break;
				}
				case 'W':
				case 'S':
				{
					labState->playerMoveDirection.y = 0.0f;
					break;
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

	if(entity->recharge > 0.0f)
	{
		Assert(entity->recharge <= entity->rechargeFrom);
		F32 rechargeRatio = entity->recharge / entity->rechargeFrom;
		V4 rechargeBarBackgroundColor = MakeColor(0.5f, 0.5f, 0.5f);
		V4 rechargeBarColor = MakeColor(1.0f, 1.0f, 0.0f);
		V4 rechargeBarOutlineColor = MakeColor(0.0f, 0.0f, 0.0f);
		V2 rechargeBarCenter = entity->position + MakeVector(0.0f, -1.8f * EntityRadius);
		F32 rechargeBarWidth = 2.0f * EntityRadius;
		F32 rechargeBarHeight = 0.3f * EntityRadius;
		Rect rechargeBar = MakeRect(rechargeBarCenter, rechargeBarWidth, rechargeBarHeight);
		DrawRect(canvas, rechargeBar, rechargeBarBackgroundColor);
		Rect rechargeBarFilled = rechargeBar;
		rechargeBarFilled.right = Lerp(rechargeBarFilled.left, rechargeRatio, rechargeBarFilled.right);
		DrawRect(canvas, rechargeBarFilled, rechargeBarColor);
		DrawRectOutline(canvas, rechargeBar, rechargeBarOutlineColor);
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

	Entity* player = &labState->player;
	Entity* enemy = &labState->enemy;

	player->position = player->position + PlayerSpeed * seconds * labState->playerMoveDirection;

	player->position.x = Clip(player->position.x, EntityRadius, mapWidth - EntityRadius);
	player->position.y = Clip(player->position.y, EntityRadius, mapHeight - EntityRadius);


	V4 playerColor = MakeColor(0.0f, 1.0f, 1.0f);
	DrawEntity(canvas, player, playerColor);

	V4 enemyColor = MakeColor(1.0f, 0.0f, 0.0f);
	DrawEntity(canvas, enemy, enemyColor);

	B32 foundTargetTile = false;
	F32 enemyTargetDistance = 0.0f;

	V2 targetDirection = {};
	V2 targetPosition = {};
	TileIndex playerTileIndex = GetContainingTileIndex(player->position);
	F32 attackDistance = 2.5f * EntityRadius;

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

			TileIndex tileIndex = MakeTileIndex(row, col);
			if(IsValidTileIndex(tileIndex))
			{
				V2 position = player->position + attackDistance * direction;
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
	F32 enemyMoveDistance = seconds * enemyMoveSpeed;

	if(enemy->position.x < targetPosition.x)
	{
		enemy->position.x = Min2(enemy->position.x + enemyMoveDistance, targetPosition.x);
	}
	else
	{
		enemy->position.x = Max2(enemy->position.x - enemyMoveDistance, targetPosition.x);
	}

	if(enemy->position.y < targetPosition.y)
	{
		enemy->position.y = Min2(enemy->position.y + enemyMoveDistance, targetPosition.y);
	}
	else
	{
		enemy->position.y = Max2(enemy->position.y - enemyMoveDistance, targetPosition.y);
	}

	enemy->recharge = Max2(enemy->recharge - seconds, 0.0f);

	if(MaxDistance(enemy->position, player->position) <= EnemyAttackRadius)
	{
		if(enemy->recharge == 0.0f)
		{
			player->health = IntMax2(player->health - 10, 0);

			enemy->rechargeFrom = 3.0f;
			enemy->recharge = enemy->rechargeFrom;
		}
	}

	DrawEntityBars(canvas, player);
	DrawEntityBars(canvas, enemy);

	V2 mapCenter = MakePoint(mapWidth * 0.5f, mapHeight * 0.5f);
	Camera* camera = &labState->camera;
	camera->center = mapCenter;
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

// TODO: Real time Monk abilities!
// TODO: Computer controlled enemies
	// TODO: Only get pulled from a certain distance!
	// TODO: Multiple enemies
	// TODO: Don't stack upon each other (find a simple solution in this task)
// TODO: Cult Warrior class
// TODO: Paladin class