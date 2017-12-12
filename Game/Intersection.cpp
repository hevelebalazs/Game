#include "Intersection.h"

float TrafficLight::radius = 2.0f;
float TrafficLight::switchTime = 3.0f;
float TrafficLight::yellowTime = 1.0f;

// TODO: use this where it is done manually
TrafficLight* TrafficLightOfRoad(Intersection* intersection, Road* road) {
	if (intersection->leftRoad == road)        return &intersection->leftTrafficLight;
	else if (intersection->rightRoad == road)  return &intersection->rightTrafficLight;
	else if (intersection->topRoad == road)    return &intersection->topTrafficLight;
	else if (intersection->bottomRoad == road) return &intersection->bottomTrafficLight;
	else return 0;
}

bool IsPointOnIntersection(Point point, Intersection intersection) {
	float roadWidth = GetIntersectionRoadWidth(intersection);
	
	float left   = intersection.coordinate.x - (roadWidth * 0.5f);
	float right  = intersection.coordinate.x + (roadWidth * 0.5f);
	float top    = intersection.coordinate.y - (roadWidth * 0.5f);
	float bottom = intersection.coordinate.y + (roadWidth * 0.5f);

	// TODO: create an IsPointInRect function?
	if (point.x < left || point.x > right) return false;
	if (point.y < top || point.y > bottom) return false;

	return true;
}

static void StartTrafficLight(TrafficLight* trafficLight) {
	trafficLight->color = TrafficLight_Green;
	trafficLight->timeLeft = TrafficLight::switchTime;
}

static void UpdateTrafficLight(TrafficLight* trafficLight, float seconds) {
	if (trafficLight->color == TrafficLight_Green) {
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) {
			trafficLight->timeLeft += TrafficLight::yellowTime;
			trafficLight->color = TrafficLight_Yellow;
		}
	}
	else if (trafficLight->color == TrafficLight_Yellow) {
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) {
			trafficLight->color = TrafficLight_Red;
		}
	}
}

static void DrawTrafficLight(Renderer renderer, TrafficLight trafficLight) {
	Color drawColor = {};

	switch (trafficLight.color) {
		case TrafficLight_Green: {
			drawColor = {0.0f, 1.0f, 0.0f}; 
			break;
		}
		case TrafficLight_Yellow: {
			drawColor = {1.0f, 1.0f, 0.0f};
			break;
		}
		case TrafficLight_Red: {
			drawColor = {1.0f, 0.0f, 0.0f};
			break;
		}
	}

	Point position = trafficLight.position;
	float radius = trafficLight.radius;

	DrawRect(
		renderer,
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		drawColor
	);
}

void InitTrafficLights(Intersection* intersection) {
	int roadCount = 0;
	if (intersection->leftRoad)   roadCount++;
	if (intersection->rightRoad)  roadCount++;
	if (intersection->topRoad)    roadCount++;
	if (intersection->bottomRoad) roadCount++;

	if (roadCount <= 2) return;

	float roadWidth = GetIntersectionRoadWidth(*intersection);

	Point coordinate = intersection->coordinate;

	if (intersection->leftRoad) {
		intersection->leftTrafficLight.position = {coordinate.x - roadWidth * 0.5f,  coordinate.y + roadWidth * 0.25f};
	}
	if (intersection->rightRoad) {
		intersection->rightTrafficLight.position = {coordinate.x + roadWidth * 0.5f,  coordinate.y - roadWidth * 0.25f};
	}
	if (intersection->topRoad) {
		intersection->topTrafficLight.position = {coordinate.x - roadWidth * 0.25f, coordinate.y - roadWidth * 0.5f};
	}
	if (intersection->bottomRoad) {
		intersection->bottomTrafficLight.position = {coordinate.x + roadWidth * 0.25f, coordinate.y + roadWidth * 0.5f};
	}

	if (intersection->leftRoad)   intersection->leftTrafficLight.color   = TrafficLight_Red;
	if (intersection->rightRoad)  intersection->rightTrafficLight.color  = TrafficLight_Red;
	if (intersection->topRoad)    intersection->topTrafficLight.color    = TrafficLight_Red;
	if (intersection->bottomRoad) intersection->bottomTrafficLight.color = TrafficLight_Red;

	if (intersection->leftRoad)        StartTrafficLight(&intersection->leftTrafficLight);
	else if (intersection->topRoad)    StartTrafficLight(&intersection->topTrafficLight);
	else if (intersection->rightRoad)  StartTrafficLight(&intersection->rightTrafficLight);
	else if (intersection->bottomRoad) StartTrafficLight(&intersection->bottomTrafficLight);
}

void UpdateTrafficLights(Intersection* intersection, float seconds) {
	int roadCount = 0;
	if (intersection->leftRoad)   roadCount++;
	if (intersection->rightRoad)  roadCount++;
	if (intersection->topRoad)    roadCount++;
	if (intersection->bottomRoad) roadCount++;

	if (roadCount <= 2) return;

	// TODO: introduce an "active" member?
	// TODO: this is total nuts, update this to arrays as soon as possible!
	if (intersection->leftRoad && intersection->leftTrafficLight.color != TrafficLight_Red) {
		UpdateTrafficLight(&intersection->leftTrafficLight, seconds);
		if (intersection->leftTrafficLight.color == TrafficLight_Red) {
			if (intersection->topRoad)         StartTrafficLight(&intersection->topTrafficLight);
			else if (intersection->rightRoad)  StartTrafficLight(&intersection->rightTrafficLight);
			else if (intersection->bottomRoad) StartTrafficLight(&intersection->bottomTrafficLight);
		}
	}
	else if (intersection->topRoad && intersection->topTrafficLight.color != TrafficLight_Red) {
		UpdateTrafficLight(&intersection->topTrafficLight, seconds);
		if (intersection->topTrafficLight.color == TrafficLight_Red) {
			if (intersection->rightRoad)       StartTrafficLight(&intersection->rightTrafficLight);
			else if (intersection->bottomRoad) StartTrafficLight(&intersection->bottomTrafficLight);
			else if (intersection->leftRoad)   StartTrafficLight(&intersection->leftTrafficLight);
		}
	}
	else if (intersection->rightRoad && intersection->rightTrafficLight.color != TrafficLight_Red) {
		UpdateTrafficLight(&intersection->rightTrafficLight, seconds);
		if (intersection->rightTrafficLight.color == TrafficLight_Red) {
			if (intersection->bottomRoad)    StartTrafficLight(&intersection->bottomTrafficLight);
			else if (intersection->leftRoad) StartTrafficLight(&intersection->leftTrafficLight);
			else if (intersection->topRoad)  StartTrafficLight(&intersection->topTrafficLight);
		}
	}
	else if (intersection->bottomRoad && intersection->bottomTrafficLight.color != TrafficLight_Red) {
		UpdateTrafficLight(&intersection->bottomTrafficLight, seconds);
		if (intersection->bottomTrafficLight.color == TrafficLight_Red) {
			if (intersection->leftRoad)       StartTrafficLight(&intersection->leftTrafficLight);
			else if (intersection->topRoad)   StartTrafficLight(&intersection->topTrafficLight);
			else if (intersection->rightRoad) StartTrafficLight(&intersection->rightTrafficLight);
		}
	}
}

void DrawTrafficLights(Renderer renderer, Intersection intersection) {
	int roadCount = 0;
	if (intersection.leftRoad)   roadCount++;
	if (intersection.rightRoad)  roadCount++;
	if (intersection.topRoad)    roadCount++;
	if (intersection.bottomRoad) roadCount++;

	if (roadCount <= 2) return;

	if (intersection.leftRoad)   DrawTrafficLight(renderer, intersection.leftTrafficLight);
	if (intersection.rightRoad)  DrawTrafficLight(renderer, intersection.rightTrafficLight);
	if (intersection.topRoad)    DrawTrafficLight(renderer, intersection.topTrafficLight);
	if (intersection.bottomRoad) DrawTrafficLight(renderer, intersection.bottomTrafficLight);
}

float GetIntersectionRoadWidth(Intersection intersection) {
	if (intersection.leftRoad)   return intersection.leftRoad->width;
	if (intersection.rightRoad)  return intersection.rightRoad->width;
	if (intersection.topRoad)    return intersection.topRoad->width;
	if (intersection.bottomRoad) return intersection.bottomRoad->width;

	return 0.0f;
}

void HighlightIntersection(Renderer renderer, Intersection intersection, Color color) {
	if (intersection.leftRoad || intersection.rightRoad || intersection.topRoad || intersection.bottomRoad) {
		float roadWidth = GetIntersectionRoadWidth(intersection);

		Point coordinate = intersection.coordinate;

		float stripeWidth = roadWidth / 10.0f;
		float top =    coordinate.y - (roadWidth / 2.0f);
		float bottom = coordinate.y + (roadWidth / 2.0f);
		float left =   coordinate.x - (roadWidth / 2.0f);
		float right =  coordinate.x + (roadWidth / 2.0f);

		DrawRect(renderer, top, left, bottom, right, color);
	}
}

void DrawIntersection(Renderer renderer, Intersection intersection) {
	float roadWidth = GetIntersectionRoadWidth(intersection);

	Point coordinate = intersection.coordinate;

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

	if (intersection.topRoad) roadCount++;
	if (intersection.leftRoad) roadCount++;
	if (intersection.bottomRoad) roadCount++;
	if (intersection.rightRoad) roadCount++;

	if (roadCount > 2) {
		if (intersection.topRoad) {
			DrawRect(renderer, top, left, top + stripeWidth, right, stripeColor);
		}
		if (intersection.leftRoad) {
			DrawRect(renderer, top, left, bottom, left + stripeWidth, stripeColor);
		}
		if (intersection.bottomRoad) {
			DrawRect(renderer, bottom - stripeWidth, left, bottom, right, stripeColor);
		}
		if (intersection.rightRoad) {
			DrawRect(renderer, top, right - stripeWidth, bottom, right, stripeColor);
		}
	}
	else {
		if (intersection.topRoad) {
			DrawRect(renderer, top, midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (intersection.leftRoad) {
			DrawRect(renderer, midY - (stripeWidth / 2.0f), left, midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (intersection.bottomRoad) {
			DrawRect(renderer, midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), bottom, midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (intersection.rightRoad) {
			DrawRect(renderer, midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), right, stripeColor);
		}
	}
}