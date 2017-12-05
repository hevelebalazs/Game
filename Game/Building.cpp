#include <math.h>

#include "Building.h"
#include "Geometry.h"

extern float entranceWidth = 10.0f;
extern float wallWidth = 2.0f;

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

void GenerateBuildingInside(Building* building) {
	BuildingInside* inside = new BuildingInside;
	building->inside = inside;

	inside->wallCount = 4;
	inside->walls = new Line[inside->wallCount];

	float halfWallWidth = wallWidth * 0.5f;

	inside->walls[0].x1 = building->left;
	inside->walls[0].y1 = building->top + halfWallWidth;
	inside->walls[0].x2 = building->right;
	inside->walls[0].y2 = building->top + halfWallWidth;

	inside->walls[1].x1 = building->right - halfWallWidth;
	inside->walls[1].y1 = building->top;
	inside->walls[1].x2 = building->right - halfWallWidth;
	inside->walls[1].y2 = building->bottom;

	inside->walls[2].x1 = building->right;
	inside->walls[2].y1 = building->bottom - halfWallWidth;
	inside->walls[2].x2 = building->left;
	inside->walls[2].y2 = building->bottom - halfWallWidth;

	inside->walls[3].x1 = building->left + halfWallWidth;
	inside->walls[3].y1 = building->bottom;
	inside->walls[3].x2 = building->left + halfWallWidth;
	inside->walls[3].y2 = building->top;

	int roomRows = 3;
	int roomCols = 3;
	inside->roomCount = (roomRows * roomCols);
	inside->rooms = new Room[inside->roomCount];

	float roomWidth = (building->right - building->left) / ((float)roomCols);
	float roomHeight = (building->bottom - building->top) / ((float)roomRows);

	// TODO: use relative positions inside building?
	int roomId = 0;
	for (int row = 0; row < roomRows; ++row) {
		for (int col = 0; col < roomCols; ++col) {
			Room* room = &inside->rooms[roomId];

			room->left   = building->left + (col * roomWidth);
			room->right  = building->left + ((col + 1) * roomWidth);
			room->top    = building->top + (row * roomHeight);
			room->bottom = building->top + ((row + 1) * roomHeight);

			roomId++;
		}
	}
}

Room* GetRoom(Building* building, Point position) {
	Room* result = 0;

	BuildingInside* inside = building->inside;
	for (int i = 0; i < inside->roomCount; ++i) {
		Room* room = &inside->rooms[i];

		if ((room->left < position.x && position.x <= room->right) &&
			(room->top < position.y && position.y <= room->bottom)) { 
			result = room;
		}
	}

	return result;
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

	float entranceLineWidth = 0.8f;
	Color entranceColor = Color{
		color.red * 0.5f, 
		color.green * 0.5f, 
		color.blue * 0.5f
	};

	DrawLine(renderer, building.entrancePoint1, building.entrancePoint2, entranceColor, entranceLineWidth);
}

void DrawBuildingInside(Renderer renderer, Building building) {
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

	Color wallColor = Color{
		color.red + 0.25f,
		color.green + 0.25f,
		color.blue + 0.25f
	};

	DrawRect(renderer, building.top, building.left, building.bottom, building.right, color);

	BuildingInside* inside = building.inside;

	for (int i = 0; i < inside->wallCount; ++i) {
		Line wall = inside->walls[i];

		DrawLine(renderer, wall.p1, wall.p2, wallColor, wallWidth);
	}
}

void HighLightRoom(Renderer renderer, Room room, Color color) {
	DrawRect(renderer, room.top, room.left, room.bottom, room.right, color);
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