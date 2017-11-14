#include "Building.h"

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
			
			if (right < road->endPoint1.y && right < road->endPoint2.y) {
				connectPointFarShow.x = road->endPoint1.x - (road->width * 0.5f);
				connectPointClose.x = right;
			}
			else {
				connectPointFarShow.x = road->endPoint1.x + (road->width * 0.5f);
				connectPointClose.x = left;
			}
		}
		else if (road->endPoint2.y == road->endPoint2.y) {
			connectPointFar.y = road->endPoint1.y;
			connectPointFar.x = center.x;
			connectPointFarShow.x = center.x;
			connectPointClose.x = center.x;

			if (bottom < road->endPoint1.x && bottom < road->endPoint2.x) {
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

		if ((intersection->coordinate.x - halfRoadWidth <= center.x) && (center.x <= intersection->coordinate.x + halfRoadWidth)) {
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
		else {
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

		if (building->connectElem.type > 4) throw 1;

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

bool Building::IsCrossed(Point point1, Point point2) {
	if (point1.x < left && point2.x < left) return false;
	if (point1.x > right && point2.x > right) return false;
	if (point1.y < top && point2.y < top) return false;
	if (point1.y > bottom && point2.y > bottom) return false;
	return true;
}

Point Building::ClosestCrossPoint(Point closePoint, Point farPoint) {
	Point result = {};

	if (closePoint.x < farPoint.x) {
		result.x = left;
		result.y = closePoint.y;
	}
	else if (closePoint.x > farPoint.x) {
		result.x = right;
		result.y = closePoint.y;
	}
	else if (closePoint.y < farPoint.y) {
		result.x = closePoint.x;
		result.y = top;
	}
	else if (closePoint.y > farPoint.y) {
		result.x = closePoint.x;
		result.y = bottom;
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
	if (top > bottom || left > right) {
		color = Color{0.0f, 0.0f, 1.0f};
	}

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

	renderer.DrawRect(
		top, left, bottom, right,
		color
	);
}