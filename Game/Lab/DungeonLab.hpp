#pragma once

#include "Lab.hpp"

#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../DungeonMap.hpp"

#define PlayerSide 1.0f
#define EnemySide 1.0f

#define MaxDungeonLabEnemyN 512

struct PointCollision
{
	V2 position;
	I32 collidingLineN;
	Line collidingLine1;
	Line collidingLine2;
};

struct DungeonLabEnemy
{
	V2 position;
	B32 followsPlayer;
};

struct DungeonLabState
{
	Camera camera;
	Canvas canvas;
	B32 running;
	
	DungeonMap map;

	V2 playerPosition;
	V2 playerVelocity;

	DungeonLabEnemy enemies[MaxDungeonLabEnemyN];
	I32 enemyN;

	PointCollision playerCollision;
};
DungeonLabState gDungeonLabState;

static void func DungeonLabResize(DungeonLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 5.0f;
	camera->center = MakePoint(0.0f, 0.0f);

	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(&labState->canvas.bitmap, backgroundColor);
}

static void func DungeonLabBlit(Canvas* canvas, HDC context, RECT rect)
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

static void func CreateEnemyInRoom(DungeonLabState* labState, Room* room)
{
	Assert(labState->enemyN < MaxDungeonLabEnemyN);
	DungeonLabEnemy* enemy = &labState->enemies[labState->enemyN];
	labState->enemyN++;

	F32 maxDistanceFromCenter = RoomSide * 0.5f - EnemySide * 0.5f;
	V2 roomOffset = {};
	roomOffset.x = RandomBetween(-maxDistanceFromCenter, +maxDistanceFromCenter);
	roomOffset.y = RandomBetween(-maxDistanceFromCenter, +maxDistanceFromCenter);

	enemy->position = room->center + roomOffset;
}

static void func DungeonLabInit(DungeonLabState* labState, I32 windowWidth, I32 windowHeight)
{
	InitRandom();

	labState->running = true;
	labState->canvas.glyphData = GetGlobalGlyphData();
	DungeonLabResize(labState, windowWidth, windowHeight);

	DungeonMap* map = &labState->map;
	InitDungeonMap(map);

	RoomIndex startRoomIndex = GetRandomRoomIndex(map);
	Room* startRoom = GetRoomAtIndex(map, startRoomIndex);

	labState->playerPosition = startRoom->center;
	labState->enemyN = 0;

	for (I32 row = 0; row < RoomRowN; row++)
	{
		for (I32 col = 0; col < RoomColN; col++)
		{
			Room* room = &map->rooms[row * RoomColN + col];
			I32 enemyN = IntAbs(row - startRoomIndex.row) + IntAbs(col - startRoomIndex.col);
			for (I32 i = 0; i < enemyN; i++)
			{
				CreateEnemyInRoom(labState, room);
			}
		}
	}

	labState->camera.unitInPixels = 50.0f;
}

static LRESULT CALLBACK func DungeonLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	F32 playerSpeed = 5.0f;

	DungeonLabState* labState = &gDungeonLabState;
	switch (message)
	{
		case WM_SIZE:
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width  = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			DungeonLabResize(labState, width, height);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			DungeonLabBlit(&labState->canvas, context, clientRect);

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
					labState->playerVelocity.y = -playerSpeed;
					break;
				}
				case 'S':
				{
					labState->playerVelocity.y = +playerSpeed;
					break;
				}
				case 'A':
				{
					labState->playerVelocity.x = -playerSpeed;
					break;
				}
				case 'D':
				{
					labState->playerVelocity.x = +playerSpeed;
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
					labState->playerVelocity.y = 0.0f;
					break;
				}
				case 'A':
				case 'D':
				{
					labState->playerVelocity.x = 0.0f;
					break;
				}
			}
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

struct LineArray
{
	Line* lines;
	I32 size;
	I32 maxSize;
};

static LineArray func MakeLineArray(Line* lines, I32 maxLineN)
{
	LineArray lineArray = {};
	lineArray.lines = lines;
	lineArray.size = 0;
	lineArray.maxSize = maxLineN;
	return lineArray;
}

static void func AddLineToArray(LineArray* lines, Line line)
{
	Assert(lines->size < lines->maxSize);
	lines->lines[lines->size] = line;
	lines->size++;
}

static void func GetRoomWallsOnSide(V2 point1, V2 point2, B32 hasAisle, LineArray* lines)
{
	if (hasAisle)
	{
		F32 ratio1 = (RoomSide * 0.5f - AisleWidth * 0.5f) / RoomSide;
		F32 ratio2 = (RoomSide * 0.5f + AisleWidth * 0.5f) / RoomSide;
		V2 middlePoint1 = PointLerp(point1, ratio1, point2);
		V2 middlePoint2 = PointLerp(point1, ratio2, point2);
		Line line1 = MakeLine(point1, middlePoint1);
		Line line2 = MakeLine(middlePoint2, point2);
		AddLineToArray(lines, line1);
		AddLineToArray(lines, line2);
	}
	else
	{
		Line line = MakeLine(point1, point2);
		AddLineToArray(lines, line);
	}
}

static void func GetRoomWalls(Room* room, LineArray* lines)
{
	V2 center = room->center;
	V2 topLeft     = MakePoint(center.x - RoomSide * 0.5f, center.y - RoomSide * 0.5f);
	V2 topRight    = MakePoint(center.x + RoomSide * 0.5f, center.y - RoomSide * 0.5f);
	V2 bottomLeft  = MakePoint(center.x - RoomSide * 0.5f, center.y + RoomSide * 0.5f);
	V2 bottomRight = MakePoint(center.x + RoomSide * 0.5f, center.y + RoomSide * 0.5f);

	GetRoomWallsOnSide(topLeft,     topRight,    room->topAisle != 0,    lines);
	GetRoomWallsOnSide(topRight,    bottomRight, room->rightAisle != 0,  lines);
	GetRoomWallsOnSide(bottomRight, bottomLeft,  room->bottomAisle != 0, lines);
	GetRoomWallsOnSide(bottomLeft,  topLeft,     room->leftAisle != 0,   lines);
}

static void func GetRoomExtendedWallsOnSide(V2 point1, V2 point2, B32 hasAisle, F32 radius, LineArray* lines)
{
	if (hasAisle)
	{	
		V2 normal = NormalVector(point2 - point1);
		V2 center = 0.5f * (point1 + point2);
		V2 middlePoint1 = center - (AisleWidth * 0.5f - radius) * normal;
		V2 middlePoint2 = center + (AisleWidth * 0.5f - radius) * normal;

		Line line1 = MakeLine(point1, middlePoint1);
		Line line2 = MakeLine(point2, middlePoint2);
		AddLineToArray(lines, line1);
		AddLineToArray(lines, line2);
	}
	else
	{
		Line line = MakeLine(point1, point2);
		AddLineToArray(lines, line);
	}
}

static void func GetRoomExtendedWalls(Room* room, F32 radius, LineArray* lines)
{
	V2 center = room->center;
	F32 sideFromCenter = RoomSide * 0.5f - radius;
	V2 topLeft     = MakePoint(center.x - sideFromCenter, center.y - sideFromCenter);
	V2 topRight    = MakePoint(center.x + sideFromCenter, center.y - sideFromCenter);
	V2 bottomRight = MakePoint(center.x + sideFromCenter, center.y + sideFromCenter);
	V2 bottomLeft  = MakePoint(center.x - sideFromCenter, center.y + sideFromCenter);
	GetRoomExtendedWallsOnSide(topLeft,     topRight,    room->topAisle != 0,    radius, lines);
	GetRoomExtendedWallsOnSide(topRight,    bottomRight, room->rightAisle != 0,  radius, lines);
	GetRoomExtendedWallsOnSide(bottomRight, bottomLeft,  room->bottomAisle != 0, radius, lines);
	GetRoomExtendedWallsOnSide(bottomLeft,  topLeft,     room->leftAisle != 0,   radius, lines);
}

static void func GetAisleWalls(Aisle* aisle, LineArray* lines)
{
	if (aisle->isVertical)
	{
		V2 top    = GetRoomSideCenter(aisle->room1, RoomSideBottom);
		V2 bottom = GetRoomSideCenter(aisle->room2, RoomSideTop);
		V2 addSide = MakeVector(AisleWidth * 0.5f, 0.0f);

		Line line1 = MakeLine(top - addSide, bottom - addSide);
		AddLineToArray(lines, line1);

		Line line2 = MakeLine(top + addSide, bottom + addSide);
		AddLineToArray(lines, line2);
	}
	else
	{
		V2 left  = GetRoomSideCenter(aisle->room1, RoomSideRight);
		V2 right = GetRoomSideCenter(aisle->room2, RoomSideLeft);
		V2 addSide = MakeVector(0.0f, AisleWidth * 0.5f);

		Line line1 = MakeLine(left - addSide, right - addSide);
		AddLineToArray(lines, line1);

		Line line2 = MakeLine(left + addSide, right + addSide);
		AddLineToArray(lines, line2);
	}
}

static void func GetAisleExtendedWalls(Aisle* aisle, F32 radius, LineArray* lines)
{
	if (aisle->isVertical)
	{
		V2 top    = GetRoomSideCenter(aisle->room1, RoomSideBottom) - MakeVector(0.0f, radius);
		V2 bottom = GetRoomSideCenter(aisle->room2, RoomSideTop)    + MakeVector(0.0f, radius);
		V2 addSide = MakeVector(AisleWidth * 0.5f - radius, 0.0f);

		Line line1 = MakeLine(top - addSide, bottom - addSide);
		AddLineToArray(lines, line1);

		Line line2 = MakeLine(top + addSide, bottom + addSide);
		AddLineToArray(lines, line2);
	}
	else
	{
		V2 left  = GetRoomSideCenter(aisle->room1, RoomSideRight) - MakeVector(radius, 0.0f);
		V2 right = GetRoomSideCenter(aisle->room2, RoomSideLeft)  + MakeVector(radius, 0.0f);
		V2 addSide = MakeVector(0.0f, AisleWidth * 0.5f - radius);

		Line line1 = MakeLine(left - addSide, right - addSide);
		AddLineToArray(lines, line1);

		Line line2 = MakeLine(left + addSide, right + addSide);
		AddLineToArray(lines, line2);
	}
}

static void func DrawLines(Canvas* canvas, LineArray* lines, V4 color)
{
	for (I32 i = 0; i < lines->size; i++)
	{
		Line line = lines->lines[i];
		Bresenham(canvas, line.p1, line.p2, color);
	}
}

static void func DrawDungeonMapWalls(Canvas* canvas, DungeonMap* map)
{
	Line walls[8];
	LineArray lines = MakeLineArray(walls, 8);

	V4 wallColor = MakeColor(1.0f, 1.0f, 1.0f);
	for (I32 i = 0; i < RoomN; i++)
	{
		Room* room = &map->rooms[i];

		lines.size = 0;
		GetRoomWalls(room, &lines);
		DrawLines(canvas, &lines, wallColor);
	}

	for (I32 i = 0; i < AisleN; i++)
	{
		Aisle* aisle = &map->aisles[i];

		lines.size = 0;
		GetAisleWalls(aisle, &lines);
		DrawLines(canvas, &lines, wallColor);
	}	
}

static Line func InvertLine(Line line)
{
	Line result = {};
	result.p1 = line.p2;
	result.p2 = line.p1;
	return result;
}

static B32 func LinesAreTheSame(Line line1, Line line2)
{
	B32 result = 
		((line1.p1 == line2.p1) && (line1.p2 == line2.p2)) ||
		((line1.p1 == line2.p2) && (line1.p2 == line2.p1));
	return result;
}

static void func CheckCollisionWithLine(Line line, PointCollision* oldCollision, PointCollision* newCollision)
{
	if (oldCollision->collidingLineN == 0)
	{
		if (DoLinesCross(oldCollision->position, newCollision->position, line.p1, line.p2))
		{
			newCollision->position = LineIntersection(oldCollision->position, newCollision->position, line.p1, line.p2);

			Line collidingLine = line;
			if (!TurnsRight(line.p1, line.p2, oldCollision->position))
			{
				collidingLine = InvertLine(line);
			}

			newCollision->collidingLineN = 1;
			newCollision->collidingLine1 = collidingLine;
		}
	}
	else if (oldCollision->collidingLineN == 1)
	{
		Line line1 = oldCollision->collidingLine1;
		if (!LinesAreTheSame(line, line1))
		{
			B32 hasCommonPoint = false;
			V2 commonPoint = {};

			B32 isOnSameSide = false;
			if (line.p1 == line1.p1 || line.p1 == line1.p2)
			{
				hasCommonPoint = true;
				commonPoint = line.p1;
				isOnSameSide = TurnsRight(line1.p1, line1.p2, line.p2);
			}
			else if (line.p2 == line1.p1 || line.p2 == line1.p2)
			{
				hasCommonPoint = true;
				commonPoint = line.p2;
				isOnSameSide = TurnsRight(line1.p1, line1.p2, line.p1);
			}

			if (hasCommonPoint)
			{
				if (isOnSameSide)
				{
					B32 turnsRightBefore = TurnsRight(line.p1, line.p2, oldCollision->position);
					B32 turnsRightAfter  = TurnsRight(line.p1, line.p2, newCollision->position);
					if (turnsRightBefore != turnsRightAfter)
					{
						newCollision->collidingLineN = 2;
						newCollision->position = commonPoint;
						
						V2 otherPoint = {};
						if (commonPoint == line1.p1)
						{
							otherPoint = line1.p2;
						}
						else if (commonPoint == line1.p2)
						{
							otherPoint = line1.p1;
						}
						else
						{
							DebugBreak();
						}

						if (TurnsRight(line.p1, line.p2, otherPoint))
						{
							newCollision->collidingLine2 = line;
						}
						else
						{
							newCollision->collidingLine2 = InvertLine(line);
						}
					}
				}
			}
			else
			{
				if (DoLinesCross(oldCollision->position, newCollision->position, line.p1, line.p2))
				{
					newCollision->position = LineIntersection(oldCollision->position, newCollision->position, line.p1, line.p2);

					Line collidingLine = line;
					if (!TurnsRight(line.p1, line.p2, oldCollision->position))
					{
						collidingLine = InvertLine(line);
					}

					newCollision->collidingLineN = 2;
					newCollision->collidingLine2 = collidingLine;
				}
			}
		}
	}
	else
	{
		DebugBreak();
	}
}

static void func CheckCollisionWithAllWalls(DungeonMap* map, PointCollision* oldCollision, PointCollision* newCollision)
{
	Line lines[8];
	LineArray lineArray = MakeLineArray(lines, 8);

	for (I32 i = 0; i < RoomN; i++)
		{
			Room* room = &map->rooms[i];

			lineArray.size = 0;
			// GetRoomWalls(room, &lineArray);
			GetRoomExtendedWalls(room, PlayerSide * 0.5f, &lineArray);

			for (I32 lineI = 0; lineI < lineArray.size; lineI++)
			{
				Line line = lineArray.lines[lineI];
				CheckCollisionWithLine(line, oldCollision, newCollision);
			}
		}

		for (I32 i = 0; i < AisleN; i++)
		{
			Aisle* aisle = &map->aisles[i];

			lineArray.size = 0;
			// GetAisleWalls(aisle, &lineArray);
			GetAisleExtendedWalls(aisle, PlayerSide * 0.5f, &lineArray);

			for (I32 lineI = 0; lineI < lineArray.size; lineI++)
			{
				Line line = lineArray.lines[lineI];
				CheckCollisionWithLine(line, oldCollision, newCollision);
			}
		}
}

static void func UpdatePlayerPhysics(DungeonLabState* labState, F32 seconds)
{
	DungeonMap* map = &labState->map;

	PointCollision oldCollision = labState->playerCollision;
	oldCollision.position = labState->playerPosition;

	PointCollision newCollision = oldCollision;
	newCollision.position = oldCollision.position + seconds * labState->playerVelocity;

	if (oldCollision.collidingLineN == 0)
	{
		CheckCollisionWithAllWalls(map, &oldCollision, &newCollision);
	}
	else if (oldCollision.collidingLineN == 1)
	{
		Line line = oldCollision.collidingLine1;
		if (TurnsRight(line.p1, line.p2, newCollision.position))
		{
			newCollision.collidingLineN = 0;
		}
		else
		{
			V2 lineNormal = NormalVector(line.p2 - line.p1);
			F32 distanceFromP1 = DotProduct(lineNormal, oldCollision.position - line.p1);
			F32 lineLength = Distance(line.p1, line.p2);
			if (distanceFromP1 > lineLength || distanceFromP1 < 0.0f)
			{
				newCollision.collidingLineN = 0;
			}
			else
			{
				V2 moveVector = newCollision.position - oldCollision.position;
				F32 distanceToGo = DotProduct(lineNormal, moveVector);
				newCollision.position = oldCollision.position + distanceToGo * lineNormal;

				CheckCollisionWithAllWalls(map, &oldCollision, &newCollision);
			}
		}
	}
	else if (oldCollision.collidingLineN == 2)
	{
		Line line1 = oldCollision.collidingLine1;
		Line line2 = oldCollision.collidingLine2;
		B32 turnsRight1 = TurnsRight(line1.p1, line1.p2, newCollision.position);
		B32 turnsRight2 = TurnsRight(line2.p1, line2.p2, newCollision.position);

		if (turnsRight1 && turnsRight2)
		{
			newCollision.collidingLineN = 0;
		}
		else if (turnsRight1)
		{
			newCollision.collidingLineN = 1;
			newCollision.collidingLine1 = line2;
		}
		else if (turnsRight2)
		{
			newCollision.collidingLineN = 1;
			newCollision.collidingLine1 = line1;
		}
		else
		{
			newCollision.position = oldCollision.position;
		}

		if (newCollision.collidingLineN == 1)
		{
			Line line = newCollision.collidingLine1;
			V2 lineNormal = NormalVector(line.p2 - line.p1);
			V2 moveVector = newCollision.position - oldCollision.position;
			F32 distanceToGo = DotProduct(lineNormal, moveVector);
			newCollision.position = oldCollision.position + distanceToGo * lineNormal;
		}
	}
	else
	{
		DebugBreak();
	}

	labState->playerCollision = newCollision;
	labState->playerPosition = newCollision.position;
}

static void func UpdateEnemy(DungeonLabState* labState, DungeonLabEnemy* enemy, F32 seconds)
{
	DungeonMap* map = &labState->map;

	V2 playerPosition = labState->playerPosition;

	if (enemy->followsPlayer)
	{
		Room* playerRoom = GetRoomContainingPoint(map, playerPosition);
		Aisle* playerAisle = GetAisleContainingPoint(map, playerPosition);
		Assert(playerRoom || playerAisle);

		Room* enemyRoom = GetRoomContainingPoint(map, enemy->position);
		Aisle* enemyAisle = GetAisleContainingPoint(map, enemy->position);
		Assert(enemyRoom || enemyAisle);

		V2 targetPosition = enemy->position;
		if (enemyRoom)
		{
			if (enemyRoom == playerRoom)
			{
				targetPosition = playerPosition;
			}
			else
			{
				I32 roomSide = 0;

				F32 leftRoomRightX = enemyRoom->center.x - RoomSide * 0.5f - AisleLength;
				F32 rightRoomLeftX = enemyRoom->center.x + RoomSide * 0.5f + AisleLength;
				if (playerPosition.x < leftRoomRightX ||
					(playerAisle && playerAisle == enemyRoom->leftAisle))
				{
					roomSide = RoomSideLeft;
				}
				else if (playerPosition.x > rightRoomLeftX ||
						 (playerAisle && playerAisle == enemyRoom->rightAisle))
				{
					roomSide = RoomSideRight;
				}
				else if (playerPosition.y < enemy->position.y)
				{
					roomSide = RoomSideTop;
				}
				else
				{
					roomSide = RoomSideBottom;
				}

				targetPosition = GetOffsetRoomSideCenter(enemyRoom, roomSide, EnemySide);
				F32 distance = Distance(enemy->position, targetPosition);
				if (distance < 2.0f * EnemySide)
				{
					targetPosition = GetOffsetRoomSideCenter(enemyRoom, roomSide, -EnemySide);
				}
			}
		}
		else
		{
			Assert(enemyAisle);
			if (playerAisle == enemyAisle)
			{
				targetPosition = playerPosition;
			}
			else if (enemyAisle->isVertical)
			{
				if (playerPosition.y > enemy->position.y)
				{
					targetPosition = GetOffsetRoomAisleEntrance(enemyAisle->room2, enemyAisle, EnemySide);
				}
				else
				{
					targetPosition = GetOffsetRoomAisleEntrance(enemyAisle->room1, enemyAisle, EnemySide);
				}
			}
			else
			{
				if (playerPosition.x > enemy->position.x)
				{
					targetPosition = GetOffsetRoomAisleEntrance(enemyAisle->room2, enemyAisle, EnemySide);
				}
				else
				{
					targetPosition = GetOffsetRoomAisleEntrance(enemyAisle->room1, enemyAisle, EnemySide);
				}
			}
		}

		Canvas* canvas = &labState->canvas;
		V4 debugColor = MakeColor(0.0f, 0.0f, 1.0f);
		Bresenham(canvas, enemy->position, targetPosition, debugColor);

		F32 moveSpeed = 3.0f;
		V2 moveDirection = PointDirection(enemy->position, targetPosition);
		enemy->position = enemy->position + seconds * moveSpeed * moveDirection;
	}
	else
	{
		F32 pullDistance = 10.0f;
		if (Distance (enemy->position, playerPosition) <= pullDistance)
		{
			enemy->followsPlayer = true;
		}
	}
}

static void func DungeonLabUpdate(DungeonLabState* labState, V2 mousePosition, F32 seconds)
{
	DungeonMap* map = &labState->map;

	Canvas* canvas = &labState->canvas;
	Camera* camera = &labState->camera;

	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	UpdatePlayerPhysics(labState, seconds);

	camera->center = labState->playerPosition;

	V4 textColor = MakeColor(0.5f, 0.0f, 0.0f);

	DrawDungeonMap(canvas, map);
	DrawDungeonMapWalls(canvas, map);

	F32 playerSide = PlayerSide;
	V4 playerColor = MakeColor(1.0f, 0.0f, 0.0f);
	Rect playerRect = MakeSquareRect(labState->playerPosition, playerSide);
	DrawRect(canvas, playerRect, playerColor);

	V4 enemyColor = MakeColor(0.0f, 0.0f, 1.0f);
	for (I32 enemyI = 0; enemyI < labState->enemyN; enemyI++)
	{
		DungeonLabEnemy* enemy = &labState->enemies[enemyI];
		UpdateEnemy(labState, enemy, seconds);

		Rect enemyRect = MakeSquareRect(enemy->position, EnemySide);
		DrawRect(canvas, enemyRect, enemyColor);
	}
}

static void func DungeonLab(HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = DungeonLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "DungeonLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"DungeonLab",
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

	DungeonLabState* labState = &gDungeonLabState;

	RECT rect = {};
	GetClientRect(window, &rect);
	I32 width  = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	DungeonLabInit(labState, width, height);

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
		DungeonLabUpdate(labState, mousePosition, seconds);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		DungeonLabBlit(&labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}