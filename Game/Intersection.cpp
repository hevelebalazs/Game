#include "Geometry.h"
#include "Intersection.h"
#include "Road.h"

extern float trafficLightRadius = 2.0f;
extern float trafficLightSwitchTime = 3.0f;
extern float trafficLightYellowTime = 1.0f;

TrafficLight* TrafficLightOfRoad(Intersection* intersection, Road* road) {
	if (intersection->leftRoad == road) 
		return &intersection->leftTrafficLight;
	else if (intersection->rightRoad == road)
		return &intersection->rightTrafficLight;
	else if (intersection->topRoad == road)
		return &intersection->topTrafficLight;
	else if (intersection->bottomRoad == road)
		return &intersection->bottomTrafficLight;
	else 
		return 0;
}

bool IsPointOnIntersection(Point point, Intersection intersection) {
	float roadWidth = GetIntersectionRoadWidth(intersection);
	
	float left   = intersection.position.x - (roadWidth * 0.5f);
	float right  = intersection.position.x + (roadWidth * 0.5f);
	float top    = intersection.position.y - (roadWidth * 0.5f);
	float bottom = intersection.position.y + (roadWidth * 0.5f);

	// TODO: create an IsPointInRect function?
	if (point.x < left || point.x > right) 
		return false;

	if (point.y < top || point.y > bottom) 
		return false;

	return true;
}

bool IsPointOnIntersectionSidewalk(Point point, Intersection intersection) {
	float roadWidth = GetIntersectionRoadWidth(intersection);

	float left   = intersection.position.x - roadWidth * 0.5f;
	float right  = intersection.position.x + roadWidth * 0.5f;
	float top    = intersection.position.y - roadWidth * 0.5f;
	float bottom = intersection.position.y + roadWidth * 0.5f;

	if (!intersection.topRoad) {
		if (IsPointInRect(point, left, right, top - sideWalkWidth, top))
			return true;

		top -= sideWalkWidth;
	}
	if (!intersection.bottomRoad) {
		if (IsPointInRect(point, left, right, bottom, bottom + sideWalkWidth))
			return true;

		bottom += sideWalkWidth;
	}
	if (!intersection.leftRoad) {
		if (IsPointInRect(point, left - sideWalkWidth, left, top, bottom))
			return true;
	}
	if (!intersection.rightRoad) {
		if (IsPointInRect(point, right, right + sideWalkWidth, top, bottom))
			return true;
	}

	return false;
}

static void StartTrafficLight(TrafficLight* trafficLight) {
	trafficLight->color = TrafficLightGreen;
	trafficLight->timeLeft = trafficLightSwitchTime;
}

static void UpdateTrafficLight(TrafficLight* trafficLight, float seconds) {
	if (trafficLight->color == TrafficLightGreen) {
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) {
			trafficLight->timeLeft += trafficLightYellowTime;
			trafficLight->color = TrafficLightYellow;
		}
	}
	else if (trafficLight->color == TrafficLightYellow) {
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) {
			trafficLight->color = TrafficLightRed;
		}
	}
}

static void DrawTrafficLight(Renderer renderer, TrafficLight trafficLight) {
	Color drawColor = {};

	switch (trafficLight.color) {
		case TrafficLightGreen: {
			drawColor = {0.0f, 1.0f, 0.0f}; 
			break;
		}
		case TrafficLightYellow: {
			drawColor = {1.0f, 1.0f, 0.0f};
			break;
		}
		case TrafficLightRed: {
			drawColor = {1.0f, 0.0f, 0.0f};
			break;
		}
	}

	Point position = trafficLight.position;
	float radius = trafficLightRadius;

	DrawRect(
		renderer,
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		drawColor
	);
}

void InitTrafficLights(Intersection* intersection) {
	int roadCount = 0;
	if (intersection->leftRoad)
		roadCount++;
	if (intersection->rightRoad)
		roadCount++;
	if (intersection->topRoad)
		roadCount++;
	if (intersection->bottomRoad) 
		roadCount++;

	if (roadCount <= 2) 
		return;

	float roadWidth = GetIntersectionRoadWidth(*intersection);

	Point position = intersection->position;

	if (intersection->leftRoad)
		intersection->leftTrafficLight.position = {position.x - roadWidth * 0.5f,  position.y + roadWidth * 0.25f};
	if (intersection->rightRoad)
		intersection->rightTrafficLight.position = {position.x + roadWidth * 0.5f,  position.y - roadWidth * 0.25f};
	if (intersection->topRoad)
		intersection->topTrafficLight.position = {position.x - roadWidth * 0.25f, position.y - roadWidth * 0.5f};
	if (intersection->bottomRoad)
		intersection->bottomTrafficLight.position = {position.x + roadWidth * 0.25f, position.y + roadWidth * 0.5f};

	if (intersection->leftRoad)
		intersection->leftTrafficLight.color = TrafficLightRed;
	if (intersection->rightRoad)
		intersection->rightTrafficLight.color = TrafficLightRed;
	if (intersection->topRoad)
		intersection->topTrafficLight.color = TrafficLightRed;
	if (intersection->bottomRoad) 
		intersection->bottomTrafficLight.color = TrafficLightRed;

	if (intersection->leftRoad)
		StartTrafficLight(&intersection->leftTrafficLight);
	else if (intersection->topRoad)
		StartTrafficLight(&intersection->topTrafficLight);
	else if (intersection->rightRoad)
		StartTrafficLight(&intersection->rightTrafficLight);
	else if (intersection->bottomRoad)
		StartTrafficLight(&intersection->bottomTrafficLight);
}

void UpdateTrafficLights(Intersection* intersection, float seconds) {
	int roadCount = 0;
	if (intersection->leftRoad)
		roadCount++;
	if (intersection->rightRoad)
		roadCount++;
	if (intersection->topRoad)
		roadCount++;
	if (intersection->bottomRoad)
		roadCount++;

	if (roadCount <= 2)
		return;

	// TODO: introduce an "active" member?
	// TODO: this is total nuts, update this to arrays as soon as possible!
	if (intersection->leftRoad && intersection->leftTrafficLight.color != TrafficLightRed) {
		UpdateTrafficLight(&intersection->leftTrafficLight, seconds);
		if (intersection->leftTrafficLight.color == TrafficLightRed) {
			if (intersection->topRoad)
				StartTrafficLight(&intersection->topTrafficLight);
			else if (intersection->rightRoad)
				StartTrafficLight(&intersection->rightTrafficLight);
			else if (intersection->bottomRoad)
				StartTrafficLight(&intersection->bottomTrafficLight);
		}
	}
	else if (intersection->topRoad && intersection->topTrafficLight.color != TrafficLightRed) {
		UpdateTrafficLight(&intersection->topTrafficLight, seconds);
		if (intersection->topTrafficLight.color == TrafficLightRed) {
			if (intersection->rightRoad)
				StartTrafficLight(&intersection->rightTrafficLight);
			else if (intersection->bottomRoad)
				StartTrafficLight(&intersection->bottomTrafficLight);
			else if (intersection->leftRoad)
				StartTrafficLight(&intersection->leftTrafficLight);
		}
	}
	else if (intersection->rightRoad && intersection->rightTrafficLight.color != TrafficLightRed) {
		UpdateTrafficLight(&intersection->rightTrafficLight, seconds);
		if (intersection->rightTrafficLight.color == TrafficLightRed) {
			if (intersection->bottomRoad)
				StartTrafficLight(&intersection->bottomTrafficLight);
			else if (intersection->leftRoad)
				StartTrafficLight(&intersection->leftTrafficLight);
			else if (intersection->topRoad)
				StartTrafficLight(&intersection->topTrafficLight);
		}
	}
	else if (intersection->bottomRoad && intersection->bottomTrafficLight.color != TrafficLightRed) {
		UpdateTrafficLight(&intersection->bottomTrafficLight, seconds);
		if (intersection->bottomTrafficLight.color == TrafficLightRed) {
			if (intersection->leftRoad)
				StartTrafficLight(&intersection->leftTrafficLight);
			else if (intersection->topRoad)
				StartTrafficLight(&intersection->topTrafficLight);
			else if (intersection->rightRoad)
				StartTrafficLight(&intersection->rightTrafficLight);
		}
	}
}

void DrawTrafficLights(Renderer renderer, Intersection intersection) {
	int roadCount = 0;
	if (intersection.leftRoad)
		roadCount++;
	if (intersection.rightRoad)
		roadCount++;
	if (intersection.topRoad)
		roadCount++;
	if (intersection.bottomRoad)
		roadCount++;

	if (roadCount <= 2)
		return;

	if (intersection.leftRoad)
		DrawTrafficLight(renderer, intersection.leftTrafficLight);
	if (intersection.rightRoad)
		DrawTrafficLight(renderer, intersection.rightTrafficLight);
	if (intersection.topRoad)
		DrawTrafficLight(renderer, intersection.topTrafficLight);
	if (intersection.bottomRoad)
		DrawTrafficLight(renderer, intersection.bottomTrafficLight);
}

float GetIntersectionRoadWidth(Intersection intersection) {
	if (intersection.leftRoad)
		return intersection.leftRoad->width;
	if (intersection.rightRoad)
		return intersection.rightRoad->width;
	if (intersection.topRoad)
		return intersection.topRoad->width;
	if (intersection.bottomRoad)
		return intersection.bottomRoad->width;

	return 0.0f;
}

void HighlightIntersection(Renderer renderer, Intersection intersection, Color color) {
	if (intersection.leftRoad || intersection.rightRoad || intersection.topRoad || intersection.bottomRoad) {
		float roadWidth = GetIntersectionRoadWidth(intersection);

		Point position = intersection.position;

		float stripeWidth = roadWidth / 10.0f;
		float top =    position.y - (roadWidth / 2.0f);
		float bottom = position.y + (roadWidth / 2.0f);
		float left =   position.x - (roadWidth / 2.0f);
		float right =  position.x + (roadWidth / 2.0f);

		DrawRect(renderer, top, left, bottom, right, color);
	}
}

void HighlightIntersectionSidewalk(Renderer renderer, Intersection intersection, Color color) {
	float roadWidth = GetIntersectionRoadWidth(intersection);
	Point position = intersection.position;

	float left   = position.x - roadWidth * 0.5f;
	float right  = position.x + roadWidth * 0.5f;
	float top    = position.y - roadWidth * 0.5f;
	float bottom = position.y + roadWidth * 0.5f;

	if (!intersection.topRoad)
		DrawRect(renderer, top - sideWalkWidth, left, top, right, color);

	if (!intersection.bottomRoad)
		DrawRect(renderer, bottom, left, bottom + sideWalkWidth, right, color);

	if (!intersection.topRoad)
		top -= sideWalkWidth;
	if (!intersection.bottomRoad)
		bottom += sideWalkWidth;

	if (!intersection.leftRoad)
		DrawRect(renderer, top, left - sideWalkWidth, bottom, left, color);
	
	if (!intersection.rightRoad)
		DrawRect(renderer, top, right, bottom, right + sideWalkWidth, color);
}

static inline void DrawIntersectionSidewalk(Renderer renderer, Intersection intersection) {
	float roadWidth = GetIntersectionRoadWidth(intersection);
	Point position = intersection.position;

	float left   = position.x - roadWidth * 0.5f;
	float right  = position.x + roadWidth * 0.5f;
	float top    = position.y - roadWidth * 0.5f;
	float bottom = position.y + roadWidth * 0.5f;

	if (!intersection.topRoad)
		DrawRect(renderer, top - sideWalkWidth, left, top, right, sideWalkColor);

	if (!intersection.bottomRoad)
		DrawRect(renderer, bottom, left, bottom + sideWalkWidth, right, sideWalkColor);

	if (!intersection.topRoad)
		top -= sideWalkWidth;
	if (!intersection.bottomRoad)
		bottom += sideWalkWidth;

	if (!intersection.leftRoad)
		DrawRect(renderer, top, left - sideWalkWidth, bottom, left, sideWalkColor);
	
	if (!intersection.rightRoad)
		DrawRect(renderer, top, right, bottom, right + sideWalkWidth, sideWalkColor);
}

void DrawIntersection(Renderer renderer, Intersection intersection) {
	DrawIntersectionSidewalk(renderer, intersection);

	float roadWidth = GetIntersectionRoadWidth(intersection);

	Point coordinate = intersection.position;

	float top    = coordinate.y - (roadWidth / 2.0f);
	float bottom = coordinate.y + (roadWidth / 2.0f);
	float left   = coordinate.x - (roadWidth / 2.0f);
	float right  = coordinate.x + (roadWidth / 2.0f);

	float midX = (left + right) / 2;
	float midY = (top + bottom) / 2;

	Color color = { 0.5f, 0.5f, 0.5f };
	DrawRect(renderer, top, left, bottom, right, color);

	float stripeWidth = roadWidth / 20.0f;
	Color stripeColor = { 1.0f, 1.0f, 1.0f };

	int roadCount = 0;

	if (intersection.topRoad)
		roadCount++;
	if (intersection.leftRoad)
		roadCount++;
	if (intersection.bottomRoad)
		roadCount++;
	if (intersection.rightRoad)
		roadCount++;

	if (roadCount > 2) {
		if (intersection.topRoad)
			DrawRect(renderer, top, left, top + stripeWidth, right, stripeColor);
		if (intersection.leftRoad)
			DrawRect(renderer, top, left, bottom, left + stripeWidth, stripeColor);
		if (intersection.bottomRoad)
			DrawRect(renderer, bottom - stripeWidth, left, bottom, right, stripeColor);
		if (intersection.rightRoad)
			DrawRect(renderer, top, right - stripeWidth, bottom, right, stripeColor);
	}
	else {
		if (intersection.topRoad)
			DrawRect(renderer, top, midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		if (intersection.leftRoad)
			DrawRect(renderer, midY - (stripeWidth / 2.0f), left, midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		if (intersection.bottomRoad)
			DrawRect(renderer, midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), bottom, midX + (stripeWidth / 2.0f), stripeColor);
		if (intersection.rightRoad)
			DrawRect(renderer, midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), right, stripeColor);
	}
}