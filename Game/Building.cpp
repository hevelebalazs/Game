#include <math.h>

#include "Building.h"
#include "Geometry.h"

extern float entranceWidth = 3.0f;
extern float wallWidth = 0.5f;

static float doorWidth = 3.0f;
static float minRoomSide = 5.0f;
static float maxRoomSide = 20.0f;

float Building::connectRoadWidth;

// TODO: move these to a math file?
float Min2(float x, float y) {
	if (x < y) return x;
	else return y;
}

float Max2(float x, float y) {
	if (x > y) return x;
	else return y;
}

static Line ConnectingLine(Point point1, Point point2) {
	Line result = {};
	result.p1 = point1;
	result.p2 = point2;
	return result;
}

// TODO: move this to a math file?
static float RandomBetween(float left, float right) {
	return (left)+(right - left) * ((float)rand() / (float)RAND_MAX);
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

// TODO: move this to a math file?
static bool IsBetween(float test, float min, float max) {
	return (min <= test && test <= max);
}

// TODO: rewrite this not using recursion
static void GenerateWalls(Building* building, WallHelper* wallHelper,
						  int leftWallIndex, int rightWallIndex, int topWallIndex, int bottomWallIndex,
						  float minRoomSide, float maxRoomSide) {
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

			if (IsBetween(topDist1, minRoomSide, maxRoomSide) && IsBetween(topDist2, minRoomSide, maxRoomSide)) {
				cutY = topY;
				canCutHorizontally = true;
			}
			else if (IsBetween(bottomDist1, minRoomSide, maxRoomSide) && IsBetween(bottomDist2, minRoomSide, maxRoomSide)) {
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

			if (IsBetween(topDist1, minRoomSide, maxRoomSide) && IsBetween(topDist2, minRoomSide, height)) {
				cutY = topY;
				canCutHorizontally = true;
			}
			else if (IsBetween(bottomDist1, minRoomSide, maxRoomSide) && IsBetween(bottomDist2, minRoomSide, height)) {
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

			if (IsBetween(leftDist1, minRoomSide, maxRoomSide) && IsBetween(leftDist2, minRoomSide, width)) {
				cutX = leftX;
				canCutVertically = true;
			}
			else if (IsBetween(rightDist1, minRoomSide, maxRoomSide) && IsBetween(rightDist2, minRoomSide, width)) {
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

			if (IsBetween(leftDist1, minRoomSide, maxRoomSide) && IsBetween(leftDist2, minRoomSide, maxRoomSide)) {
				cutX = leftX;
				canCutVertically = true;
			}
			else if (IsBetween(rightDist1, minRoomSide, maxRoomSide) && IsBetween(rightDist2, minRoomSide, maxRoomSide)) {
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

		if (random < 0.5f) cutHorizontally = true;
		else cutVertically = true;
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

void GenerateBuildingInside(Building* building, WallHelper* wallHelper) {
	BuildingInside* inside = new BuildingInside;
	building->inside = inside;

	float buildingWidth = (building->right - building->left);
	float buildingHeight = (building->bottom - building->top);

	int maxWallCount = 4 + ((int)buildingWidth / (int)minRoomSide) * ((int)buildingHeight / (int)minRoomSide);

	float halfWallWidth = wallWidth * 0.5f;

	Line entrance = {};
	entrance.p1 = building->entrancePoint1;
	entrance.p2 = building->entrancePoint2;

	Line leftWall = VerticalWall(building->top, building->bottom, building->left + halfWallWidth);
	if (entrance.x1 == building->left && entrance.x2 == building->left) {
		AddHelperWallWithDoor(wallHelper, leftWall, entrance);
	}
	else {
		AddHelperWall(wallHelper, leftWall);
	}

	Line rightWall = VerticalWall(building->top, building->bottom, building->right - halfWallWidth);
	if (entrance.x1 == building->right && entrance.x2 == building->right) {
		AddHelperWallWithDoor(wallHelper, rightWall, entrance);
	}
	else {
		AddHelperWall(wallHelper, rightWall);
	}

	Line topWall = HorizontalWall(building->left, building->right, building->top + halfWallWidth);
	if (entrance.y1 == building->top && entrance.y2 == building->top) {
		AddHelperWallWithDoor(wallHelper, topWall, entrance);
	}
	else {
		AddHelperWall(wallHelper, topWall);
	}

	Line bottomWall = HorizontalWall(building->left, building->right, building->bottom - halfWallWidth);
	if (entrance.y1 == building->bottom && entrance.y2 == building->bottom) {
		AddHelperWallWithDoor(wallHelper, bottomWall, entrance);
	}
	else {
		AddHelperWall(wallHelper, bottomWall);
	}

	GenerateWalls(building, wallHelper, 0, 1, 2, 3, minRoomSide, maxRoomSide);

	int wallCount = 0;
	for (int i = 0; i < wallHelper->wallCount; ++i) {
		if (wallHelper->hasDoor[i]) wallCount += 2;
		else wallCount += 1;
	}

	inside->wallCount = 0;
	inside->walls = new Line[wallCount];

	for (int i = 0; i < wallHelper->wallCount; ++i) {
		Line wall = wallHelper->walls[i];

		if (wallHelper->hasDoor[i]) {
			Line door = wallHelper->doors[i];

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
}

// TODO: rewrite this using vector maths
// TODO: should this be a real function in GridMapCreator.cpp?
void ConnectBuildingToElem(Building* building, MapElem elem) {
	Point center = {(building->left + building->right) * 0.5f, (building->top + building->bottom) * 0.5f};

	if (elem.type == MapElemNone) {
		// TODO: should this ever happen?
	}
	else if (elem.type == MapElemRoad) {
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

		bool betweenX = (fabsf(center.x - intersection->coordinate.x) <= halfRoadWidth);
		bool betweenY = (fabsf(center.y - intersection->coordinate.y) <= halfRoadWidth);

		if (!betweenX && !betweenY) throw 1;

		if (betweenX) {
			building->connectPointFar.y = intersection->coordinate.y;
			building->connectPointFar.x = center.x;
			building->connectPointFarShow.x = center.x;
			building->connectPointClose.x = center.x;

			if (center.y > intersection->coordinate.y) {
				building->connectPointFarShow.y = building->connectPointFar.y + halfRoadWidth;
				building->connectPointClose.y = building->top;
			}
			else {
				building->connectPointFarShow.y = building->connectPointFar.y - halfRoadWidth;
				building->connectPointClose.y = building->bottom;
			}
		}
		else if (betweenY) {
			building->connectPointFar.x = intersection->coordinate.x;
			building->connectPointFar.y = center.y;
			building->connectPointFarShow.y = center.y;
			building->connectPointClose.y = center.y;

			if (center.x > intersection->coordinate.x) {
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
	if (point.x < building.left || point.x > building.right) return false;
	if (point.y < building.top || point.y > building.bottom) return false;
	return true;
}

static bool IsPointOnEdge(Point point, Building building) {
	if (point.x == building.left) return true;
	if (point.x == building.right) return true;
	if (point.y == building.top) return true;
	if (point.y == building.bottom) return true;

	return false;
}

// TODO: can this be merged with ClosestCrossPoint?
bool IsBuildingCrossed(Building building, Point point1, Point point2) {
	if (IsPointOnEdge(point1, building)) return true;
	if (IsPointOnEdge(point2, building)) return true;

	Point topLeft     = {building.left,  building.top};
	Point topRight    = {building.right, building.top};
	Point bottomLeft  = {building.left,  building.bottom};
	Point bottomRight = {building.right, building.bottom};

	if (DoLinesCross(topLeft, topRight, point1, point2)) return true;
	if (DoLinesCross(topRight, bottomRight, point1, point2)) return true;
	if (DoLinesCross(bottomRight, bottomLeft, point1, point2)) return true;
	if (DoLinesCross(bottomLeft, topLeft, point1, point2)) return true;

	return false;
}

BuildingCrossInfo ExtBuildingClosestCrossInfo(Building* building, float radius, Point closePoint, Point farPoint) {
	BuildingCrossInfo result = {};
	float minDistanceSquare = 0.0f;
	bool foundAny = false;

	Point topLeft = {building->left - radius, building->top - radius};
	Point topRight = {building->right + radius, building->top - radius};
	Point bottomLeft = {building->left - radius, building->bottom + radius};
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
			}
		}
	}

	Point inTopLeft = {building->left + radius, building->top + radius};
	Point inTopRight = {building->right - radius, building->top + radius};
	Point inBottomLeft = {building->left + radius, building->bottom - radius};
	Point inBottomRight = {building->right - radius, building->bottom - radius};
	Point inPoints[5] = {inTopLeft, inTopRight, inBottomRight, inBottomLeft, inTopLeft};

	for (int i = 0; i < 4; ++i) {
		Point inCorner1 = inPoints[i];
		Point inCorner2 = inPoints[i + 1];

		if (DoLinesCross(inCorner1, inCorner2, closePoint, farPoint)) {
			Point intersection = LineIntersection(inCorner1, inCorner2, closePoint, farPoint);
			float distanceSquare = DistanceSquare(closePoint, intersection);

			if (foundAny == false || distanceSquare < minDistanceSquare) {
				minDistanceSquare = distanceSquare;
				foundAny = true;

				result.building = building;
				result.crossPoint = intersection;
				result.corner1 = inCorner1;
				result.corner2 = inCorner2;
			}
		}
	}

	// TODO: create directed points for entrance points that point outside of the building
	Point entrancePointOut1 = building->entrancePoint1;
	Point entrancePointOut2 = building->entrancePoint2;
	Point entrancePointIn1 = building->entrancePoint1;
	Point entrancePointIn2 = building->entrancePoint2;

	if (building->entrancePoint1.x == building->left) {
		entrancePointOut1.x -= radius;
		entrancePointOut2.x -= radius;
		entrancePointIn1.x += radius;
		entrancePointIn2.x += radius;
	}
	else if (building->entrancePoint1.x == building->right) {
		entrancePointOut1.x += radius;
		entrancePointOut2.x += radius;
		entrancePointIn1.x -= radius;
		entrancePointIn2.x -= radius;
	}
	else if (building->entrancePoint1.y == building->top) {
		entrancePointOut1.y -= radius;
		entrancePointOut2.y -= radius;
		entrancePointIn1.y += radius;
		entrancePointIn2.y += radius;
	}
	else if (building->entrancePoint1.y == building->bottom) {
		entrancePointOut1.y += radius;
		entrancePointOut2.y += radius;
		entrancePointIn1.y -= radius;
		entrancePointIn2.y -= radius;
	}

	Point entranceSide11 = building->entrancePoint1;
	Point entranceSide12 = building->entrancePoint1;
	Point entranceSide21 = building->entrancePoint2;
	Point entranceSide22 = building->entrancePoint2;

	if (building->entrancePoint1.x == building->left || building->entrancePoint1.x == building->right) {
		if (building->entrancePoint1.y < building->entrancePoint2.y) {
			entranceSide11.y += radius;
			entranceSide12.y += radius;
			entranceSide21.y -= radius;
			entranceSide22.y -= radius;
		}
		else {
			entranceSide11.y -= radius;
			entranceSide12.y -= radius;
			entranceSide21.y += radius;
			entranceSide22.y += radius;
		}

		entranceSide11.x -= radius;
		entranceSide12.x += radius;

		entranceSide21.x -= radius;
		entranceSide22.x += radius;
	}
	else if (building->entrancePoint1.y == building->top || building->entrancePoint1.y == building->bottom) {
		if (building->entrancePoint1.x < building->entrancePoint2.x) {
			entranceSide11.x += radius;
			entranceSide12.x += radius;
			entranceSide21.x -= radius;
			entranceSide22.x -= radius;
		}
		else {
			entranceSide11.x -= radius;
			entranceSide12.x -= radius;
			entranceSide21.x += radius;
			entranceSide22.x += radius;
		}

		entranceSide11.y -= radius;
		entranceSide12.y += radius;

		entranceSide21.y -= radius;
		entranceSide22.y += radius;
	}

	if (DoLinesCross(entranceSide11, entranceSide12, closePoint, farPoint)) {
		result.building = building;
		result.corner1 = entranceSide11;
		result.corner2 = entranceSide12;
		result.entrance = EntranceSide;
	}
	else if (DoLinesCross(entranceSide21, entranceSide22, closePoint, farPoint)) {
		result.building = building;
		result.corner1 = entranceSide21;
		result.corner2 = entranceSide22;
		result.entrance = EntranceSide;
	}
	else if (foundAny) {
		if (DoLinesCross(entrancePointOut1, entrancePointOut2, closePoint, farPoint)) {
			result.entrance = EntranceOut;
		}
		else if (DoLinesCross(entrancePointIn1, entrancePointIn2, closePoint, farPoint)) {
			result.entrance = EntranceIn;
		}
		else {
			result.entrance = EntranceNone;
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

void HighLightBuilding(Renderer renderer, Building building, Color color) {
	DrawRect(
		renderer,
		building.top, building.left, building.bottom, building.right,
		color
	);
}

void DrawBuilding(Renderer renderer, Building building) {
	Color color = {};

	switch (building.type) {
		case BuildingType_Black: {
				color = Color{0.0f, 0.0f, 0.0f};
				break;
			}

		case BuildingType_Red: {
				color = Color{0.5f, 0.0f, 0.0f};
				break;
			}

		case BuildingType_Green: {
				color = Color{0.0f, 0.5f, 0.0f};
				break;
			}

		case BuildingType_Blue: {
				color = Color{0.0f, 0.0f, 0.5f};
				break;
			}
	}

	DrawRect(
		renderer,
		building.top, building.left, building.bottom, building.right,
		color
	);
}

void DrawBuildingInside(Renderer renderer, Building building) {
	Color color = {};

	switch (building.type) {
		case BuildingType_Black: {
			color = Color{0.75f, 0.75f, 0.75f};
			break;
		}

		case BuildingType_Red: {
			color = Color{0.75f, 0.0f, 0.0f};
			break;
		}

		case BuildingType_Green: {
			color = Color{0.0f, 0.75f, 0.0f};
			break;
		}

		case BuildingType_Blue: {
			color = Color{0.0f, 0.0f, 0.75f};
			break;
		}
	}

	Color wallColor = color;
	if (wallColor.red   > 0.2f) wallColor.red   -= 0.2f;
	if (wallColor.green > 0.2f) wallColor.green -= 0.2f;
	if (wallColor.blue  > 0.2f) wallColor.blue  -= 0.2f;

	DrawRect(renderer, building.top, building.left, building.bottom, building.right, color);

	BuildingInside* inside = building.inside;

	for (int i = 0; i < inside->wallCount; ++i) {
		Line wall = inside->walls[i];

		DrawGridLine(renderer, wall.p1, wall.p2, wallColor, wallWidth);
	}
}

void DrawConnectRoad(Renderer renderer, Building building) {
	// TODO: make this a static member of Road?
	Color roadColor = Color{0.5f, 0.5f, 0.5f};

	float roadWidth = building.connectRoadWidth;

	if (building.roadAround) {
		DrawRect(
			renderer,
			building.top - roadWidth, building.left - roadWidth,
			building.bottom + roadWidth, building.right + roadWidth,
			roadColor
		);
	}

	DrawLine(renderer, building.connectPointClose, building.connectPointFarShow, roadColor, building.connectRoadWidth);
}