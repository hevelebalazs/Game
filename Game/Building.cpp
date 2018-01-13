#include "Building.h"
#include "Math.h"
#include "Geometry.h"

extern float connectRoadWidth = 5.0f;
extern float entranceWidth = 3.0f;
extern float wallWidth = 0.5f;

static float doorWidth = 3.0f;
static float minRoomSide = 5.0f;
static float maxRoomSide = 20.0f;

static Line ConnectingLine(Point point1, Point point2) {
	Line result = {};
	result.p1 = point1;
	result.p2 = point2;
	return result;
}

static Line HorizontalWall(float left, float right, float y) {
	Line wall = {};
	wall.x1 = left;
	wall.y1 = y;
	wall.x2 = right;
	wall.y2 = y;
	return wall;
}

static Line VerticalWall(float top, float bottom, float x) {
	Line wall = {};
	wall.x1 = x;
	wall.y1 = top;
	wall.x2 = x;
	wall.y2 = bottom;
	return wall;
}

struct WallHelper {
	int maxWallCount;
	int wallCount;
	Line* walls;
	bool* hasDoor;
	Line* doors;
};

static void AddHelperWall(WallHelper* helper, Line wall) {
	if (helper->wallCount < helper->maxWallCount) {
		helper->walls[helper->wallCount] = wall;
		helper->hasDoor[helper->wallCount] = false;
		helper->wallCount++;
	}
}

static void AddHelperWallWithDoor(WallHelper* helper, Line wall, Line door) {
	if (helper->wallCount < helper->maxWallCount) {
		helper->walls[helper->wallCount] = wall;
		helper->hasDoor[helper->wallCount] = true;
		helper->doors[helper->wallCount] = door;
		helper->wallCount++;
	}
}

// TODO: rewrite this not using recursion
static void GenerateWalls(Building* building, WallHelper* wallHelper,
						  int leftWallIndex, int rightWallIndex, int topWallIndex, int bottomWallIndex,
						  float minRoomSide, float maxRoomSide) 
{
	float left   = wallHelper->walls[leftWallIndex].x1;
	float right  = wallHelper->walls[rightWallIndex].x1;
	float top    = wallHelper->walls[topWallIndex].y1;
	float bottom = wallHelper->walls[bottomWallIndex].y1;

	float width = (right - left);
	float height = (bottom - top);

	float cutDistance = RandomBetween(minRoomSide, maxRoomSide);

	bool canCutHorizontally = (cutDistance <= height - minRoomSide);
	float cutY = top + cutDistance;
	{
		int doorCount = 0;
		Line doors[2] = {};

		if (wallHelper->hasDoor[leftWallIndex]) {
			doors[doorCount] = wallHelper->doors[leftWallIndex];
			doors[doorCount].y1 -= wallWidth * 0.5f;
			doors[doorCount].y2 += wallWidth * 0.5f;
			doorCount++;
		}
		if (wallHelper->hasDoor[rightWallIndex]) {
			doors[doorCount] = wallHelper->doors[rightWallIndex];
			doors[doorCount].y1 -= wallWidth * 0.5f;
			doors[doorCount].y2 += wallWidth * 0.5f;
			doorCount++;
		}

		if (doorCount == 2 && (doors[0].y1 < doors[1].y2 && doors[1].y1 < doors[0].y2)) {
			Line door = {};
			door.y1 = Min2(doors[0].y1, doors[1].y1);
			door.y2 = Max2(doors[0].y2, doors[1].y2);

			doorCount = 1;
			doors[0] = door;
		}

		if (doorCount > 0 && (doors[0].y1 < cutY && cutY < doors[0].y2)) {
			float topY = doors[0].y1;
			float topDist1 = topY - top;
			float topDist2 = bottom - topY;

			float bottomY = doors[0].y2;
			float bottomDist1 = bottomY - top;
			float bottomDist2 = bottom - bottomY;

			if (topDist1 >= minRoomSide && topDist2 >= minRoomSide) {
				cutY = topY;
				canCutHorizontally = true;
			}
			else if (bottomDist1 >= minRoomSide && bottomDist2 >= minRoomSide) {
				cutY = bottomY;
				canCutHorizontally = true;
			}
			else {
				canCutHorizontally = false;
			}
		}

		if (doorCount > 1 && (doors[1].y1 < cutY && cutY < doors[1].y2)) {
			float topY = doors[1].y1;
			float topDist1 = topY - top;
			float topDist2 = bottom - topY;

			float bottomY = doors[1].y2;
			float bottomDist1 = bottomY - top;
			float bottomDist2 = bottom - bottomY;

			if (topDist1 >= minRoomSide && topDist2 >= minRoomSide) {
				cutY = topY;
				canCutHorizontally = true;
			}
			else if (bottomDist1 >= minRoomSide && bottomDist2 >= minRoomSide) {
				cutY = bottomY;
				canCutHorizontally = true;
			}
			else {
				canCutHorizontally = false;
			}
		}
	}

	bool canCutVertically = (cutDistance <= width - minRoomSide);
	float cutX = left + cutDistance;
	{
		int doorCount = 0;
		Line doors[2] = {};

		if (wallHelper->hasDoor[topWallIndex]) {
			doors[doorCount] = wallHelper->doors[topWallIndex];
			doors[doorCount].x1 -= wallWidth * 0.5f;
			doors[doorCount].x2 += wallWidth * 0.5f;
			doorCount++;
		}
		if (wallHelper->hasDoor[bottomWallIndex]) {
			doors[doorCount] = wallHelper->doors[bottomWallIndex];
			doors[doorCount].x1 -= wallWidth * 0.5f;
			doors[doorCount].x2 += wallWidth * 0.5f;
			doorCount++;
		}

		if (doorCount == 2 && (doors[0].x1 < doors[1].x2 && doors[1].x1 < doors[0].x2)) {
			Line door = {};
			door.x1 = Min2(doors[0].x1, doors[1].x1);
			door.x2 = Max2(doors[0].x2, doors[1].x2);

			doorCount = 1;
			doors[0] = door;
		}

		if (doorCount > 0 && (doors[0].x1 < cutX && cutX < doors[0].x2)) {
			float leftX = doors[0].x1;
			float leftDist1 = leftX - left;
			float leftDist2 = right - leftX;

			float rightX = doors[0].x2;
			float rightDist1 = rightX - left;
			float rightDist2 = right - rightX;

			if (leftDist1 >= minRoomSide && leftDist2 >= minRoomSide) {
				cutX = leftX;
				canCutVertically = true;
			}
			else if (rightDist1 >= minRoomSide && rightDist2 >= minRoomSide) {
				cutX = rightX;
				canCutVertically = true;
			}
			else {
				canCutVertically = false;
			}
		}

		if (doorCount > 1 && (doors[1].x1 < cutX && cutX < doors[1].x2)) {
			float leftX = doors[1].x1;
			float leftDist1 = leftX - left;
			float leftDist2 = right - leftX;

			float rightX = doors[1].x2;
			float rightDist1 = rightX - left;
			float rightDist2 = right - rightX;

			if (leftDist1 >= minRoomSide && leftDist2 >= minRoomSide) {
				cutX = leftX;
				canCutVertically = true;
			}
			else if (rightDist1 >= minRoomSide && rightDist2 >= minRoomSide) {
				cutX = rightX;
				canCutVertically = true;
			}
			else {
				canCutVertically = false;
			}
		}
	}

	bool cutHorizontally = false;
	bool cutVertically = false;

	if (canCutHorizontally && canCutVertically) {
		float random = RandomBetween(0.0f, 1.0f);

		if (random < 0.5f) 
			cutHorizontally = true;
		else 
			cutVertically = true;
	}
	else if (canCutHorizontally) {
		cutHorizontally = true;
	}
	else if (canCutVertically) {
		cutVertically = true;
	}

	BuildingInside* inside = building->inside;
	
	if (cutVertically) {
		int wallIndex = wallHelper->wallCount;

		Line wall = VerticalWall(top, bottom, cutX);

		float centerY = RandomBetween(
			Min2(wall.y1, wall.y2) + doorWidth * 0.5f,
			Max2(wall.y1, wall.y2) - doorWidth * 0.5f
		);

		Line door = {};
		door.x1 = wall.x1;
		door.y1 = centerY - doorWidth * 0.5f;
		door.x2 = wall.x2;
		door.y2 = centerY + doorWidth * 0.5f;

		AddHelperWallWithDoor(wallHelper, wall, door);

		GenerateWalls(building, wallHelper,     wallIndex, rightWallIndex, topWallIndex, bottomWallIndex, minRoomSide, maxRoomSide);
		GenerateWalls(building, wallHelper, leftWallIndex,      wallIndex, topWallIndex, bottomWallIndex, minRoomSide, maxRoomSide);
	}
	else if (cutHorizontally) {
		int wallIndex = wallHelper->wallCount;

		Line wall = HorizontalWall(left, right, cutY);

		float centerX = RandomBetween(
			Min2(wall.x1, wall.x2) + doorWidth * 0.5f,
			Max2(wall.x1, wall.x2) - doorWidth * 0.5f
		);

		Line door = {};
		door.x1 = centerX - doorWidth * 0.5f;
		door.y1 = wall.y1;
		door.x2 = centerX + doorWidth * 0.5f;
		door.y2 = wall.y2;
		AddHelperWallWithDoor(wallHelper, wall, door);

		GenerateWalls(building, wallHelper, leftWallIndex, rightWallIndex,    wallIndex, bottomWallIndex, minRoomSide, maxRoomSide);
		GenerateWalls(building, wallHelper, leftWallIndex, rightWallIndex, topWallIndex,       wallIndex, minRoomSide, maxRoomSide);
	}
}

void GenerateBuildingInside(Building* building, MemArena* arena, MemArena* tmpArena) {
	BuildingInside* inside = ArenaPushType(arena, BuildingInside);
	building->inside = inside;

	WallHelper wallHelper = {};
	wallHelper.maxWallCount = 100;
	wallHelper.wallCount = 0;
	wallHelper.walls = ArenaPushArray(tmpArena, Line, wallHelper.maxWallCount);
	wallHelper.hasDoor = ArenaPushArray(tmpArena, bool, wallHelper.maxWallCount);
	wallHelper.doors = ArenaPushArray(tmpArena, Line, wallHelper.maxWallCount);

	float buildingWidth = (building->right - building->left);
	float buildingHeight = (building->bottom - building->top);

	int maxWallCount = 4 + ((int)buildingWidth / (int)minRoomSide) * ((int)buildingHeight / (int)minRoomSide);

	float halfWallWidth = wallWidth * 0.5f;

	Line entrance = {};
	entrance.p1 = building->entrancePoint1;
	entrance.p2 = building->entrancePoint2;

	Line leftWall = VerticalWall(building->top + halfWallWidth, building->bottom - halfWallWidth, building->left + halfWallWidth);
	if (entrance.x1 == building->left && entrance.x2 == building->left)
		AddHelperWallWithDoor(&wallHelper, leftWall, entrance);
	else
		AddHelperWall(&wallHelper, leftWall);

	Line rightWall = VerticalWall(building->top + halfWallWidth, building->bottom - halfWallWidth, building->right - halfWallWidth);
	if (entrance.x1 == building->right && entrance.x2 == building->right)
		AddHelperWallWithDoor(&wallHelper, rightWall, entrance);
	else
		AddHelperWall(&wallHelper, rightWall);

	Line topWall = HorizontalWall(building->left, building->right, building->top + halfWallWidth);
	if (entrance.y1 == building->top && entrance.y2 == building->top)
		AddHelperWallWithDoor(&wallHelper, topWall, entrance);
	else
		AddHelperWall(&wallHelper, topWall);

	Line bottomWall = HorizontalWall(building->left, building->right, building->bottom - halfWallWidth);
	if (entrance.y1 == building->bottom && entrance.y2 == building->bottom)
		AddHelperWallWithDoor(&wallHelper, bottomWall, entrance);
	else
		AddHelperWall(&wallHelper, bottomWall);

	GenerateWalls(building, &wallHelper, 0, 1, 2, 3, minRoomSide, maxRoomSide);

	int wallCount = 0;
	for (int i = 0; i < wallHelper.wallCount; ++i) {
		if (wallHelper.hasDoor[i]) 
			wallCount += 2;
		else 
			wallCount += 1;
	}

	inside->wallCount = 0;
	inside->walls = ArenaPushArray(arena, Line, wallCount);

	for (int i = 0; i < wallHelper.wallCount; ++i) {
		Line wall = wallHelper.walls[i];

		if (wallHelper.hasDoor[i]) {
			Line door = wallHelper.doors[i];

			if (wall.x1 == wall.x2) {
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
			else if (wall.y1 == wall.y2) {
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
		else {
			inside->walls[inside->wallCount] = wall;
			inside->wallCount++;
		}
	}

	ArenaPopTo(tmpArena, wallHelper.walls);
}

// TODO: rewrite this using vector maths
// TODO: should this be in GridMapCreator.cpp?
void ConnectBuildingToElem(Building* building, MapElem elem) {
	Point center = {(building->left + building->right) * 0.5f, (building->top + building->bottom) * 0.5f};

	if (elem.type == MapElemRoad) {
		Road* road = elem.road;

		if (road->endPoint1.x == road->endPoint2.x) {
			building->connectPointFar.x = road->endPoint1.x;
			building->connectPointFar.y = center.y;
			building->connectPointFarShow.y = center.y;
			building->connectPointClose.y = center.y;
			
			if (building->right < road->endPoint1.x) {
				building->connectPointFarShow.x = road->endPoint1.x - (road->width * 0.5f);
				building->connectPointClose.x = building->right;
			}
			else {
				building->connectPointFarShow.x = road->endPoint1.x + (road->width * 0.5f);
				building->connectPointClose.x = building->left;
			}
		}
		else if (road->endPoint1.y == road->endPoint2.y) {
			building->connectPointFar.y = road->endPoint1.y;
			building->connectPointFar.x = center.x;
			building->connectPointFarShow.x = center.x;
			building->connectPointClose.x = center.x;

			if (building->bottom < road->endPoint1.y) {
				building->connectPointFarShow.y = road->endPoint1.y - (road->width * 0.5f);
				building->connectPointClose.y = building->bottom;
			}
			else {
				building->connectPointFarShow.y = road->endPoint1.y + (road->width * 0.5f);
				building->connectPointClose.y = building->top;
			}
		}
	}
	else if (elem.type == MapElemIntersection) {
		Intersection* intersection = elem.intersection;

		float halfRoadWidth = GetIntersectionRoadWidth(*intersection) * 0.5f;

		bool betweenX = (Abs(center.x - intersection->position.x) <= halfRoadWidth);
		bool betweenY = (Abs(center.y - intersection->position.y) <= halfRoadWidth);

		if (betweenX) {
			building->connectPointFar.y = intersection->position.y;
			building->connectPointFar.x = center.x;
			building->connectPointFarShow.x = center.x;
			building->connectPointClose.x = center.x;

			if (center.y > intersection->position.y) {
				building->connectPointFarShow.y = building->connectPointFar.y + halfRoadWidth;
				building->connectPointClose.y = building->top;
			}
			else {
				building->connectPointFarShow.y = building->connectPointFar.y - halfRoadWidth;
				building->connectPointClose.y = building->bottom;
			}
		}
		else if (betweenY) {
			building->connectPointFar.x = intersection->position.x;
			building->connectPointFar.y = center.y;
			building->connectPointFarShow.y = center.y;
			building->connectPointClose.y = center.y;

			if (center.x > intersection->position.x) {
				building->connectPointFarShow.x = building->connectPointFar.x + halfRoadWidth;
				building->connectPointClose.x = building->left;
			}
			else {
				building->connectPointFarShow.x = building->connectPointFar.x - halfRoadWidth;
				building->connectPointClose.x = building->right;
			}
		}
	}
	else if (elem.type == MapElemBuilding) {
		Building* connectBuilding = elem.building;

		building->connectPointFar = ClosestBuildingCrossPoint(*connectBuilding, building->connectPointClose, building->connectPointFar);
		building->connectPointFarShow = building->connectPointFar;
	}

	building->connectElem = elem;

	building->entrancePoint1 = building->connectPointClose;
	building->entrancePoint2 = building->connectPointClose;

	if (building->connectPointClose.x == building->left || building->connectPointClose.x == building->right) {
		building->entrancePoint1.y -= entranceWidth * 0.5f;
		building->entrancePoint2.y += entranceWidth * 0.5f;
	}
	else if (building->connectPointClose.y == building->top || building->connectPointClose.y == building->bottom) {
		building->entrancePoint1.x -= entranceWidth * 0.5f;
		building->entrancePoint2.x += entranceWidth * 0.5f;
	}
}

bool IsPointInBuilding(Point point, Building building) {
	if (point.x < building.left || point.x > building.right) 
		return false;

	if (point.y < building.top || point.y > building.bottom) 
		return false;

	return true;
}

bool IsPointInExtBuilding(Point point, Building building, float radius) {
	if (point.x < building.left - radius || point.x > building.right + radius) 
		return false;

	if (point.y < building.top - radius || point.y > building.bottom + radius) 
		return false;

	return true;
}

static bool IsPointOnEdge(Point point, Building building) {
	if (point.x == building.left) 
		return true;

	if (point.x == building.right) 
		return true;

	if (point.y == building.top) 
		return true;

	if (point.y == building.bottom) 
		return true;

	return false;
}

// TODO: can this be merged with ClosestCrossPoint?
bool IsBuildingCrossed(Building building, Point point1, Point point2) {
	if (IsPointOnEdge(point1, building)) 
		return true;

	if (IsPointOnEdge(point2, building)) 
		return true;

	Point topLeft     = {building.left,  building.top};
	Point topRight    = {building.right, building.top};
	Point bottomLeft  = {building.left,  building.bottom};
	Point bottomRight = {building.right, building.bottom};

	if (DoLinesCross(topLeft, topRight, point1, point2)) 
		return true;

	if (DoLinesCross(topRight, bottomRight, point1, point2)) 
		return true;

	if (DoLinesCross(bottomRight, bottomLeft, point1, point2)) 
		return true;

	if (DoLinesCross(bottomLeft, topLeft, point1, point2)) 
		return true;

	return false;
}

BuildingCrossInfo ExtBuildingClosestCrossInfo(Building* building, float radius, Point closePoint, Point farPoint) {
	BuildingCrossInfo result = {};
	result.type = CrossNone;
	
	float minDistanceSquare = 0.0f;
	bool foundAny = false;

	Point topLeft =     {building->left  - radius, building->top    - radius};
	Point topRight =    {building->right + radius, building->top    - radius};
	Point bottomLeft =  {building->left  - radius, building->bottom + radius};
	Point bottomRight = {building->right + radius, building->bottom + radius};
	Point points[5] = {topLeft, topRight, bottomRight, bottomLeft, topLeft};

	for (int i = 0; i < 4; ++i) {
		Point corner1 = points[i];
		Point corner2 = points[i + 1];

		if (DoLinesCross(corner1, corner2, closePoint, farPoint)) {
			Point intersection = LineIntersection(corner1, corner2, closePoint, farPoint);
			float distanceSquare = DistanceSquare(closePoint, intersection);

			if (foundAny == false || distanceSquare < minDistanceSquare) {
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
	Point entrance1 = building->entrancePoint1;
	Point entrance2 = building->entrancePoint2;

	if (entrance1.x == building->left && entrance2.x == building->left) {
		entrance1.x -= radius;
		entrance2.x -= radius;
	}
	else if (entrance1.x == building->right && entrance2.x == building->right) {
		entrance1.x += radius;
		entrance2.x += radius;
	}
	else if (entrance1.y == building->top && entrance2.y == building->top) {
		entrance1.y -= radius;
		entrance2.y -= radius;
	}
	else if (entrance2.y == building->bottom && entrance2.y == building->bottom) {
		entrance1.y += radius;
		entrance2.y += radius;
	}

	if (entrance1.x < entrance2.x) {
		entrance1.x += radius;
		entrance2.x -= radius;
	}
	else if (entrance1.x > entrance2.x) {
		entrance1.x -= radius;
		entrance2.x += radius;
	}

	if (entrance1.y < entrance2.y) {
		entrance1.y += radius;
		entrance2.y -= radius;
	}
	else if (entrance1.y > entrance2.y) {
		entrance1.y -= radius;
		entrance2.y += radius;
	}

	if (DoLinesCross(entrance1, entrance2, closePoint, farPoint)) {
		Point intersection = LineIntersection(entrance1, entrance2, closePoint, farPoint);
		
		result.building = building;
		result.crossPoint = intersection;
		result.corner1 = building->entrancePoint1;
		result.corner2 = building->entrancePoint2;
		result.type = CrossEntrance;
	}

	return result;
}

BuildingCrossInfo ExtBuildingInsideClosestCrossInfo(Building* building, float radius, Point closePoint, Point farPoint) {
	BuildingCrossInfo result = {};
	result.type = CrossNone;

	float minDistanceSquare = 0.0f;
	bool foundAny = false;

	BuildingInside* inside = building->inside;
	for (int i = 0; i < inside->wallCount; ++i) {
		Line wall = inside->walls[i];

		Point endPoint1 = wall.p1;
		Point endPoint2 = wall.p2;

		if (endPoint1.x < endPoint2.x) {
			endPoint1.x -= radius;
			endPoint2.x += radius;
		}
		else if (endPoint1.x > endPoint2.x) {
			endPoint1.x += radius;
			endPoint2.x -= radius;
		}

		if (endPoint1.y < endPoint2.y) {
			endPoint1.y -= radius;
			endPoint2.y += radius;
		}
		else if (endPoint1.y > endPoint2.y) {
			endPoint1.y += radius;
			endPoint2.y -= radius;
		}

		Point add = {};
		if (endPoint1.x == endPoint2.x) 
			add = {1.0f, 0.0f};
		else 
			add = {0.0f, 1.0f};

		float wallRadius = radius + (wallWidth * 0.5f);

		Point point1 = PointSum(endPoint1, PointProd(wallRadius, add));
		Point point2 = PointSum(endPoint1, PointProd(-wallRadius, add));
		Point point3 = PointSum(endPoint2, PointProd(-wallRadius, add));
		Point point4 = PointSum(endPoint2, PointProd(wallRadius, add));

		Point points[5] = {point1, point2, point3, point4, point1};

		for (int i = 0; i < 4; ++i) {
			Point corner1 = points[i];
			Point corner2 = points[i + 1];

			if (DoLinesCross(corner1, corner2, closePoint, farPoint)) {
				Point intersection = LineIntersection(corner1, corner2, closePoint, farPoint);
				float distanceSquare = DistanceSquare(closePoint, intersection);

				if (foundAny == false || distanceSquare < minDistanceSquare) {
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

Point ClosestBuildingCrossPoint(Building building, Point closePoint, Point farPoint) {
	Point result = {};
	float minDistanceSquare = 0.0f;
	bool foundAny = false;

	Point topLeft = {building.left,  building.top};
	Point topRight = {building.right, building.top};
	Point bottomLeft = {building.left,  building.bottom};
	Point bottomRight = {building.right, building.bottom};

	if (DoLinesCross(topLeft, topRight, closePoint, farPoint)) {
		Point intersection = LineIntersection(topLeft, topRight, closePoint, farPoint);
		intersection.y = building.top;
		float distanceSquare = DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) {
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(topRight, bottomRight, closePoint, farPoint)) {
		Point intersection = LineIntersection(topRight, bottomRight, closePoint, farPoint);
		intersection.x = building.right;
		float distanceSquare = DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) {
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(bottomRight, bottomLeft, closePoint, farPoint)) {
		Point intersection = LineIntersection(bottomRight, bottomLeft, closePoint, farPoint);
		intersection.y = building.bottom;
		float distanceSquare = DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) {
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(bottomLeft, topLeft, closePoint, farPoint)) {
		Point intersection = LineIntersection(bottomLeft, topLeft, closePoint, farPoint);
		intersection.x = building.left;
		float distanceSquare = DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) {
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	return result;
}

bool IsPointOnBuildingConnector(Point point, Building building) {
	float roadWidth = connectRoadWidth;

	if (building.roadAround) {
		float left   = building.left   - roadWidth;
		float right  = building.right  + roadWidth;
		float top    = building.top    - roadWidth;
		float bottom = building.bottom + roadWidth;

		if (IsPointInRect(point, left, right, top, bottom)) 
			return true;
	}

	float left   = Min2(building.connectPointFarShow.x, building.connectPointClose.x);
	float right  = Max2(building.connectPointFarShow.x, building.connectPointClose.x);
	float top    = Min2(building.connectPointFarShow.y, building.connectPointClose.y);
	float bottom = Max2(building.connectPointFarShow.y, building.connectPointClose.y);

	if (left == right) {
		left  -= roadWidth * 0.5f;
		right += roadWidth * 0.5f;
	}
	if (top == bottom) {
		top    -= roadWidth * 0.5f;
		bottom += roadWidth * 0.5f;
	}

	if (IsPointInRect(point, left, right, top, bottom)) 
		return true;
	else 
		return false;
}

void HighlightBuilding(Renderer renderer, Building building, Color color) {
	DrawRect(
		renderer,
		building.top, building.left, building.bottom, building.right,
		color
	);
}

void DrawBuilding(Renderer renderer, Building building) {
	if (building.right < CameraLeftCoord(renderer.camera))
		return;
	if (building.left > CameraRightCoord(renderer.camera))
		return;
	if (building.bottom < CameraTopCoord(renderer.camera))
		return;
	if (building.top > CameraBottomCoord(renderer.camera))
		return;

	Color color = {};

	switch (building.type) {
		case BuildingBlack: {
				color = Color{0.0f, 0.0f, 0.0f};
				break;
			}

		case BuildingRed: {
				color = Color{0.5f, 0.0f, 0.0f};
				break;
			}

		case BuildingGreen: {
				color = Color{0.0f, 0.5f, 0.0f};
				break;
			}

		case BuildingBlue: {
				color = Color{0.0f, 0.0f, 0.5f};
				break;
			}
	}

	Point topLeft     = Point{building.left,  building.top};
	Point topRight    = Point{building.right, building.top};
	Point bottomLeft  = Point{building.left,  building.bottom};
	Point bottomRight = Point{building.right, building.bottom};

	Point topLeftZ     = ProjectPointToGround(*renderer.camera, topLeft,     building.height);
	Point topRightZ    = ProjectPointToGround(*renderer.camera, topRight,    building.height);
	Point bottomLeftZ  = ProjectPointToGround(*renderer.camera, bottomLeft,  building.height);
	Point bottomRightZ = ProjectPointToGround(*renderer.camera, bottomRight, building.height);

	float leftZ   = Min2(topLeftZ.x, bottomLeftZ.x);
	float rightZ  = Max2(topRightZ.x, bottomRightZ.x);
	float topZ    = Min2(topLeftZ.y, topRightZ.y);
	float bottomZ = Max2(bottomLeftZ.y, bottomRightZ.y);

	DrawRect(renderer, topZ, leftZ, bottomZ, rightZ, color);

	if (building.left < leftZ)
		DrawQuadPoints(renderer, topLeft, topLeftZ, bottomLeftZ, bottomLeft, color);
	if (building.right > rightZ)
		DrawQuadPoints(renderer, bottomRight, bottomRightZ, topRightZ, topRight, color);
	if (building.top < topZ)
		DrawQuadPoints(renderer, topRight, topRightZ, topLeftZ, topLeft, color);
	if (building.bottom > bottomZ)
		DrawQuadPoints(renderer, bottomLeft, bottomLeftZ, bottomRightZ, bottomRight, color);

	Color frameColor = Color{1.0f, 1.0f, 1.0f};
	Bresenham(renderer, topLeftZ,     topRightZ,    frameColor);
	Bresenham(renderer, topRightZ,    bottomRightZ, frameColor);
	Bresenham(renderer, bottomRightZ, bottomLeftZ,  frameColor);
	Bresenham(renderer, bottomLeftZ,  topLeftZ,     frameColor);

	if (building.left < leftZ)
		Bresenham(renderer, topLeft, bottomLeft, frameColor);
	if (building.right > rightZ)
		Bresenham(renderer, topRight, bottomRight, frameColor);
	if (building.top < topZ)
		Bresenham(renderer, topLeft, topRight, frameColor);
	if (building.bottom > bottomZ)
		Bresenham(renderer, bottomLeft, bottomRight, frameColor);

	if ((building.left < leftZ) || (building.top < topZ))
		Bresenham(renderer, topLeft, topLeftZ, frameColor);
	if ((building.right > rightZ) || (building.top < topZ))
		Bresenham(renderer, topRight, topRightZ, frameColor);
	if ((building.left < leftZ) || (building.bottom > bottomZ))
		Bresenham(renderer, bottomLeft, bottomLeftZ, frameColor);
	if ((building.right > rightZ) || (building.bottom > bottomZ))
		Bresenham(renderer, bottomRight, bottomRightZ, frameColor);

	/*
	DrawRect(
		renderer,
		topZ, leftZ, bottomZ, rightZ,
		color
	);
	*/
}

void DrawBuildingInside(Renderer renderer, Building building) {
	Color color = {};

	switch (building.type) {
		case BuildingBlack: {
			color = Color{0.75f, 0.75f, 0.75f};
			break;
		}

		case BuildingRed: {
			color = Color{0.75f, 0.0f, 0.0f};
			break;
		}

		case BuildingGreen: {
			color = Color{0.0f, 0.75f, 0.0f};
			break;
		}

		case BuildingBlue: {
			color = Color{0.0f, 0.0f, 0.75f};
			break;
		}
	}

	Color wallColor = color;
	if (wallColor.red   > 0.2f) 
		wallColor.red   -= 0.2f;

	if (wallColor.green > 0.2f) 
		wallColor.green -= 0.2f;

	if (wallColor.blue  > 0.2f) 
		wallColor.blue  -= 0.2f;

	DrawRect(renderer, building.top, building.left, building.bottom, building.right, color);

	BuildingInside* inside = building.inside;

	for (int i = 0; i < inside->wallCount; ++i) {
		Line wall = inside->walls[i];

		DrawGridLine(renderer, wall.p1, wall.p2, wallColor, wallWidth);
	}
}

// TODO: move this to Geometry?
static inline bool IsPointOnGridLine(Point point, Point line1, Point line2) {
	bool result = false;

	if (line1.x == line2.x)
		result = (point.x == line1.x);
	else if (line1.y == line2.y)
		result = (point.y == line1.y);

	return result;
}

static inline bool IsCornerVisible(BuildingInside* inside, Point center, Point corner) {
	for (int i = 0; i < inside->wallCount; ++i) {
		Line wall = inside->walls[i];

		if (!IsPointOnGridLine(corner, wall.p1, wall.p2) && DoLinesCross(center, corner, wall.p1,wall.p2)) 
			return false;
	}

	return true;
}

enum CornerType {
	CornerEdge,
	CornerEnter,
	CornerLeave
};

struct Corner {
	CornerType type;
	Point point;
};

struct CornerHelper {
	int cornerCount;
	Corner* corners;
	Corner* tmpCorners;
	MemArena* arena;
};

static void AddCorner(CornerHelper* helper, CornerType type, Point point) {
	Corner corner = {};
	corner.type = type;
	corner.point = point;

	ArenaPush(helper->arena, Corner, corner);
	helper->cornerCount++;
}

static inline bool AreCornersInOrder(Point center, Corner corner1, Corner corner2) {
	Point point1 = corner1.point;
	Point point2 = corner2.point;

	if (point1.x <= center.x && point2.x > center.x) 
		return true;
	else if (point1.x > center.x && point2.x <= center.x)
		return false;
	else if (TurnsRight(center, point1, point2))
		return true;
	else
		return false;
}

static inline void MergeCornerArrays(CornerHelper* helper, Point center, int leftStart, int leftEnd, int rightStart, int rightEnd) {
	int left = leftStart;
	int right = rightStart;

	for (int i = leftStart; i <= rightEnd; ++i) {
		bool chooseLeft = false;
		bool chooseRight = false;

		if (left > leftEnd) 
			chooseRight = true;
		else if (right > rightEnd)
			chooseLeft = true;
		else if (AreCornersInOrder(center, helper->corners[left], helper->corners[right]))
			chooseLeft = true;
		else
			chooseRight = true;

		if (chooseLeft) {
			helper->tmpCorners[i] = helper->corners[left];
			left++;
		}

		if (chooseRight) {
			helper->tmpCorners[i] = helper->corners[right];
			right++;
		}
	}

	// TODO: use some version of memcpy here?
	for (int i = leftStart; i <= rightEnd; ++i) {
		helper->corners[i] = helper->tmpCorners[i];
	}
}

static inline void SortCorners(CornerHelper* helper, Point center) {
	int length = 1;
	while (length <= helper->cornerCount) {
		int leftStart = 0;
		while (leftStart < helper->cornerCount) {
			int leftEnd = leftStart + (length - 1);

			int rightStart = leftEnd + 1;
			if (rightStart >= helper->cornerCount) 
				break;

			int rightEnd = rightStart + (length - 1);
			if (rightEnd >= helper->cornerCount) 
				rightEnd = helper->cornerCount - 1;

			MergeCornerArrays(helper, center, leftStart, leftEnd, rightStart, rightEnd);

			leftStart = rightEnd + 1;
		}

		length *= 2;
	}
}

static Point NextVisiblePointAlongRay(Building building, Point closePoint, Point farPoint, float maxDistance) {
	BuildingInside* inside = building.inside;

	Point result = farPoint;
	float minDistanceSquare = maxDistance * maxDistance;

	Point direction = PointDirection(closePoint, farPoint);
	Point farFarPoint = PointSum(
		closePoint, 
		PointProd(maxDistance, direction)
	);

	for (int i = 0; i < inside->wallCount; ++i) {
		Line wall = inside->walls[i];

		if (PointEqual(wall.p1, farPoint)) 
			continue;

		if (PointEqual(wall.p2, farPoint)) 
			continue;

		if (DoLinesCross(closePoint, farFarPoint, wall.p1, wall.p2)) {
			Point crossPoint = LineIntersection(closePoint, farFarPoint, wall.p1, wall.p2);

			float distanceSquare = DistanceSquare(closePoint, crossPoint);

			if (distanceSquare < minDistanceSquare) {
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	Point topLeft     = Point{building.left, building.top};
	Point topRight    = Point{building.right, building.top};
	Point bottomLeft  = Point{building.left, building.bottom};
	Point bottomRight = Point{building.right, building.bottom};

	if (!PointEqual(topLeft, farPoint) && !PointEqual(topRight, farPoint)) {
		if (DoLinesCross(closePoint, farFarPoint, topLeft, topRight)) {
			Point crossPoint = LineIntersection(closePoint, farFarPoint, topLeft, topRight);

			float distanceSquare = DistanceSquare(closePoint, crossPoint);

			if (distanceSquare < minDistanceSquare) {
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	if (!PointEqual(topRight, farPoint) && !PointEqual(bottomRight, farPoint)) {
		if (DoLinesCross(closePoint, farFarPoint, topRight, bottomRight)) {
			Point crossPoint = LineIntersection(closePoint, farFarPoint, topRight, bottomRight);

			float distanceSquare = DistanceSquare(closePoint, crossPoint);

			if (distanceSquare < minDistanceSquare) {
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	if (!PointEqual(bottomRight, farPoint) && !PointEqual(bottomLeft, farPoint)) {
		if (DoLinesCross(closePoint, farFarPoint, bottomRight, bottomLeft)) {
			Point crossPoint = LineIntersection(closePoint, farFarPoint, bottomRight, bottomLeft);

			float distanceSquare = DistanceSquare(closePoint, crossPoint);

			if (distanceSquare < minDistanceSquare) {
				minDistanceSquare = distanceSquare;
				result = crossPoint;
			}
		}
	}

	if (!PointEqual(bottomLeft, farPoint) && !PointEqual(topLeft, farPoint)) {
		if (DoLinesCross(closePoint, farFarPoint, bottomLeft, topLeft)) {
			Point crossPoint = LineIntersection(closePoint, farFarPoint, bottomLeft, topLeft);

			float distanceSquare = DistanceSquare(closePoint, crossPoint);

			if (distanceSquare < minDistanceSquare) {
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
void DrawVisibleAreaInBuilding(Renderer renderer, Building building, Point center, MemArena* tmpArena) {
	BuildingInside* inside = building.inside;

	Color lineColor = Color{1.0f, 1.0f, 1.0f};

	float maxDistance = (building.bottom - building.top) + (building.right - building.left);

	CornerHelper helper = {};

	helper.arena = tmpArena;
	helper.cornerCount = 0;
	helper.corners = ArenaPushArray(tmpArena, Corner, 0);

	for (int i = 0; i < inside->wallCount; ++i) {
		Line wall = inside->walls[i];

		if (IsCornerVisible(inside, center, wall.p1)) {
			if (TurnsRight(center, wall.p1, wall.p2))
				AddCorner(&helper, CornerEnter, wall.p1);
			else 
				AddCorner(&helper, CornerLeave, wall.p1);
		}
		if (IsCornerVisible(inside, center, wall.p2)) {
			if (TurnsRight(center, wall.p2, wall.p1))
				AddCorner(&helper, CornerEnter, wall.p2);
			else
				AddCorner(&helper, CornerLeave, wall.p2);
		}
	}

	Point topLeft     = Point{building.left, building.top};
	Point topRight    = Point{building.right, building.top};
	Point bottomLeft  = Point{building.left, building.bottom};
	Point bottomRight = Point{building.right, building.bottom};

	if (IsCornerVisible(inside, center, topLeft)) 
		AddCorner(&helper, CornerEdge, topLeft);

	if (IsCornerVisible(inside, center, topRight))
		AddCorner(&helper, CornerEdge, topRight);

	if (IsCornerVisible(inside, center, bottomLeft))
		AddCorner(&helper, CornerEdge, bottomLeft);

	if (IsCornerVisible(inside, center, bottomRight))
		AddCorner(&helper, CornerEdge, bottomRight);

	helper.tmpCorners = ArenaPushArray(tmpArena, Corner, helper.cornerCount);

	SortCorners(&helper, center);

	Point* points = ArenaPushArray(tmpArena, Point, 0);
	int pointCount = 0;

	for (int i = 0; i < helper.cornerCount; ++i) {
		Corner corner = helper.corners[i];

		Point farPoint = NextVisiblePointAlongRay(building, center, corner.point, maxDistance);

		if ((corner.type == CornerEnter) && !PointEqual(farPoint, corner.point)) {
			ArenaPushVar(tmpArena, farPoint);
			pointCount++;
		}

		ArenaPushVar(tmpArena, corner.point);
		pointCount++;

		if ((corner.type == CornerLeave) && !PointEqual(farPoint, corner.point)) {
			ArenaPushVar(tmpArena, farPoint);
			pointCount++;
		}
	}

	for (int i = 0; i < pointCount; ++i) {
		Point point = points[i];
		Point nextPoint = {};

		if ((i + 1) < pointCount)
			nextPoint = points[i + 1];
		else
			nextPoint = points[0];

		Bresenham(renderer, point, nextPoint, lineColor);
	}

	FloodFill(renderer, center, lineColor, tmpArena);

	ArenaPopTo(tmpArena, helper.corners);
}

void HighlightBuildingConnector(Renderer renderer, Building building, Color color) {
	float roadWidth = connectRoadWidth;

	if (building.roadAround) {
		DrawRect(
			renderer,
			building.top - roadWidth, building.left - roadWidth,
			building.bottom + roadWidth, building.right + roadWidth,
			color
		);

		// TODO: this is a lazy solution, do this properly!
		DrawBuilding(renderer, building);
	}

	DrawLine(renderer, building.connectPointClose, building.connectPointFarShow, color, roadWidth);
}

void DrawConnectRoad(Renderer renderer, Building building) {
	// TODO: make this a static member of Road?
	Color roadColor = Color{0.5f, 0.5f, 0.5f};

	float roadWidth = connectRoadWidth;

	if (building.roadAround) {
		DrawRect(
			renderer,
			building.top - roadWidth, building.left - roadWidth,
			building.bottom + roadWidth, building.right + roadWidth,
			roadColor
		);
	}

	DrawLine(renderer, building.connectPointClose, building.connectPointFarShow, roadColor, roadWidth);
}