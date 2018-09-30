#pragma once

#include <Windows.h>

struct Entity
{
	V2 position;
	V2 velocity;
	F32 radius;

	F32 health;
	F32 maxHealth;
};

#define CombatLabEnemyN 10
struct CombatLabState
{
	Camera camera;
	Canvas canvas;
	B32 running;

	Entity player;
	Entity enemies[CombatLabEnemyN];

	F32 circleRadius;
	F32 circleTotalDuration;
	F32 circleDurationLeft;
	B32 circleEnabled;
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
		case WM_KEYDOWN:
		{
			WPARAM keyCode = wparam;
			switch (keyCode)
			{
				case 'W':
				{
					player->velocity.y = -10.0f;
					break;
				}
				case 'S': {
					player->velocity.y = +10.0f;
					break;
				}
				case 'A':
				{
					player->velocity.x = -10.0f;
					break;
				}
				case 'D':
				{
					player->velocity.x = +10.0f;
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
				case 'W': 
				case 'S':
				{
					player->velocity.y = 0.0f;
					break;
				}
				case 'A':
				case 'D':
				{
					player->velocity.x = 0.0f;
					break;
				}
				case 'F':
				{
					DoDamage(player, 20.0f);
					break;
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			labState->circleEnabled = true;
			break;
		}
		case WM_LBUTTONUP:
		{
			labState->circleEnabled = false;
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
		enemy->position.x = player->position.x + RandomBetween(-10.0f, +10.0f);
		enemy->position.y = player->position.y + RandomBetween(-10.0f, +10.0f);
		enemy->radius = 1.0f;

		enemy->maxHealth = maxHealth;
		enemy->health = maxHealth;
	}

	labState->circleTotalDuration = 0.5f;

	CombatLabResize(labState, windowWidth, windowHeight);
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

static void DrawCircleBorder(Canvas canvas, V2 center, F32 radius, V4 color)
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

static void DrawEntity(Canvas canvas, Entity* entity, V4 color)
{
	DrawCircle(canvas, entity->position, entity->radius, color);

	V4 healthBarBackgroundColor = MakeColor(0.5f, 0.0f, 0.0f);
	V4 healthBarColor = MakeColor(1.0f, 0.0f, 0.0f);
	F32 healthBarWidth = 2.0f * entity->radius;
	F32 healthBarHeight = 0.1f * healthBarWidth;

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

static void CombatLabUpdate(CombatLabState* labState, F32 seconds)
{
	Entity* player = &labState->player;
	if (labState->circleDurationLeft == 0.0f)
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

	if (labState->circleDurationLeft > 0.0f)
	{
		labState->circleDurationLeft = Max2(0.0f, labState->circleDurationLeft - seconds);
		if (labState->circleDurationLeft > 0.0f)
		{
			F32 durationRatio = 1.0f - labState->circleDurationLeft / labState->circleTotalDuration;
			V4 circleColor = MakeColor(1.0f, 1.0f, 0.0f);
			F32 fillRadius = labState->player.radius + (durationRatio) * (labState->circleRadius - labState->player.radius);
			DrawCircleBorder(canvas, labState->player.position, labState->circleRadius, circleColor);
			DrawCircle(canvas, labState->player.position, fillRadius, circleColor);
		}
		else
		{
			DamageEnemiesInCircle(labState, labState->player.position, labState->circleRadius, 10.0f);
		}
	}
	
	Assert(labState->circleDurationLeft >= 0.0f);
	if (labState->circleDurationLeft == 0.0f)
	{
		if (labState->circleEnabled)
		{
			labState->circleRadius = 3.0f;
			labState->circleDurationLeft = labState->circleTotalDuration;
		}
	}

	V4 playerColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawEntity(canvas, &labState->player, playerColor);

	V4 enemyColor = MakeColor(0.5f, 0.0f, 0.0f);
	for (I32 i = 0; i < CombatLabEnemyN; i++)
		DrawEntity(canvas, &labState->enemies[i], enemyColor);
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

		CombatLabUpdate(labState, seconds);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		CombatLabBlit(labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}

// TODO: Damage in an arc?
// TODO: Enemy movement
// TODO: Enemy abilities
// TODO: Disable controls while dead!
// TODO: Better controls (releasing/pressing buttons)!
// TODO: Better physics?