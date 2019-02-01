#pragma once

#include "Lab.hpp"

#include "../Debug.hpp"
#include "../Draw.hpp"

#define RoomRowN 5
#define RoomColN 10
#define RoomN (RoomRowN * RoomColN)
#define RoomSide 20.0f

#define AisleWidth  3.0f
#define AisleLength 20.0f
#define AisleN (RoomRowN * (RoomColN - 1) + RoomColN * (RoomRowN - 1))

#define PlayerSide 1.0f
#define EnemySide 1.0f

#define MaxDungeonLabEnemyN 512

#define DungeonLabArenaSize (10 * MegaByte)

struct Room;
struct Aisle
{
	B32 isVertical;

	V2 position1;
	V2 position2;

	Room* room1;
	Room* room2;
};

struct Room
{
	V2 position;
	Aisle* leftAisle;
	Aisle* rightAisle;
	Aisle* topAisle;
	Aisle* bottomAisle;

	I32 difficulty;
};

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
	
	Room rooms[RoomN];
	Aisle aisles[AisleN];

	I8 arenaMemory[DungeonLabArenaSize];
	MemArena arena;

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

enum RoomSideDirection
{
	RoomSideTop,
	RoomSideBottom,
	RoomSideLeft,
	RoomSideRight
};

static V2 func GetRoomSideCenter(Room* room, I32 roomSide)
{
	V2 result = room->position;
	if (roomSide == RoomSideTop)
	{
		result.y -= RoomSide * 0.5f;
	}
	else if (roomSide == RoomSideBottom)
	{
		result.y += RoomSide * 0.5f;
	}
	else if (roomSide == RoomSideLeft)
	{
		result.x -= RoomSide * 0.5f;
	}
	else if (roomSide == RoomSideRight)
	{
		result.x += RoomSide * 0.5f;
	}
	return result;
}

static V2 func GetOffsetRoomSideCenter(Room* room, I32 roomSide, F32 distanceIn)
{	
	V2 result = room->position;

	F32 distanceFromCenter = RoomSide * 0.5f - distanceIn; 
	if (roomSide == RoomSideTop)
	{
		result.y -= distanceFromCenter;
	}
	else if (roomSide == RoomSideBottom)
	{
		result.y += distanceFromCenter;
	}
	else if (roomSide == RoomSideLeft)
	{
		result.x -= distanceFromCenter;
	}
	else if (roomSide == RoomSideRight)
	{
		result.x += distanceFromCenter;
	}
	else
	{
		DebugBreak();
	}

	return result;
}

static void func ConnectLeftAndRightRooms(Room* leftRoom, Room* rightRoom, Aisle* aisle)
{
	Assert(leftRoom->position.y == rightRoom->position.y);
	Assert(leftRoom->position.x < rightRoom->position.x);

	leftRoom->rightAisle = aisle;
	rightRoom->leftAisle = aisle;
	aisle->isVertical = false;
	aisle->room1 = leftRoom;
	aisle->room2 = rightRoom;
	aisle->position1 = GetRoomSideCenter(leftRoom, RoomSideRight);
	aisle->position2 = GetRoomSideCenter(rightRoom, RoomSideLeft);
}

static void func ConnectTopAndBottomRooms(Room* topRoom, Room* bottomRoom, Aisle* aisle)
{
	Assert(topRoom->position.x == bottomRoom->position.x);
	Assert(topRoom->position.y < bottomRoom->position.y);

	topRoom->bottomAisle = aisle;
	bottomRoom->topAisle = aisle;
	aisle->isVertical = true;
	aisle->room1 = topRoom;
	aisle->room2 = bottomRoom;
	aisle->position1 = GetRoomSideCenter(topRoom, RoomSideBottom);
	aisle->position2 = GetRoomSideCenter(bottomRoom, RoomSideTop);
}

struct RoomIndex
{
	I32 row;
	I32 col;
};

B32 operator==(RoomIndex index1, RoomIndex index2)
{
	B32 result = (index1.row == index2.row && index1.col == index2.col);
	return result;
}

static B32 func RoomIndexIsValid(RoomIndex roomIndex)
{
	B32 result = IsIntBetween(roomIndex.row, 0, RoomRowN - 1) && IsIntBetween(roomIndex.col, 0, RoomColN - 1);
	return result;
}

static Room* func GetRoomAtIndex(DungeonLabState* labState, RoomIndex roomIndex)
{
	Assert(RoomIndexIsValid(roomIndex));
	Room* room = &labState->rooms[roomIndex.row * RoomColN + roomIndex.col];
	return room;
}

static RoomIndex MakeRoomIndex(I32 row, I32 col)
{
	RoomIndex roomIndex = {};
	roomIndex.row = row;
	roomIndex.col = col;
	return roomIndex;
}

static Room* func GetRoomAtIndex(DungeonLabState* labState, I32 row, I32 col)
{
	RoomIndex index = {};
	index.row = row;
	index.col = col;
	Room* room = GetRoomAtIndex(labState, index);
	return room;
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

	enemy->position = room->position + roomOffset;
}

static void func DungeonLabInit(DungeonLabState* labState, I32 windowWidth, I32 windowHeight)
{
	InitRandom();

	labState->running = true;
	labState->canvas.glyphData = GetGlobalGlyphData();

	MemArena* arena = &labState->arena;
	*arena = CreateMemArena(labState->arenaMemory, DungeonLabArenaSize);

	DungeonLabResize(labState, windowWidth, windowHeight);

	F32 mapWidth  = RoomColN * RoomSide + (RoomColN - 1) * AisleLength;
	F32 mapHeight = RoomRowN * RoomSide + (RoomRowN - 1) * AisleLength;

	I32 roomIndex = 0;
	F32 roomTop = -mapHeight * 0.5f;
	for (I32 row = 0; row < RoomRowN; row++)
	{
		F32 roomLeft = -mapWidth * 0.5f;
		for (I32 col = 0; col < RoomColN; col++)
		{
			Room* room = &labState->rooms[roomIndex];
			roomIndex++;

			room->position = MakePoint(roomLeft, roomTop);

			roomLeft += (RoomSide + AisleLength);
		}

		roomTop += (RoomSide + AisleLength);
	}

	I32 aisleN = 0;
	for (I32 row = 0; row < RoomRowN; row++)
	{
		for (I32 left = 0; left < RoomColN - 1; left++)
		{
			Room* leftRoom  = GetRoomAtIndex(labState, row, left);
			Room* rightRoom = GetRoomAtIndex(labState, row, left + 1);
			ConnectLeftAndRightRooms(leftRoom, rightRoom, &labState->aisles[aisleN]);
			aisleN++;
		}
	}

	for (I32 col = 0; col < RoomColN; col++)
	{
		for (I32 top = 0; top < RoomRowN - 1; top++)
		{
			Room* topRoom    = GetRoomAtIndex(labState, top, col);
			Room* bottomRoom = GetRoomAtIndex(labState, top + 1, col);
			ConnectTopAndBottomRooms(topRoom, bottomRoom, &labState->aisles[aisleN]);
			aisleN++;
		}
	}

	RoomIndex startRoomIndex = {};
	startRoomIndex.row = IntRandom(0, RoomRowN - 1);
	startRoomIndex.col = IntRandom(0, RoomColN - 1);
	Room* startRoom = GetRoomAtIndex(labState, startRoomIndex);
	startRoom->difficulty = 0;

	labState->playerPosition = startRoom->position;
	
	RoomIndex* roomIndexQueue = ArenaPushArray(arena, RoomIndex, RoomN);
	I32 queueN = 0;
	roomIndexQueue[queueN] = startRoomIndex;
	queueN++;

	for (I32 i = 0; i < queueN; i++)
	{
		RoomIndex roomIndex = roomIndexQueue[i];
		Room* room = GetRoomAtIndex(labState, roomIndex);
		
		RoomIndex adjacentRoomIndexes[4] = {};
		adjacentRoomIndexes[0] = MakeRoomIndex(roomIndex.row, roomIndex.col - 1);
		adjacentRoomIndexes[1] = MakeRoomIndex(roomIndex.row, roomIndex.col + 1);
		adjacentRoomIndexes[2] = MakeRoomIndex(roomIndex.row - 1, roomIndex.col);
		adjacentRoomIndexes[3] = MakeRoomIndex(roomIndex.row + 1, roomIndex.col);
		for (I32 j = 0; j < 4; j++)
		{
			RoomIndex adjacentRoomIndex = adjacentRoomIndexes[j];
			if (RoomIndexIsValid(adjacentRoomIndex))
			{
				Room* adjacentRoom = GetRoomAtIndex(labState, adjacentRoomIndex);
				if (adjacentRoom->difficulty == 0 && adjacentRoom != startRoom)
				{
					adjacentRoom->difficulty = room->difficulty + 1;
					roomIndexQueue[queueN] = adjacentRoomIndex;
					queueN++;
				}
			}
		}
	}
	ArenaPopTo(arena, roomIndexQueue);

	labState->camera.unitInPixels = 50.0f;

	labState->enemyN = 0;
	for (I32 roomI = 0; roomI < RoomN; roomI++)
	{
		Room* room = &labState->rooms[roomI];
		for (I32 enemyI = 0; enemyI < room->difficulty; enemyI++)
		{
			CreateEnemyInRoom(labState, room);
		}
	}
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
	V2 center = room->position;
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
	V2 center = room->position;
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

static void func DrawAllWalls(Canvas* canvas, DungeonLabState* labState)
{
	Line walls[8];
	LineArray lines = MakeLineArray(walls, 8);

	V4 wallColor = MakeColor(1.0f, 1.0f, 1.0f);
	for (I32 i = 0; i < RoomN; i++)
	{
		Room* room = &labState->rooms[i];

		lines.size = 0;
		GetRoomWalls(room, &lines);
		DrawLines(canvas, &lines, wallColor);
	}

	for (I32 i = 0; i < AisleN; i++)
	{
		Aisle* aisle = &labState->aisles[i];

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

static void func CheckCollisionWithAllWalls(DungeonLabState* labState, PointCollision* oldCollision, PointCollision* newCollision)
{
	Line lines[8];
	LineArray lineArray = MakeLineArray(lines, 8);

	for (I32 i = 0; i < RoomN; i++)
		{
			Room* room = &labState->rooms[i];

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
			Aisle* aisle = &labState->aisles[i];

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
	PointCollision oldCollision = labState->playerCollision;
	oldCollision.position = labState->playerPosition;

	PointCollision newCollision = oldCollision;
	newCollision.position = oldCollision.position + seconds * labState->playerVelocity;

	if (oldCollision.collidingLineN == 0)
	{
		CheckCollisionWithAllWalls(labState, &oldCollision, &newCollision);
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

				CheckCollisionWithAllWalls(labState, &oldCollision, &newCollision);
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

static B32 func RoomContainsPoint(Room* room, V2 point)
{
	Rect rect = MakeSquareRect(room->position, RoomSide);
	B32 result = RectContainsPoint(rect, point);
	return result;
}

static Room* func GetRoomContainingPoint(DungeonLabState* labState, V2 point)
{
	Room* result = 0;

	for (I32 roomI = 0; roomI < RoomN; roomI++)
	{
		Room* room = &labState->rooms[roomI];
		if (RoomContainsPoint(room, point))
		{
			result = room;
			break;
		}
	}

	return result;
}

static V2 func GetAisleCenter(Aisle* aisle)
{
	Room* room1 = aisle->room1;
	Room* room2 = aisle->room2;
	Assert(room1 != 0 && room2 != 0);
	V2 center = 0.5f * (room1->position + room2->position);
	return center;
}

static B32 func AisleContainsPoint(Aisle* aisle, V2 point)
{
	V2 center = GetAisleCenter(aisle);

	Rect rect = {};
	if (aisle->isVertical)
	{
		rect = MakeRect(center, AisleWidth, AisleLength);
	}
	else
	{
		rect = MakeRect(center, AisleLength, AisleWidth);
	}

	B32 result = RectContainsPoint(rect, point);
	return result;
}

static Aisle* func GetAisleContainingPoint(DungeonLabState* labState, V2 point)
{
	Aisle* result = 0;

	for (I32 aisleI = 0; aisleI < AisleN; aisleI++)
	{
		Aisle* aisle = &labState->aisles[aisleI];
		if (AisleContainsPoint(aisle, point))
		{
			result = aisle;
			break;
		}
	}

	return result;
}

static Room* GetNextRoomOnSide(Room* room, I32 roomSide)
{
	Room* nextRoom = 0;

	if (roomSide == RoomSideLeft && room->leftAisle != 0)
	{
		nextRoom = room->leftAisle->room1;
	}
	else if (roomSide == RoomSideRight && room->rightAisle != 0)
	{
		nextRoom = room->rightAisle->room2;
	}
	else if (roomSide == RoomSideTop && room->topAisle != 0)
	{
		nextRoom = room->topAisle->room1;
	}
	else if (roomSide == RoomSideBottom && room->bottomAisle != 0)
	{
		nextRoom = room->bottomAisle->room2;
	}

	return nextRoom;
}

static V2 func GetOffsetRoomAisleEntrance(Room* room, Aisle* aisle, F32 distanceIn)
{
	Assert(room != 0 && aisle != 0);

	V2 entrance = {};
	if (room->leftAisle == aisle)
	{
		entrance = GetOffsetRoomSideCenter(room, RoomSideLeft, distanceIn);
	}
	else if (room->rightAisle == aisle)
	{
		entrance = GetOffsetRoomSideCenter(room, RoomSideRight, distanceIn);
	}
	else if (room->topAisle == aisle)
	{
		entrance = GetOffsetRoomSideCenter(room, RoomSideTop, distanceIn);
	}
	else if (room->bottomAisle == aisle)
	{
		entrance = GetOffsetRoomSideCenter(room, RoomSideBottom, distanceIn);
	}
	else
	{
		DebugBreak();
	}

	return entrance;
}

static void func UpdateEnemy(DungeonLabState* labState, DungeonLabEnemy* enemy, F32 seconds)
{
	V2 playerPosition = labState->playerPosition;

	if (enemy->followsPlayer)
	{
		Room* playerRoom = GetRoomContainingPoint(labState, playerPosition);
		Aisle* playerAisle = GetAisleContainingPoint(labState, playerPosition);
		Assert(playerRoom || playerAisle);

		Room* enemyRoom = GetRoomContainingPoint(labState, enemy->position);
		Aisle* enemyAisle = GetAisleContainingPoint(labState, enemy->position);
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

				F32 leftRoomRightX = enemyRoom->position.x - RoomSide * 0.5f - AisleLength;
				F32 rightRoomLeftX = enemyRoom->position.x + RoomSide * 0.5f + AisleLength;
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
	Canvas* canvas = &labState->canvas;
	Camera* camera = &labState->camera;

	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	UpdatePlayerPhysics(labState, seconds);

	camera->center = labState->playerPosition;

	V4 roomColor = MakeColor(0.5f, 0.5f, 0.5f);
	V4 textColor = MakeColor(0.5f, 0.0f, 0.0f);
	for (I32 i = 0; i < RoomN; i++)
	{
		Room* room = &labState->rooms[i];
		F32 left   = room->position.x - RoomSide * 0.5f;
		F32 right  = room->position.x + RoomSide * 0.5f;
		F32 top    = room->position.y - RoomSide * 0.5f;
		F32 bottom = room->position.y + RoomSide * 0.5f;
		DrawRectLRTB(canvas, left, right, top, bottom, roomColor);

		/*
		I8 textBuffer[8];
		String text = StartString(textBuffer, 8);
		AddInt(&text, room->difficulty);
		DrawTextLineXCentered(canvas, textBuffer, room->position.y, room->position.x, textColor);
		*/
	}

	V4 aisleColor = roomColor;
	for (I32 i = 0; i < AisleN; i++)
	{
		Aisle* aisle = &labState->aisles[i];

		F32 x1 = aisle->room1->position.x;
		F32 x2 = aisle->room2->position.x;
		F32 y1 = aisle->room1->position.y;
		F32 y2 = aisle->room2->position.y;

		if (aisle->isVertical)
		{
			Assert(x1 == x2);
			Assert(y1 < y2);
			F32 left   = x1 - AisleWidth * 0.5f;
			F32 right  = x1 + AisleWidth * 0.5f;
			F32 top    = y1 + RoomSide * 0.5f;
			F32 bottom = y2 - RoomSide * 0.5f;
			DrawRectLRTB(canvas, left, right, top, bottom, aisleColor);
		}
		else
		{
			Assert(y1 == y2);
			Assert(x1 < x2);
			F32 left   = x1 + RoomSide * 0.5f;
			F32 right  = x2 - RoomSide * 0.5f;
			F32 top    = y1 - AisleWidth * 0.5f;
			F32 bottom = y1 + AisleWidth * 0.5f;
			DrawRectLRTB(canvas, left, right, top, bottom, aisleColor);
		}
	}

	DrawAllWalls(canvas, labState);

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