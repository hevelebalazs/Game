#pragma once

#include "BuildingType.hpp"
#include "Draw.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Road.hpp"
#include "Type.hpp"

#define ConnectRoadWidth	5.0f
#define EntranceWidth		3.0f
#define WallWidth			0.5f

#define DoorWidth			3.0f
#define MinRoomSide			5.0f
#define MaxRoomSide			20.0f

enum CrossType 
{
	CrossNone,
	CrossWall,
	CrossEntrance
};

struct Building;
struct BuildingCrossInfo 
{
	Building* building;
	V2 crossPoint;
	V2 corner1;
	V2 corner2;
	CrossType type;
};

struct BuildingInside 
{
	I32 wallCount;
	Line* walls;
};

struct Building 
{
	// TODO: remove this
	BuildingType type;

	// TODO: save four corner points instead
	F32 top;
	F32 left;
	F32 bottom;
	F32 right;

	F32 height;

	B32 roadAround;

	V2 connectPointClose;
	V2 connectPointFarShow;
	V2 connectPointFar;

	V2 entrancePoint1;
	V2 entrancePoint2;

	I32 connectTreeHeight;

	Road* connectRoad;

	BuildingInside* inside;
};

static Line ConnectingLine(V2 point1, V2 point2)
{
	Line result = {};
	result.p1 = point1;
	result.p2 = point2;
	return result;
}

static Line HorizontalWall(F32 left, F32 right, F32 y)
{
	Line wall = {};
	wall.x1 = left;
	wall.y1 = y;
	wall.x2 = right;
	wall.y2 = y;
	return wall;
}

static Line VerticalWall(F32 top, F32 bottom, F32 x)
{
	Line wall = {};
	wall.x1 = x;
	wall.y1 = top;
	wall.x2 = x;
	wall.y2 = bottom;
	return wall;
}

struct WallHelper
{
	I32 maxWallCount;
	I32 wallCount;
	Line* walls;
	B32* hasDoor;
	Line* doors;
};

static void AddHelperWall(WallHelper* helper, Line wall)
{
	if (helper->wallCount < helper->maxWallCount) 
	{
		helper->walls[helper->wallCount] = wall;
		helper->hasDoor[helper->wallCount] = false;
		helper->wallCount++;
	}
}

static void AddHelperWallWithDoor(WallHelper* helper, Line wall, Line door)
{
	if (helper->wallCount < helper->maxWallCount)
	{
		helper->walls[helper->wallCount] = wall;
		helper->hasDoor[helper->wallCount] = true;
		helper->doors[helper->wallCount] = door;
		helper->wallCount++;
	}
}

// TODO: rewrite this not using recursion
static void GenerateWalls(Building* building, WallHelper* wallHelper,
						  I32 leftWallIndex, I32 rightWallIndex, I32 topWallIndex, I32 bottomWallIndex,
						  F32 minRoomSide, F32 maxRoomSide) 
{
	F32 left   = wallHelper->walls[leftWallIndex].x1;
	F32 right  = wallHelper->walls[rightWallIndex].x1;
	F32 top    = wallHelper->walls[topWallIndex].y1;
	F32 bottom = wallHelper->walls[bottomWallIndex].y1;

	F32 width = (right - left);
	F32 height = (bottom - top);

	F32 cutDistance = RandomBetween(minRoomSide, maxRoomSide);

	B32 canCutHorizontally = (cutDistance <= height - minRoomSide);
	F32 cutY = top + cutDistance;
	{
		I32 doorCount = 0;
		Line doors[2] = {};

		if (wallHelper->hasDoor[leftWallIndex]) 
		{
			doors[doorCount] = wallHelper->doors[leftWallIndex];
			doors[doorCount].y1 -= WallWidth * 0.5f;
			doors[doorCount].y2 += WallWidth * 0.5f;
			doorCount++;
		}
		if (wallHelper->hasDoor[rightWallIndex]) 
		{
			doors[doorCount] = wallHelper->doors[rightWallIndex];
			doors[doorCount].y1 -= WallWidth * 0.5f;
			doors[doorCount].y2 += WallWidth * 0.5f;
			doorCount++;
		}

		if (doorCount == 2 && (doors[0].y1 < doors[1].y2 && doors[1].y1 < doors[0].y2)) 
		{
			Line door = {};
			door.y1 = Min2(doors[0].y1, doors[1].y1);
			door.y2 = Max2(doors[0].y2, doors[1].y2);

			doorCount = 1;
			doors[0] = door;
		}

		if (doorCount > 0 && (doors[0].y1 < cutY && cutY < doors[0].y2)) 
		{
			F32 topY = doors[0].y1;
			F32 topDist1 = topY - top;
			F32 topDist2 = bottom - topY;

			F32 bottomY = doors[0].y2;
			F32 bottomDist1 = bottomY - top;
			F32 bottomDist2 = bottom - bottomY;

			if (topDist1 >= minRoomSide && topDist2 >= minRoomSide) 
			{
				cutY = topY;
				canCutHorizontally = true;
			} 
			else if (bottomDist1 >= minRoomSide && bottomDist2 >= minRoomSide) 
			{
				cutY = bottomY;
				canCutHorizontally = true;
			} 
			else
			{
				canCutHorizontally = false;
			}
		}

		if (doorCount > 1 && (doors[1].y1 < cutY && cutY < doors[1].y2)) 
		{
			F32 topY = doors[1].y1;
			F32 topDist1 = topY - top;
			F32 topDist2 = bottom - topY;

			F32 bottomY = doors[1].y2;
			F32 bottomDist1 = bottomY - top;
			F32 bottomDist2 = bottom - bottomY;

			if (topDist1 >= minRoomSide && topDist2 >= minRoomSide) 
			{
				cutY = topY;
				canCutHorizontally = true;
			} 
			else if (bottomDist1 >= minRoomSide && bottomDist2 >= minRoomSide) 
			{
				cutY = bottomY;
				canCutHorizontally = true;
			} 
			else 
			{
				canCutHorizontally = false;
			}
		}
	}

	B32 canCutVertically = (cutDistance <= width - minRoomSide);
	F32 cutX = left + cutDistance;
	{
		I32 doorCount = 0;
		Line doors[2] = {};

		if (wallHelper->hasDoor[topWallIndex]) 
		{
			doors[doorCount] = wallHelper->doors[topWallIndex];
			doors[doorCount].x1 -= WallWidth * 0.5f;
			doors[doorCount].x2 += WallWidth * 0.5f;
			doorCount++;
		}

		if (wallHelper->hasDoor[bottomWallIndex]) 
		{
			doors[doorCount] = wallHelper->doors[bottomWallIndex];
			doors[doorCount].x1 -= WallWidth * 0.5f;
			doors[doorCount].x2 += WallWidth * 0.5f;
			doorCount++;
		}

		if (doorCount == 2 && (doors[0].x1 < doors[1].x2 && doors[1].x1 < doors[0].x2)) 
		{
			Line door = {};
			door.x1 = Min2(doors[0].x1, doors[1].x1);
			door.x2 = Max2(doors[0].x2, doors[1].x2);

			doorCount = 1;
			doors[0] = door;
		}

		if (doorCount > 0 && (doors[0].x1 < cutX && cutX < doors[0].x2)) 
		{
			F32 leftX = doors[0].x1;
			F32 leftDist1 = leftX - left;
			F32 leftDist2 = right - leftX;

			F32 rightX = doors[0].x2;
			F32 rightDist1 = rightX - left;
			F32 rightDist2 = right - rightX;

			if (leftDist1 >= minRoomSide && leftDist2 >= minRoomSide) 
			{
				cutX = leftX;
				canCutVertically = true;
			} 
			else if (rightDist1 >= minRoomSide && rightDist2 >= minRoomSide) 
			{
				cutX = rightX;
				canCutVertically = true;
			} 
			else 
			{
				canCutVertically = false;
			}
		}

		if (doorCount > 1 && (doors[1].x1 < cutX && cutX < doors[1].x2)) 
		{
			F32 leftX = doors[1].x1;
			F32 leftDist1 = leftX - left;
			F32 leftDist2 = right - leftX;

			F32 rightX = doors[1].x2;
			F32 rightDist1 = rightX - left;
			F32 rightDist2 = right - rightX;

			if (leftDist1 >= minRoomSide && leftDist2 >= minRoomSide) 
			{
				cutX = leftX;
				canCutVertically = true;
			} 
			else if (rightDist1 >= minRoomSide && rightDist2 >= minRoomSide) 
			{
				cutX = rightX;
				canCutVertically = true;
			} 
			else 
			{
				canCutVertically = false;
			}
		}
	}

	B32 cutHorizontally = false;
	B32 cutVertically = false;

	if (canCutHorizontally && canCutVertically) 
	{
		F32 random = RandomBetween(0.0f, 1.0f);

		if (random < 0.5f)
		{
			cutHorizontally = true;
		}
		else 
		{
			cutVertically = true;
		}
	} 
	else if (canCutHorizontally) 
	{
		cutHorizontally = true;
	} 
	else if (canCutVertically) 
	{
		cutVertically = true;
	}

	BuildingInside* inside = building->inside;
	
	if (cutVertically) 
	{
		I32 wallIndex = wallHelper->wallCount;

		Line wall = VerticalWall(top, bottom, cutX);

		F32 centerY = RandomBetween(
			Min2(wall.y1, wall.y2) + DoorWidth * 0.5f,
			Max2(wall.y1, wall.y2) - DoorWidth * 0.5f
		);

		Line door = {};
		door.x1 = wall.x1;
		door.y1 = centerY - DoorWidth * 0.5f;
		door.x2 = wall.x2;
		door.y2 = centerY + DoorWidth * 0.5f;

		AddHelperWallWithDoor(wallHelper, wall, door);

		GenerateWalls(building, wallHelper,     wallIndex, rightWallIndex, topWallIndex, bottomWallIndex, minRoomSide, maxRoomSide);
		GenerateWalls(building, wallHelper, leftWallIndex,      wallIndex, topWallIndex, bottomWallIndex, minRoomSide, maxRoomSide);
	}
	else if (cutHorizontally) 
	{
		I32 wallIndex = wallHelper->wallCount;

		Line wall = HorizontalWall(left, right, cutY);

		F32 centerX = RandomBetween(
			Min2(wall.x1, wall.x2) + DoorWidth * 0.5f,
			Max2(wall.x1, wall.x2) - DoorWidth * 0.5f
		);

		Line door = {};
		door.x1 = centerX - DoorWidth * 0.5f;
		door.y1 = wall.y1;
		door.x2 = centerX + DoorWidth * 0.5f;
		door.y2 = wall.y2;
		AddHelperWallWithDoor(wallHelper, wall, door);

		GenerateWalls(building, wallHelper, leftWallIndex, rightWallIndex,    wallIndex, bottomWallIndex, minRoomSide, maxRoomSide);
		GenerateWalls(building, wallHelper, leftWallIndex, rightWallIndex, topWallIndex,       wallIndex, minRoomSide, maxRoomSide);
	}
}

static void GenerateBuildingInside(Building* building, MemArena* arena, MemArena* tmpArena)
{
	BuildingInside* inside = ArenaPushType(arena, BuildingInside);
	building->inside = inside;

	WallHelper wallHelper = {};
	wallHelper.maxWallCount = 100;
	wallHelper.wallCount = 0;
	wallHelper.walls = ArenaPushArray(tmpArena, Line, wallHelper.maxWallCount);
	wallHelper.hasDoor = ArenaPushArray(tmpArena, B32, wallHelper.maxWallCount);
	wallHelper.doors = ArenaPushArray(tmpArena, Line, wallHelper.maxWallCount);

	F32 buildingWidth = (building->right - building->left);
	F32 buildingHeight = (building->bottom - building->top);

	I32 maxWallCount = 4 + ((I32)buildingWidth / (I32)MinRoomSide) * ((I32)buildingHeight / (I32)MinRoomSide);

	F32 halfWallWidth = WallWidth * 0.5f;

	Line entrance = {};
	entrance.p1 = building->entrancePoint1;
	entrance.p2 = building->entrancePoint2;

	Line leftWall = VerticalWall(building->top + halfWallWidth, building->bottom - halfWallWidth, building->left + halfWallWidth);
	if (entrance.x1 == building->left && entrance.x2 == building->left)
	{
		AddHelperWallWithDoor(&wallHelper, leftWall, entrance);
	}
	else
	{
		AddHelperWall(&wallHelper, leftWall);
	}

	Line rightWall = VerticalWall(building->top + halfWallWidth, building->bottom - halfWallWidth, building->right - halfWallWidth);
	if (entrance.x1 == building->right && entrance.x2 == building->right)
	{
		AddHelperWallWithDoor(&wallHelper, rightWall, entrance);
	}
	else
	{
		AddHelperWall(&wallHelper, rightWall);
	}

	Line topWall = HorizontalWall(building->left, building->right, building->top + halfWallWidth);
	if (entrance.y1 == building->top && entrance.y2 == building->top)
	{
		AddHelperWallWithDoor(&wallHelper, topWall, entrance);
	}
	else
	{
		AddHelperWall(&wallHelper, topWall);
	}

	Line bottomWall = HorizontalWall(building->left, building->right, building->bottom - halfWallWidth);
	if (entrance.y1 == building->bottom && entrance.y2 == building->bottom)
	{
		AddHelperWallWithDoor(&wallHelper, bottomWall, entrance);
	}
	else
	{
		AddHelperWall(&wallHelper, bottomWall);
	}

	GenerateWalls(building, &wallHelper, 0, 1, 2, 3, MinRoomSide, MaxRoomSide);

	I32 wallCount = 0;
	for (I32 i = 0; i < wallHelper.wallCount; ++i) 
	{
		if (wallHelper.hasDoor[i]) 
		{
			wallCount += 2;
		}
		else 
		{
			wallCount += 1;
		}
	}

	inside->wallCount = 0;
	inside->walls = ArenaPushArray(arena, Line, wallCount);

	for (I32 i = 0; i < wallHelper.wallCount; ++i) 
	{
		Line wall = wallHelper.walls[i];

		if (wallHelper.hasDoor[i]) 
		{
			Line door = wallHelper.doors[i];

			if (wall.x1 == wall.x2) 
			{
				Line topWall = VerticalWall(
					Min2(wall.y1, wall.y2), 
					Min2(door.y1, door.y2), 
					wall.x1
				);
				Line bottomWall = VerticalWall(
					Max2(door.y1, door.y2),
					Max2(wall.y1, wall.y2),
					wall.x1
				);

				inside->walls[inside->wallCount] = topWall;
				inside->wallCount++;
				inside->walls[inside->wallCount] = bottomWall;
				inside->wallCount++;
			} 
			else if (wall.y1 == wall.y2) 
			{
				Line leftWall = HorizontalWall(
					Min2(wall.x1, wall.x2),
					Min2(door.x1, door.x2),
					wall.y1
				);
				Line rightWall = HorizontalWall(
					Max2(door.x1, door.x2),
					Max2(wall.x1, wall.x2),
					wall.y1
				);

				inside->walls[inside->wallCount] = leftWall;
				inside->wallCount++;
				inside->walls[inside->wallCount] = rightWall;
				inside->wallCount++;
			}
		}
		else 
		{
			inside->walls[inside->wallCount] = wall;
			inside->wallCount++;
		}
	}

	ArenaPopTo(tmpArena, wallHelper.walls);
}

static void ConnectBuildingToRoad(Building* building, Road* road)
{
	// TODO: Update this in a Building project!
}

static B32 IsPointInBuilding(V2 point, Building building)
{
	if (point.x < building.left || point.x > building.right)
	{
		return false;
	}

	if (point.y < building.top || point.y > building.bottom) 
	{
		return false;
	}

	return true;
}

static B32 IsPointInExtBuilding(V2 point, Building building, F32 radius)
{
	if (point.x < building.left - radius || point.x > building.right + radius) 
	{
		return false;
	}

	if (point.y < building.top - radius || point.y > building.bottom + radius) 
	{
		return false;
	}

	return true;
}

static B32 IsPointOnEdge(V2 point, Building building)
{
	if (point.x == building.left)
	{
		return true;
	}

	if (point.x == building.right) 
	{
		return true;
	}

	if (point.y == building.top) 
	{
		return true;
	}

	if (point.y == building.bottom) 
	{
		return true;
	}

	return false;
}

// TODO: can this be merged with ClosestCrossPoint?
static B32 IsBuildingCrossed(Building building, V2 point1, V2 point2)
{
	if (IsPointOnEdge(point1, building))
	{
		return true;
	}

	if (IsPointOnEdge(point2, building)) 
	{
		return true;
	}

	V2 topLeft     = {building.left,  building.top};
	V2 topRight    = {building.right, building.top};
	V2 bottomLeft  = {building.left,  building.bottom};
	V2 bottomRight = {building.right, building.bottom};

	if (DoLinesCross(topLeft, topRight, point1, point2)) 
	{
		return true;
	}

	if (DoLinesCross(topRight, bottomRight, point1, point2)) 
	{
		return true;
	}

	if (DoLinesCross(bottomRight, bottomLeft, point1, point2)) 
	{
		return true;
	}

	if (DoLinesCross(bottomLeft, topLeft, point1, point2)) 
	{
		return true;
	}

	return false;
}

static BuildingCrossInfo ExtBuildingClosestCrossInfo(Building* building, F32 radius, V2 closePoint, V2 farPoint)
{
	BuildingCrossInfo result = {};
	result.type = CrossNone;
	
	F32 minDistanceSquare = 0.0f;
	B32 foundAny = false;

	V2 topLeft =     {building->left  - radius, building->top    - radius};
	V2 topRight =    {building->right + radius, building->top    - radius};
	V2 bottomLeft =  {building->left  - radius, building->bottom + radius};
	V2 bottomRight = {building->right + radius, building->bottom + radius};
	V2 points[5] = {topLeft, topRight, bottomRight, bottomLeft, topLeft};

	for (I32 i = 0; i < 4; ++i) 
	{
		V2 corner1 = points[i];
		V2 corner2 = points[i + 1];

		if (DoLinesCross(corner1, corner2, closePoint, farPoint)) 
		{
			V2 intersection = LineIntersection(corner1, corner2, closePoint, farPoint);
			F32 distanceSquare = DistanceSquare(closePoint, intersection);

			if (foundAny == false || distanceSquare < minDistanceSquare) 
			{
				minDistanceSquare = distanceSquare;
				foundAny = true;

				result.building = building;
				result.crossPoint = intersection;
				result.corner1 = corner1;
				result.corner2 = corner2;
				result.type = CrossWall;
			}
		}
	}

	// TODO: create directed points for entrance points that point outside of the building
	V2 entrance1 = building->entrancePoint1;
	V2 entrance2 = building->entrancePoint2;

	if (entrance1.x == building->left && entrance2.x == building->left) 
	{
		entrance1.x -= radius;
		entrance2.x -= radius;
	} 
	else if (entrance1.x == building->right && entrance2.x == building->right) 
	{
		entrance1.x += radius;
		entrance2.x += radius;
	} 
	else if (entrance1.y == building->top && entrance2.y == building->top) 
	{
		entrance1.y -= radius;
		entrance2.y -= radius;
	} 
	else if (entrance2.y == building->bottom && entrance2.y == building->bottom) 
	{
		entrance1.y += radius;
		entrance2.y += radius;
	}

	if (entrance1.x < entrance2.x) 
	{
		entrance1.x += radius;
		entrance2.x -= radius;
	} 
	else if (entrance1.x > entrance2.x) 
	{
		entrance1.x -= radius;
		entrance2.x += radius;
	}

	if (entrance1.y < entrance2.y) 
	{
		entrance1.y += radius;
		entrance2.y -= radius;
	} 
	else if (entrance1.y > entrance2.y) 
	{
		entrance1.y -= radius;
		entrance2.y += radius;
	}

	if (DoLinesCross(entrance1, entrance2, closePoint, farPoint)) 
	{
		V2 intersection = LineIntersection(entrance1, entrance2, closePoint, farPoint);
		
		result.building = building;
		result.crossPoint = intersection;
		result.corner1 = building->entrancePoint1;
		result.corner2 = building->entrancePoint2;
		result.type = CrossEntrance;
	}

	return result;
}

static BuildingCrossInfo ExtBuildingInsideClosestCrossInfo(Building* building, F32 radius, V2 closePoint, V2 farPoint)
{
	BuildingCrossInfo result = {};
	result.type = CrossNone;

	F32 minDistanceSquare = 0.0f;
	B32 foundAny = false;

	BuildingInside* inside = building->inside;
	for (I32 i = 0; i < inside->wallCount; ++i) 
	{
		Line wall = inside->walls[i];

		V2 endPoint1 = wall.p1;
		V2 endPoint2 = wall.p2;

		if (endPoint1.x < endPoint2.x) 
		{
			endPoint1.x -= radius;
			endPoint2.x += radius;
		} 
		else if (endPoint1.x > endPoint2.x) 
		{
			endPoint1.x += radius;
			endPoint2.x -= radius;
		}

		if (endPoint1.y < endPoint2.y) 
		{
			endPoint1.y -= radius;
			endPoint2.y += radius;
		} 
		else if (endPoint1.y > endPoint2.y) 
		{
			endPoint1.y += radius;
			endPoint2.y -= radius;
		}

		V2 add = {};
		if (endPoint1.x == endPoint2.x) 
		{
			add = {1.0f, 0.0f};
		}
		else 
		{
			add = {0.0f, 1.0f};
		}

		F32 wallRadius = radius + (WallWidth * 0.5f);

		V2 point1 = endPoint1 + (wallRadius * add);
		V2 point2 = endPoint1 - (wallRadius * add);
		V2 point3 = endPoint2 - (wallRadius * add);
		V2 point4 = endPoint2 + (wallRadius * add);

		V2 points[5] = {point1, point2, point3, point4, point1};

		for (I32 i = 0; i < 4; ++i) 
		{
			V2 corner1 = points[i];
			V2 corner2 = points[i + 1];

			if (DoLinesCross(corner1, corner2, closePoint, farPoint)) 
			{
				V2 intersection = LineIntersection(corner1, corner2, closePoint, farPoint);
				F32 distanceSquare = DistanceSquare(closePoint, intersection);

				if (foundAny == false || distanceSquare < minDistanceSquare) 
				{
					minDistanceSquare = distanceSquare;
					foundAny = true;

					result.building = building;
					result.crossPoint = intersection;
					result.corner1 = corner1;
					result.corner2 = corner2;
					result.type = CrossWall;
				}
			}
		}
	}

	return result;
}

static V2 ClosestBuildingCrossPoint(Building building, V2 closePoint, V2 farPoint)
{
	V2 result = {};
	F32 minDistanceSquare = 0.0f;
	B32 foundAny = false;

	V2 topLeft = {building.left,  building.top};
	V2 topRight = {building.right, building.top};
	V2 bottomLeft = {building.left,  building.bottom};
	V2 bottomRight = {building.right, building.bottom};

	if (DoLinesCross(topLeft, topRight, closePoint, farPoint)) 
	{
		V2 intersection = LineIntersection(topLeft, topRight, closePoint, farPoint);
		intersection.x = closePoint.x;
		intersection.y = building.top;
		F32 distanceSquare = DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) 
		{
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(topRight, bottomRight, closePoint, farPoint)) 
	{
		V2 intersection = LineIntersection(topRight, bottomRight, closePoint, farPoint);
		intersection.x = building.right;
		intersection.y = closePoint.y;
		F32 distanceSquare = DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) 
		{
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(bottomRight, bottomLeft, closePoint, farPoint)) 
	{
		V2 intersection = LineIntersection(bottomRight, bottomLeft, closePoint, farPoint);
		intersection.x = closePoint.x;
		intersection.y = building.bottom;
		F32 distanceSquare = DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) 
		{
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(bottomLeft, topLeft, closePoint, farPoint)) 
	{
		V2 intersection = LineIntersection(bottomLeft, topLeft, closePoint, farPoint);
		intersection.x = building.left;
		intersection.y = closePoint.y;
		F32 distanceSquare = DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) 
		{
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	return result;
}

static B32 IsPointOnBuildingConnector(V2 point, Building building)
{
	F32 roadWidth = ConnectRoadWidth;

	if (building.roadAround) 
	{
		F32 left   = building.left   - roadWidth;
		F32 right  = building.right  + roadWidth;
		F32 top    = building.top    - roadWidth;
		F32 bottom = building.bottom + roadWidth;

		if (IsPointInRect(point, left, right, top, bottom)) 
		{
			return true;
		}
	}

	F32 left   = Min2(building.connectPointFarShow.x, building.connectPointClose.x);
	F32 right  = Max2(building.connectPointFarShow.x, building.connectPointClose.x);
	F32 top    = Min2(building.connectPointFarShow.y, building.connectPointClose.y);
	F32 bottom = Max2(building.connectPointFarShow.y, building.connectPointClose.y);

	if (left == right) 
	{
		left  -= roadWidth * 0.5f;
		right += roadWidth * 0.5f;
	}
	if (top == bottom) 
	{
		top    -= roadWidth * 0.5f;
		bottom += roadWidth * 0.5f;
	}

	if (IsPointInRect(point, left, right, top, bottom)) 
	{
		return true;
	}
	else 
	{
		return false;
	}
}

static void HighlightBuilding(Canvas canvas, Building building, V4 color)
{
	DrawRect(
		canvas,
		building.left, building.right, building.top, building.bottom,
		color
	);
}

static void DrawBuildingInside(Canvas canvas, Building building)
{
	V4 color = {};

	switch (building.type) 
	{
		case BuildingBlack:
			color = MakeColor(0.75f, 0.75f, 0.75f);
			break;

		case BuildingRed:
			color = MakeColor(0.75f, 0.0f, 0.0f);
			break;

		case BuildingGreen:
			color = MakeColor(0.0f, 0.75f, 0.0f);
			break;

		case BuildingBlue:
			color = MakeColor(0.0f, 0.0f, 0.75f);
			break;
	}

	V4 wallColor = color;
	if (wallColor.red > 0.2f) 
	{
		wallColor.red -= 0.2f;
	}

	if (wallColor.green > 0.2f) 
	{
		wallColor.green -= 0.2f;
	}

	if (wallColor.blue > 0.2f) 
	{
		wallColor.blue -= 0.2f;
	}

	DrawRect(canvas, building.left, building.right, building.top, building.bottom, color);

	BuildingInside* inside = building.inside;

	for (I32 i = 0; i < inside->wallCount; ++i) 
	{
		Line wall = inside->walls[i];
		DrawGridLine(canvas, wall.p1, wall.p2, wallColor, WallWidth);
	}
}

// TODO: move this to Geometry?
static B32 IsPointOnGridLine(V2 point, V2 line1, V2 line2)
{
	B32 result = false;

	if (line1.x == line2.x)
	{
		result = (point.x == line1.x);
	}
	else if (line1.y == line2.y)
	{
		result = (point.y == line1.y);
	}

	return result;
}

static B32 IsCornerVisible(BuildingInside* inside, V2 center, V2 corner)
{
	for (I32 i = 0; i < inside->wallCount; ++i) 
	{
		Line wall = inside->walls[i];

		if (!IsPointOnGridLine(corner, wall.p1, wall.p2) && DoLinesCross(center, corner, wall.p1,wall.p2)) 
		{
			return false;
		}
	}

	return true;
}

enum CornerType 
{
	CornerEdge,
	CornerEnter,
	CornerLeave
};

struct Corner 
{
	CornerType type;
	V2 point;
};

struct CornerHelper 
{
	I32 cornerCount;
	Corner* corners;
	Corner* tmpCorners;
	MemArena* arena;
};

static void AddCorner(CornerHelper* helper, CornerType type, V2 point)
{
	Corner corner = {};
	corner.type = type;
	corner.point = point;

	ArenaPush(helper->arena, Corner, corner);
	helper->cornerCount++;
}

static B32 AreCornersInOrder(V2 center, Corner corner1, Corner corner2)
{
	V2 point1 = corner1.point;
	V2 point2 = corner2.point;

	if (point1.x <= center.x && point2.x > center.x) 
	{
		return true;
	}
	else if (point1.x > center.x && point2.x <= center.x)
	{
		return false;
	}
	else if (TurnsRight(center, point1, point2))
	{
		return true;
	}
	else
	{
		return false;
	}
}

static void MergeCornerArrays(CornerHelper* helper, V2 center, I32 leftStart, I32 leftEnd, I32 rightStart, I32 rightEnd)
{
	I32 left = leftStart;
	I32 right = rightStart;

	for (I32 i = leftStart; i <= rightEnd; ++i) 
	{
		B32 chooseLeft = false;
		B32 chooseRight = false;

		if (left > leftEnd) 
		{
			chooseRight = true;
		}
		else if (right > rightEnd)
		{
			chooseLeft = true;
		}
		else if (AreCornersInOrder(center, helper->corners[left], helper->corners[right]))
		{
			chooseLeft = true;
		}
		else
		{
			chooseRight = true;
		}

		if (chooseLeft) 
		{
			helper->tmpCorners[i] = helper->corners[left];
			left++;
		}

		if (chooseRight) 
		{
			helper->tmpCorners[i] = helper->corners[right];
			right++;
		}
	}

	// TODO: use some version of memcpy here?
	for (I32 i = leftStart; i <= rightEnd; ++i) 
	{
		helper->corners[i] = helper->tmpCorners[i];
	}
}

static void SortCorners(CornerHelper* helper, V2 center)
{
	I32 length = 1;
	while (length <= helper->cornerCount) 
	{
		I32 leftStart = 0;
		while (leftStart < helper->cornerCount) 
		{
			I32 leftEnd = leftStart + (length - 1);

			I32 rightStart = leftEnd + 1;
			if (rightStart >= helper->cornerCount) 
			{
				break;
			}

			I32 rightEnd = rightStart + (length - 1);
			if (rightEnd >= helper->cornerCount) 
			{
				rightEnd = helper->cornerCount - 1;
			}

			MergeCornerArrays(helper, center, leftStart, leftEnd, rightStart, rightEnd);

			leftStart = rightEnd + 1;
		}

		length *= 2;
	}
}

static V2 NextVisiblePointAlongRay(Building building, V2 closePoint, V2 farPoint, F32 maxDistance)
{
	BuildingInside* inside = building.inside;

	V2 result = farPoint;
	F32 minDistanceSquare = maxDistance * maxDistance;

	V2 direction = PointDirection(closePoint, farPoint);
	V2 farFarPoint = closePoint + (maxDistance * direction);

	for (I32 i = 0; i < inside->wallCount; ++i) 
	{
		Line wall = inside->walls[i];

		if (wall.p1 == farPoint)
		{
			continue;
		}

		if (wall.p2 == farPoint)
		{
			continue;
		}

		if (DoLinesCross(closePoint, farFarPoint, wall.p1, wall.p2)) 
		{
			V2 crossPoint = LineIntersection(closePoint, farFarPoint, wall.p1, wall.p2);

			F32 distanceSquare = DistanceSquare(closePoint, crossPoint);
			if (distanceSquare < minDistanceSquare) 
			{
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	V2 topLeft     = MakePoint(building.left, building.top);
	V2 topRight    = MakePoint(building.right, building.top);
	V2 bottomLeft  = MakePoint(building.left, building.bottom);
	V2 bottomRight = MakePoint(building.right, building.bottom);

	if (topLeft != farPoint && topRight != farPoint) 
	{
		if (DoLinesCross(closePoint, farFarPoint, topLeft, topRight)) 
		{
			V2 crossPoint = LineIntersection(closePoint, farFarPoint, topLeft, topRight);

			F32 distanceSquare = DistanceSquare(closePoint, crossPoint);
			if (distanceSquare < minDistanceSquare) 
			{
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	if (topRight != farPoint && bottomRight != farPoint) 
	{
		if (DoLinesCross(closePoint, farFarPoint, topRight, bottomRight)) 
		{
			V2 crossPoint = LineIntersection(closePoint, farFarPoint, topRight, bottomRight);

			F32 distanceSquare = DistanceSquare(closePoint, crossPoint);
			if (distanceSquare < minDistanceSquare) 
			{
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	if (bottomRight != farPoint && bottomLeft != farPoint) 
	{
		if (DoLinesCross(closePoint, farFarPoint, bottomRight, bottomLeft)) 
		{
			V2 crossPoint = LineIntersection(closePoint, farFarPoint, bottomRight, bottomLeft);

			F32 distanceSquare = DistanceSquare(closePoint, crossPoint);
			if (distanceSquare < minDistanceSquare) 
			{
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	if (bottomLeft != farPoint && topLeft != farPoint) 
	{
		if (DoLinesCross(closePoint, farFarPoint, bottomLeft, topLeft)) 
		{
			V2 crossPoint = LineIntersection(closePoint, farFarPoint, bottomLeft, topLeft);

			F32 distanceSquare = DistanceSquare(closePoint, crossPoint);
			if (distanceSquare < minDistanceSquare) 
			{
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	return result;
}

// TODO: this is an n-square solution, is there a way to improve it?
//       something along the lines of rays on the bitmap?
//       or something based on rooms and doors?
//       or should there be a big area and each line would cut down from it if crossed?
static void DrawVisibleAreaInBuilding(Canvas canvas, Building building, V2 center, MemArena* tmpArena)
{
	BuildingInside* inside = building.inside;

	V4 lineColor = MakeColor(1.0f, 1.0f, 1.0f);

	F32 maxDistance = (building.bottom - building.top) + (building.right - building.left);

	CornerHelper helper = {};

	helper.arena = tmpArena;
	helper.cornerCount = 0;
	helper.corners = ArenaPushArray(tmpArena, Corner, 0);

	for (I32 i = 0; i < inside->wallCount; ++i) 
	{
		Line wall = inside->walls[i];

		if (IsCornerVisible(inside, center, wall.p1)) 
		{
			if (TurnsRight(center, wall.p1, wall.p2))
			{
				AddCorner(&helper, CornerEnter, wall.p1);
			}
			else 
			{
				AddCorner(&helper, CornerLeave, wall.p1);
			}
		}
		if (IsCornerVisible(inside, center, wall.p2)) 
		{
			if (TurnsRight(center, wall.p2, wall.p1))
			{
				AddCorner(&helper, CornerEnter, wall.p2);
			}
			else
			{
				AddCorner(&helper, CornerLeave, wall.p2);
			}
		}
	}

	V2 topLeft     = MakePoint(building.left, building.top);
	V2 topRight    = MakePoint(building.right, building.top);
	V2 bottomLeft  = MakePoint(building.left, building.bottom);
	V2 bottomRight = MakePoint(building.right, building.bottom);

	if (IsCornerVisible(inside, center, topLeft)) 
	{
		AddCorner(&helper, CornerEdge, topLeft);
	}

	if (IsCornerVisible(inside, center, topRight))
	{
		AddCorner(&helper, CornerEdge, topRight);
	}

	if (IsCornerVisible(inside, center, bottomLeft))
	{
		AddCorner(&helper, CornerEdge, bottomLeft);
	}

	if (IsCornerVisible(inside, center, bottomRight))
	{
		AddCorner(&helper, CornerEdge, bottomRight);
	}

	helper.tmpCorners = ArenaPushArray(tmpArena, Corner, helper.cornerCount);

	SortCorners(&helper, center);

	V2* points = ArenaPushArray(tmpArena, V2, 0);
	I32 pointCount = 0;

	for (I32 i = 0; i < helper.cornerCount; ++i) 
	{
		Corner corner = helper.corners[i];

		V2 farPoint = NextVisiblePointAlongRay(building, center, corner.point, maxDistance);

		if ((corner.type == CornerEnter) && (farPoint != corner.point)) 
		{
			ArenaPushVar(tmpArena, farPoint);
			pointCount++;
		}

		ArenaPushVar(tmpArena, corner.point);
		pointCount++;

		if ((corner.type == CornerLeave) && (farPoint != corner.point)) 
		{
			ArenaPushVar(tmpArena, farPoint);
			pointCount++;
		}
	}

	for (I32 i = 0; i < pointCount; ++i) 
	{
		V2 point = points[i];
		V2 nextPoint = {};

		if ((i + 1) < pointCount)
		{
			nextPoint = points[i + 1];
		}
		else
		{
			nextPoint = points[0];
		}

		Bresenham(canvas, point, nextPoint, lineColor);
	}

	FloodFill(canvas, center, lineColor, tmpArena);

	ArenaPopTo(tmpArena, helper.corners);
}

static void HighlightBuildingConnector(Canvas canvas, Building building, V4 color)
{
	F32 roadWidth = ConnectRoadWidth;

	if (building.roadAround) 
	{
		DrawRect(
			canvas,
			building.left - roadWidth, building.right + roadWidth,
			building.top - roadWidth, building.bottom + roadWidth,
			color
		);

		// TODO: this is a lazy solution, do this properly!
		// DrawBuilding(draw, building);
	}

	DrawLine(canvas, building.connectPointClose, building.connectPointFarShow, color, roadWidth);
}

static void DrawConnectRoad(Canvas canvas, Building building, Texture roadTexture)
{
	F32 roadWidth = ConnectRoadWidth;

	if (building.roadAround) 
	{
		WorldTextureRect(
			canvas,
			building.left - roadWidth, building.right + roadWidth,
			building.top - roadWidth, building.bottom + roadWidth,
			roadTexture
		);
	}

	WorldTextureGridLine(
		canvas, 
		building.connectPointClose, building.connectPointFarShow, roadWidth, 
		roadTexture
	);
}