#pragma once

#include "Debug.hpp"

#define RoomRowN 5
#define RoomColN 10
#define RoomN (RoomRowN * RoomColN)
#define RoomSide 20.0f

#define AisleWidth  3.0f
#define AisleLength 20.0f
#define AisleN (RoomRowN * (RoomColN - 1) + RoomColN * (RoomRowN - 1))

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
	V2 center;
	Aisle* leftAisle;
	Aisle* rightAisle;
	Aisle* topAisle;
	Aisle* bottomAisle;
};

struct DungeonMap
{
	Room rooms[RoomN];
	Aisle aisles[AisleN];
};

enum RoomSideDirection
{
	RoomSideTop,
	RoomSideBottom,
	RoomSideLeft,
	RoomSideRight
};

static V2 func GetRoomSideCenter (Room* room, I32 roomSide)
{
	V2 result = room->center;
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

static V2 func GetOffsetRoomSideCenter (Room* room, I32 roomSide, F32 distanceIn)
{	
	V2 result = room->center;

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
		DebugBreak ();
	}

	return result;
}

static void func ConnectLeftAndRightRooms (Room* leftRoom, Room* rightRoom, Aisle* aisle)
{
	Assert (leftRoom->center.y == rightRoom->center.y);
	Assert (leftRoom->center.x < rightRoom->center.x);

	leftRoom->rightAisle = aisle;
	rightRoom->leftAisle = aisle;
	aisle->isVertical = false;
	aisle->room1 = leftRoom;
	aisle->room2 = rightRoom;
	aisle->position1 = GetRoomSideCenter (leftRoom, RoomSideRight);
	aisle->position2 = GetRoomSideCenter (rightRoom, RoomSideLeft);
}

static void func ConnectTopAndBottomRooms (Room* topRoom, Room* bottomRoom, Aisle* aisle)
{
	Assert (topRoom->center.x == bottomRoom->center.x);
	Assert (topRoom->center.y < bottomRoom->center.y);

	topRoom->bottomAisle = aisle;
	bottomRoom->topAisle = aisle;
	aisle->isVertical = true;
	aisle->room1 = topRoom;
	aisle->room2 = bottomRoom;
	aisle->position1 = GetRoomSideCenter (topRoom, RoomSideBottom);
	aisle->position2 = GetRoomSideCenter (bottomRoom, RoomSideTop);
}

struct RoomIndex
{
	I32 row;
	I32 col;
};

B32 operator== (RoomIndex index1, RoomIndex index2)
{
	B32 result = (index1.row == index2.row && index1.col == index2.col);
	return result;
}

static B32 func RoomIndexIsValid (RoomIndex roomIndex)
{
	B32 result = IsIntBetween (roomIndex.row, 0, RoomRowN - 1) && IsIntBetween (roomIndex.col, 0, RoomColN - 1);
	return result;
}

static RoomIndex func GetRandomRoomIndex (DungeonMap* map)
{
	RoomIndex index = {};
	index.row = IntRandom (0, RoomRowN - 1);
	index.col = IntRandom (0, RoomColN - 1);
	Assert (RoomIndexIsValid (index));
	return index;
}

static Room* func GetRoomAtIndex (DungeonMap* map, RoomIndex roomIndex)
{
	Assert (RoomIndexIsValid (roomIndex));
	Room* room = &map->rooms[roomIndex.row * RoomColN + roomIndex.col];
	return room;
}

static RoomIndex MakeRoomIndex (I32 row, I32 col)
{
	RoomIndex roomIndex = {};
	roomIndex.row = row;
	roomIndex.col = col;
	return roomIndex;
}

static Room* func GetRoomAtIndex (DungeonMap* map, I32 row, I32 col)
{
	RoomIndex index = {};
	index.row = row;
	index.col = col;
	Room* room = GetRoomAtIndex (map, index);
	return room;
}

static void func InitDungeonMap (DungeonMap* map)
{
	F32 mapWidth  = RoomColN * RoomSide + (RoomColN - 1) * AisleLength;
	F32 mapHeight = RoomRowN * RoomSide + (RoomRowN - 1) * AisleLength;

	I32 roomIndex = 0;
	F32 roomTop = -mapHeight * 0.5f;
	for (I32 row = 0; row < RoomRowN; row++)
	{
		F32 roomLeft = -mapWidth * 0.5f;
		for (I32 col = 0; col < RoomColN; col++)
		{
			Room* room = &map->rooms[roomIndex];
			roomIndex++;

			room->center = MakePoint (roomLeft, roomTop);

			roomLeft += (RoomSide + AisleLength);
		}

		roomTop += (RoomSide + AisleLength);
	}

	I32 aisleN = 0;
	for (I32 row = 0; row < RoomRowN; row++)
	{
		for (I32 left = 0; left < RoomColN - 1; left++)
		{
			Room* leftRoom  = GetRoomAtIndex (map, row, left);
			Room* rightRoom = GetRoomAtIndex (map, row, left + 1);
			ConnectLeftAndRightRooms (leftRoom, rightRoom, &map->aisles[aisleN]);
			aisleN++;
		}
	}

	for (I32 col = 0; col < RoomColN; col++)
	{
		for (I32 top = 0; top < RoomRowN - 1; top++)
		{
			Room* topRoom    = GetRoomAtIndex (map, top, col);
			Room* bottomRoom = GetRoomAtIndex (map, top + 1, col);
			ConnectTopAndBottomRooms (topRoom, bottomRoom, &map->aisles[aisleN]);
			aisleN++;
		}
	}
}


static B32 func RoomContainsPoint (Room* room, V2 point)
{
	Rect rect = MakeSquareRect (room->center, RoomSide);
	B32 result = RectContainsPoint (rect, point);
	return result;
}

static Room* func GetRoomContainingPoint (DungeonMap* map, V2 point)
{
	Room* result = 0;

	for (I32 roomI = 0; roomI < RoomN; roomI++)
	{
		Room* room = &map->rooms[roomI];
		if (RoomContainsPoint (room, point))
		{
			result = room;
			break;
		}
	}

	return result;
}

static V2 func GetAisleCenter (Aisle* aisle)
{
	Room* room1 = aisle->room1;
	Room* room2 = aisle->room2;
	Assert (room1 != 0 && room2 != 0);
	V2 center = 0.5f * (room1->center + room2->center);
	return center;
}

static B32 func AisleContainsPoint (Aisle* aisle, V2 point)
{
	V2 center = GetAisleCenter (aisle);

	Rect rect = {};
	if (aisle->isVertical)
	{
		rect = MakeRect (center, AisleWidth, AisleLength);
	}
	else
	{
		rect = MakeRect (center, AisleLength, AisleWidth);
	}

	B32 result = RectContainsPoint (rect, point);
	return result;
}

static Aisle* func GetAisleContainingPoint (DungeonMap* map, V2 point)
{
	Aisle* result = 0;

	for (I32 aisleI = 0; aisleI < AisleN; aisleI++)
	{
		Aisle* aisle = &map->aisles[aisleI];
		if (AisleContainsPoint (aisle, point))
		{
			result = aisle;
			break;
		}
	}

	return result;
}

static Room* GetNextRoomOnSide (Room* room, I32 roomSide)
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

static V2 func GetOffsetRoomAisleEntrance (Room* room, Aisle* aisle, F32 distanceIn)
{
	Assert (room != 0 && aisle != 0);

	V2 entrance = {};
	if (room->leftAisle == aisle)
	{
		entrance = GetOffsetRoomSideCenter (room, RoomSideLeft, distanceIn);
	}
	else if (room->rightAisle == aisle)
	{
		entrance = GetOffsetRoomSideCenter (room, RoomSideRight, distanceIn);
	}
	else if (room->topAisle == aisle)
	{
		entrance = GetOffsetRoomSideCenter (room, RoomSideTop, distanceIn);
	}
	else if (room->bottomAisle == aisle)
	{
		entrance = GetOffsetRoomSideCenter (room, RoomSideBottom, distanceIn);
	}
	else
	{
		DebugBreak ();
	}

	return entrance;
}

static void func DrawRoom (Canvas* canvas, Room* room)
{
	V4 color = MakeColor (0.5f, 0.5f, 0.5f);
	F32 left   = room->center.x - RoomSide * 0.5f;
	F32 right  = room->center.x + RoomSide * 0.5f;
	F32 top    = room->center.y - RoomSide * 0.5f;
	F32 bottom = room->center.y + RoomSide * 0.5f;
	DrawRectLRTB (canvas, left, right, top, bottom, color);
}

static void func DrawAisle (Canvas* canvas, Aisle* aisle)
{
	V4 color = MakeColor (0.5f, 0.5f, 0.5f);
	
		F32 x1 = aisle->room1->center.x;
		F32 x2 = aisle->room2->center.x;
		F32 y1 = aisle->room1->center.y;
		F32 y2 = aisle->room2->center.y;

		if (aisle->isVertical)
		{
			Assert (x1 == x2);
			Assert (y1 < y2);
			F32 left   = x1 - AisleWidth * 0.5f;
			F32 right  = x1 + AisleWidth * 0.5f;
			F32 top    = y1 + RoomSide * 0.5f;
			F32 bottom = y2 - RoomSide * 0.5f;
			DrawRectLRTB (canvas, left, right, top, bottom, color);
		}
		else
		{
			Assert (y1 == y2);
			Assert (x1 < x2);
			F32 left   = x1 + RoomSide * 0.5f;
			F32 right  = x2 - RoomSide * 0.5f;
			F32 top    = y1 - AisleWidth * 0.5f;
			F32 bottom = y1 + AisleWidth * 0.5f;
			DrawRectLRTB (canvas, left, right, top, bottom, color);
		}
}

static void func DrawDungeonMap (Canvas* canvas, DungeonMap* map)
{
	for (I32 i = 0; i < RoomN; i++)
	{
		Room* room = &map->rooms[i];
		DrawRoom (canvas, room);
	}
	
	for (I32 i = 0; i < AisleN; i++)
	{
		Aisle* aisle = &map->aisles[i];
		DrawAisle (canvas, aisle);
	}
}