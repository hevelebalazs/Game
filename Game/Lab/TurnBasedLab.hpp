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
	MakeColor(0.8f, 0.0f, 0.0f),
	MakeColor(0.0f, 0.0f, 0.8f)
};

V4 HoverTeamColors[TurnBasedTeamN] = 
{
	MakeColor(0.9f, 0.1f, 0.1f),
	MakeColor(0.1f, 0.0f, 0.9f)
};

struct TurnBasedEntity
{
	TileIndex tileIndex;
	I32 teamIndex;
	I32 actionPoints;
	I32 healthPoints;
	I32 maxHealthPoints;
};

struct TurnBasedAbility
{
	I32 maxDistance;
	I32 damage;
	I32 actionPoints;
};
TurnBasedAbility gTurnBasedAbilities[2];

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
};
TurnBasedLabState gTurnBasedLabState;

static void func TurnBasedLabResize(TurnBasedLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 5.0f;
	camera->center = MakePoint(0.0f, 0.0f);
}

static void func TurnBasedLabBlit(Canvas* canvas, HDC context, RECT rect)
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

B32 operator==(TileIndex index1, TileIndex index2)
{
	B32 result = (index1.row == index2.row && 
				  index1.col == index2.col);
	return result;
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

static B32 func IsValidTeamIndex(I32 index)
{
	B32 isValid = IsIntBetween(index, 0, TurnBasedTeamN - 1);
	return isValid;
}

static void func EndTurn(TurnBasedLabState* labState)
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
	Assert(IsValidTeamIndex(labState->activeTeamIndex));
}

static void func TurnBasedLabInit(TurnBasedLabState* labState, I32 windowWidth, I32 windowHeight)
{
	labState->running = true;
	labState->canvas.glyphData = GetGlobalGlyphData();
	TurnBasedLabResize(labState, windowWidth, windowHeight);

	TurnBasedEntity* entities = labState->entities;
	entities[0].tileIndex = MakeTileIndex(0, 0);
	entities[1].tileIndex = MakeTileIndex(0, TileColN - 1);
	entities[2].tileIndex = MakeTileIndex(TileRowN - 1, 0);
	entities[3].tileIndex = MakeTileIndex(TileRowN - 1, TileColN - 1);

	entities[0].teamIndex = 0;
	entities[1].teamIndex = 0;
	entities[2].teamIndex = 1;
	entities[3].teamIndex = 1;

	labState->activeTeamIndex = 0;
	labState->selectedEntity = 0;
	labState->selectedAbility = 0;

	for (I32 i = 0; i < TurnBasedEntityN; i++)
	{
		TurnBasedEntity* entity = &labState->entities[i];

		entity->actionPoints = MaxActionPoints;

		entity->healthPoints = 10;
		entity->maxHealthPoints = 10;
	}

	TurnBasedAbility* ability0 = &gTurnBasedAbilities[0];
	ability0->actionPoints = 1;
	ability0->damage = 1;
	ability0->maxDistance = 1;

	TurnBasedAbility* ability1 = &gTurnBasedAbilities[1];
	ability1->actionPoints = 3;
	ability1->damage = 5;
	ability1->maxDistance = 1;
}

static TurnBasedEntity* func GetEntityAtTile(TurnBasedLabState* labState, TileIndex index)
{
	TurnBasedEntity* result = 0;
	if (IsValidTileIndex(index))
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

static B32 func IsValidEntityIndex(I32 index)
{
	B32 isValid = IsIntBetween(index, 0, TurnBasedEntityN - 1);
	return isValid;
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

static B32 func IsDead(TurnBasedEntity* entity)
{
	Assert(entity != 0);
	Assert(entity->healthPoints >= 0);
	B32 isDead = (entity->healthPoints == 0);
	return isDead;
}

static B32 func EntityCanMoveTo(TurnBasedLabState* labState, TurnBasedEntity* entity, TileIndex tileIndex)
{
	B32 canMove = false;
	if (entity != 0 && IsValidTileIndex(tileIndex) && !IsDead(entity))
	{
		if (GetTileDistance(entity->tileIndex, tileIndex) <= entity->actionPoints)
		{
			canMove = (GetEntityAtTile(labState, tileIndex) == 0);
		}
	}
	return canMove;
}

static void func MoveEntityTo(TurnBasedLabState* labState, TurnBasedEntity* entity, TileIndex tileIndex)
{
	Assert(EntityCanMoveTo(labState, entity, tileIndex));

	I32 moveDistance = GetTileDistance(entity->tileIndex, tileIndex);
	entity->tileIndex = tileIndex;

	Assert(entity->actionPoints >= moveDistance);
	entity->actionPoints -= moveDistance;
}

static B32 func CanAttack(TurnBasedEntity* source, TurnBasedEntity* target, TurnBasedAbility* ability)
{
	B32 canAttack = false;
	if (source != 0 && target != 0 && source != target && ability != 0)
	{
		I32 distance = GetTileDistance(source->tileIndex, target->tileIndex);
		canAttack = ((distance <= ability->maxDistance) &&
					 (source->actionPoints >= ability->actionPoints) &&
					 !IsDead(source) && !IsDead(target));
	}
	return canAttack;
}

static void func Attack(TurnBasedEntity* source, TurnBasedEntity* target, TurnBasedAbility* ability)
{
	Assert(CanAttack(source, target, ability));
	target->healthPoints = IntMax2(0, target->healthPoints - ability->damage);
	source->actionPoints -= ability->actionPoints;
}

static void func ToggleAbility(TurnBasedLabState* labState, TurnBasedAbility* ability)
{
	if (labState->selectedEntity != 0)
	{
		if (labState->selectedAbility == ability)
		{
			labState->selectedAbility = 0;
		}
		else
		{
			labState->selectedAbility = ability;
		}
	}
}

static void func SelectEntity(TurnBasedLabState* labState, TurnBasedEntity* entity)
{
	labState->selectedEntity = entity;
	labState->selectedAbility = 0;
}

static LRESULT CALLBACK func TurnBasedLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	
	TurnBasedLabState* labState = &gTurnBasedLabState;
	switch (message)
	{
		case WM_SIZE:
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width  = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			TurnBasedLabResize(labState, width, height);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			TurnBasedLabBlit(&labState->canvas, context, clientRect);

			EndPaint(window, &paint);
			break;
		}
		case WM_SETCURSOR:
		{
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}
		case WM_KEYUP:
		{
			WPARAM keyCode = wparam;
			switch (keyCode)
			{
				case '1':
				{
					ToggleAbility(labState, &gTurnBasedAbilities[0]);
					break;
				}
				case '2':
				{
					ToggleAbility(labState, &gTurnBasedAbilities[1]);
					break;
				}
				case 'E':
				{
					EndTurn(labState);
					SelectEntity(labState, 0);
					break;
				}
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			if (IsValidTileIndex(labState->hoverTileIndex))
			{
				TurnBasedEntity* hoverEntity = GetEntityAtTile(labState, labState->hoverTileIndex);
				if (hoverEntity && !IsDead(hoverEntity))
				{
					if (labState->selectedEntity == hoverEntity)
					{
						SelectEntity(labState, 0);
					}
					else if (hoverEntity->teamIndex == labState->activeTeamIndex)
					{
						SelectEntity(labState, hoverEntity);
					}
					if (labState->selectedAbility != 0)
					{
						Assert(labState->selectedEntity != 0);
						if (CanAttack(labState->selectedEntity, hoverEntity, labState->selectedAbility))
						{
							Attack(labState->selectedEntity, hoverEntity, labState->selectedAbility);
						}
						else
						{
							labState->selectedAbility = 0;
						}
					}
				}
				else if (labState->selectedEntity)
				{
					Assert(!IsDead(labState->selectedEntity));
					Assert(labState->selectedEntity->teamIndex == labState->activeTeamIndex);
					if (labState->selectedAbility == 0)
					{
						if (EntityCanMoveTo(labState, labState->selectedEntity, labState->hoverTileIndex))
						{
							MoveEntityTo(labState, labState->selectedEntity, labState->hoverTileIndex);
						}
						else
						{
							SelectEntity(labState, 0);
						}
					}
					else
					{
						labState->selectedAbility = 0;
					}
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

static void func DrawStatusBar(TurnBasedLabState* labState)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	I32 height = 10 + TextHeightInPixels;

	I32 left = 0;
	I32 right = bitmap->width - 1;
	I32 bottom = bitmap->height - 1;
	I32 top = bottom - height;
	V4 backgroundColor = MakeColor(0.1f, 0.1f, 0.1f);
	DrawBitmapRect(bitmap, left, right, top, bottom, backgroundColor);

	GlyphData* glyphData = labState->canvas.glyphData;
	Assert(glyphData != 0);

	I8* text = 0;
	V4 textColor = {1.0f, 1.0f, 1.0f};
	if (labState->selectedAbility != 0)
	{
		text = "Click on an enemy to attack it.";
	}
	else if (labState->selectedEntity != 0)
	{
		text = "Click on the tile to move to.";
	}
	else
	{
		text = "Press 'E' to end turn.";
	}

	Assert(text != 0);
	DrawBitmapTextLineCentered(bitmap, text, glyphData, left, right, top, bottom, textColor);
}

static void func DrawHealthBar(Canvas* canvas, TurnBasedEntity* entity)
{
	Assert(entity->maxHealthPoints > 0);
	V2 tileCenter = GetTileCenter(entity->tileIndex);

	F32 left   = tileCenter.x - TileSide * 0.5f;
	F32 right  = tileCenter.x + TileSide * 0.5f;
	F32 top    = tileCenter.y - TileSide * 0.5f;
	F32 bottom = top + TileSide * 0.2f;

	V4 backgroundColor = MakeColor(0.0f, 0.3f, 0.0f);
	if (IsDead(entity))
	{
		backgroundColor = MakeColor(0.3f, 0.3f, 0.3f);
	}

	DrawRectLRTB(canvas, left, right, top, bottom, backgroundColor);

	F32 healthRatio = F32(entity->healthPoints) / F32(entity->maxHealthPoints);
	F32 filledX = Lerp(left, healthRatio, right);
	V4 filledColor = MakeColor(0.0f, 1.0f, 0.0f);
	DrawRectLRTB(canvas, left, filledX, top, bottom, filledColor);
}

static void func TurnBasedLabUpdate(TurnBasedLabState* labState, V2 mousePosition, F32 seconds)
{
	Canvas* canvas = &labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 tileGridColor = MakeColor(0.2f, 0.2f, 0.2f);

	V4 tileColor = MakeColor(0.5f, 0.5f, 0.5f);
	V4 hoverTileColor = MakeColor(0.6f, 0.6f, 0.6f);

	V4 moveTileColor = MakeColor(0.8f, 0.8f, 0.8f);
	V4 hoverMoveTileColor = MakeColor(0.9f, 0.9f, 0.9f);

	V4 attackTileColor = MakeColor(0.8f, 0.5f, 0.5f);
	V4 hoverAttackTileColor = MakeColor(0.9f, 0.6f, 0.6f);

	labState->hoverTileIndex = GetContainingTileIndex(mousePosition);

	TurnBasedEntity* selectedEntity = labState->selectedEntity;

	for (I32 row = 0; row < TileRowN; row++)
	{
		for (I32 col = 0; col < TileColN; col++)
		{
			TileIndex tileIndex = MakeTileIndex(row, col);
			TurnBasedEntity* entity = GetEntityAtTile(labState, tileIndex);
			TurnBasedAbility* ability = labState->selectedAbility;

			B32 isEntity = (entity != 0);
			B32 isHover = (tileIndex == labState->hoverTileIndex);
			B32 canMove = (ability == 0) && EntityCanMoveTo(labState, selectedEntity, tileIndex);
			B32 canAttack = ((ability != 0) &&
							 GetTileDistance(selectedEntity->tileIndex, tileIndex) <= ability->maxDistance);

			V4 color = tileColor;
			if (isEntity)
			{
				Assert(entity != 0);
				I32 teamIndex = entity->teamIndex;
				Assert(IsValidTeamIndex(teamIndex));

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

			V2 tileCenter = GetTileCenter(tileIndex);
			Rect tileRect = MakeSquareRect(tileCenter, TileSide);
			DrawRect(canvas, tileRect, color);

			if (entity != 0)
			{
				DrawHealthBar(canvas, entity);
			}

			DrawRectOutline(canvas, tileRect, tileGridColor);

			if (entity != 0 && entity->teamIndex == labState->activeTeamIndex && !IsDead(entity))
			{
				I8 actionPointsText[8];
				String actionPointsString = StartString(actionPointsText, 8);
				AddInt(&actionPointsString, entity->actionPoints);
				V2 tileCenter = GetTileCenter(tileIndex);
				V4 textColor = MakeColor(1.0f, 1.0f, 1.0f);
				DrawTextLineXYCentered(canvas, actionPointsText, tileCenter.y, tileCenter.x, textColor);
			}
		}
	}

	Camera* camera = &labState->camera;
	camera->center = MakePoint(TileColN * TileSide * 0.5f, TileRowN * TileSide * 0.5f);

	DrawStatusBar(labState);
}

static void func TurnBasedLab(HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = TurnBasedLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "TurnBasedLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
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
	Assert(window != 0);

	TurnBasedLabState* labState = &gTurnBasedLabState;

	RECT rect = {};
	GetClientRect(window, &rect);
	I32 width  = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	TurnBasedLabInit(labState, width, height);

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
		F32 milliSeconds = (F32(microSeconds) * 1000.0f) / F32(counterFrequency.QuadPart);
		F32 seconds = 0.001f * milliSeconds;
		lastCounter = counter;

		V2 mousePosition = GetMousePosition(&labState->camera, window);
		TurnBasedLabUpdate(labState, mousePosition, seconds);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		TurnBasedLabBlit(&labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}