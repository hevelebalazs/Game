#include <math.h>

#include "Building.h"
#include "Geometry.h"

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

// TODO: should this be a real function in GridMapCreator.cpp?
void Building::ConnectTo(MapElem elem) {
	Point center = {(left + right) * 0.5f, (top + bottom) * 0.5f};

	if (elem.type == MapElemType::NONE) {
		// TODO: should this ever happen?
	}
	else if (elem.type == MapElemType::ROAD) {
		Road* road = elem.road;

		if (road->endPoint1.x == road->endPoint2.x) {
			connectPointFar.x = road->endPoint1.x;
			connectPointFar.y = center.y;
			connectPointFarShow.y = center.y;
			connectPointClose.y = center.y;
			
			if (right < road->endPoint1.x) {
				connectPointFarShow.x = road->endPoint1.x - (road->width * 0.5f);
				connectPointClose.x = right;
			}
			else {
				connectPointFarShow.x = road->endPoint1.x + (road->width * 0.5f);
				connectPointClose.x = left;
			}
		}
		else if (road->endPoint1.y == road->endPoint2.y) {
			connectPointFar.y = road->endPoint1.y;
			connectPointFar.x = center.x;
			connectPointFarShow.x = center.x;
			connectPointClose.x = center.x;

			if (bottom < road->endPoint1.y) {
				connectPointFarShow.y = road->endPoint1.y - (road->width * 0.5f);
				connectPointClose.y = bottom;
			}
			else {
				connectPointFarShow.y = road->endPoint1.y + (road->width * 0.5f);
				connectPointClose.y = top;
			}
		}
	}
	else if (elem.type == MapElemType::INTERSECTION) {
		Intersection* intersection = elem.intersection;

		float halfRoadWidth = intersection->GetRoadWidth() * 0.5f;

		bool betweenX = (fabsf(center.x - intersection->coordinate.x) <= halfRoadWidth);
		bool betweenY = (fabsf(center.y - intersection->coordinate.y) <= halfRoadWidth);

		if (!betweenX && !betweenY) throw 1;

		if (betweenX) {
			connectPointFar.y = intersection->coordinate.y;
			connectPointFar.x = center.x;
			connectPointFarShow.x = center.x;
			connectPointClose.x = center.x;

			if (center.y > intersection->coordinate.y) {
				connectPointFarShow.y = connectPointFar.y + halfRoadWidth;
				connectPointClose.y = top;
			}
			else {
				connectPointFarShow.y = connectPointFar.y - halfRoadWidth;
				connectPointClose.y = bottom;
			}
		}
		else if (betweenY) {
			connectPointFar.x = intersection->coordinate.x;
			connectPointFar.y = center.y;
			connectPointFarShow.y = center.y;
			connectPointClose.y = center.y;

			if (center.x > intersection->coordinate.x) {
				connectPointFarShow.x = connectPointFar.x + halfRoadWidth;
				connectPointClose.x = left;
			}
			else {
				connectPointFarShow.x = connectPointFar.x - halfRoadWidth;
				connectPointClose.x = right;
			}
		}
	}
	else if (elem.type == MapElemType::BUILDING) {
		Building* building = elem.building;

		connectPointFar = building->ClosestCrossPoint(connectPointClose, connectPointFar);
		connectPointFarShow = connectPointFar;
	}

	connectElem = elem;
}

bool Building::IsPointInside(Point point) {
	if (point.x < left || point.x > right) return false;
	if (point.y < top || point.y > bottom) return false;
	return true;
}

static bool IsTouching(Building* building, Point point) {
	if (point.x == building->left) return true;
	if (point.x == building->right) return true;
	if (point.y == building->top) return true;
	if (point.y == building->bottom) return true;

	return false;
}

// TODO: can this be merged with ClosestCrossPoint?
bool Building::IsCrossed(Point point1, Point point2) {
	if (IsTouching(this, point1)) return true;
	if (IsTouching(this, point2)) return true;

	Point topLeft = {left, top};
	Point topRight = {right, top};
	Point bottomLeft = {left, bottom};
	Point bottomRight = {right, bottom};

	if (DoLinesCross(topLeft, topRight, point1, point2)) return true;
	if (DoLinesCross(topRight, bottomRight, point1, point2)) return true;
	if (DoLinesCross(bottomRight, bottomLeft, point1, point2)) return true;
	if (DoLinesCross(bottomLeft, topLeft, point1, point2)) return true;

	return false;
}

BuildingCrossInfo Building::ExtClosestCrossInfo(Point closePoint, Point farPoint, float radius) {
	BuildingCrossInfo result = {};
	float minDistanceSquare = 0.0f;
	bool foundAny = false;

	Point topLeft     = {left  - radius, top    - radius};
	Point topRight    = {right + radius, top    - radius};
	Point bottomLeft  = {left  - radius, bottom + radius};
	Point bottomRight = {right + radius, bottom + radius};
	Point points[5] = {topLeft, topRight, bottomRight, bottomLeft, topLeft};

	for (int i = 0; i < 4; ++i) {
		Point corner1 = points[i];
		Point corner2 = points[i + 1];

		if (DoLinesCross(corner1, corner2, closePoint, farPoint)) {
			Point intersection = LineIntersection(corner1, corner2, closePoint, farPoint);
			float distanceSquare = Point::DistanceSquare(closePoint, intersection);

			if (foundAny == false || distanceSquare < minDistanceSquare) {
				minDistanceSquare = distanceSquare;
				foundAny = true;

				result.building = this;
				result.crossPoint = intersection;
				result.corner1 = corner1;
				result.corner2 = corner2;
			}
		}
	}

	return result;
}

Point Building::ClosestCrossPoint(Point closePoint, Point farPoint) {
	Point result = {};
	float minDistanceSquare = 0.0f;
	bool foundAny = false;

	Point topLeft = {left, top};
	Point topRight = {right, top};
	Point bottomLeft = {left, bottom};
	Point bottomRight = {right, bottom};

	if (DoLinesCross(topLeft, topRight, closePoint, farPoint)) {
		Point intersection = LineIntersection(topLeft, topRight, closePoint, farPoint);
		intersection.y = top;
		float distanceSquare = Point::DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) {
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(topRight, bottomRight, closePoint, farPoint)) {
		Point intersection = LineIntersection(topRight, bottomRight, closePoint, farPoint);
		intersection.x = right;
		float distanceSquare = Point::DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) {
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(bottomRight, bottomLeft, closePoint, farPoint)) {
		Point intersection = LineIntersection(bottomRight, bottomLeft, closePoint, farPoint);
		intersection.y = bottom;
		float distanceSquare = Point::DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) {
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	if (DoLinesCross(bottomLeft, topLeft, closePoint, farPoint)) {
		Point intersection = LineIntersection(bottomLeft, topLeft, closePoint, farPoint);
		intersection.x = left;
		float distanceSquare = Point::DistanceSquare(closePoint, intersection);

		if (foundAny == false || distanceSquare < minDistanceSquare) {
			minDistanceSquare = distanceSquare;
			foundAny = true;
			result = intersection;
		}
	}

	return result;
}

void Building::HighLight(Renderer renderer, Color color) {
	renderer.DrawRect(
		top, left, bottom, right,
		color
	);
}

void Building::Draw(Renderer renderer) {
	// TODO: make this a static member of Road?
	Color roadColor = Color{0.5f, 0.5f, 0.5f};
	
	if (roadAround) {
		renderer.DrawRect(
			top - connectRoadWidth, left - connectRoadWidth,
			bottom + connectRoadWidth, right + connectRoadWidth,
			roadColor
		);
	}

	float connectPadding = connectRoadWidth * 0.5f;

	Point point1 = connectPointClose;
	Point point2 = connectPointFarShow;

	if (connectPointClose.x == connectPointFarShow.x) {
		point1.x -= connectPadding;
		point2.x += connectPadding;
	}
	else if (connectPointClose.y == connectPointFarShow.y) {
		point1.y -= connectPadding;
		point2.y += connectPadding;
	}

	renderer.DrawRect(
		point1.y, point1.x, point2.y, point2.x,
		roadColor
	);

	Color color = {};

	switch (type) {
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

	// DEBUG
	if (connectElem.type == MapElemType::INTERSECTION) color = Color{1.0f, 0.0f, 0.0f};
	else color = Color{0.0f, 0.0f, 0.0f};

	renderer.DrawRect(
		top, left, bottom, right,
		color
	);
}