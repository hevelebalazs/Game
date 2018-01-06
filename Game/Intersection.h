#pragma once

#include "Renderer.h"
#include "Road.h"

struct Road;

enum TrafficLightColor {
	TrafficLightNone,
	TrafficLightRed,
	TrafficLightYellow,
	TrafficLightGreen
};

extern float trafficLightRadius;
extern float trafficLightSwitchTime;
extern float trafficLightYellowTime;

struct TrafficLight {
	Point position;
	TrafficLightColor color;
	float timeLeft;
};

// TODO: name this QuarterIndex
enum {
	QuarterNone,
	QuarterTopLeft,
	QuarterTopRight,
	QuarterBottomLeft,
	QuarterBottomRight
};

struct Intersection {
	Point position;

	// TODO: create an array of these
	Road* leftRoad = 0;
	Road* rightRoad = 0;
	Road* topRoad = 0;
	Road* bottomRoad = 0;

	// TODO: create an array of these, each should correspond to a road
	TrafficLight leftTrafficLight;
	TrafficLight rightTrafficLight;
	TrafficLight topTrafficLight;
	TrafficLight bottomTrafficLight;
};

TrafficLight* TrafficLightOfRoad(Intersection* intersection, Road* road);
bool IsPointOnIntersection(Point point, Intersection intersection);
bool IsPointOnIntersectionSidewalk(Point point, Intersection intersection);

float GetIntersectionRoadWidth(Intersection intersection);
void InitTrafficLights(Intersection* intersection);
void UpdateTrafficLights(Intersection* intersection, float seconds);
void DrawTrafficLights(Renderer renderer, Intersection intersection);

void HighlightIntersectionSidewalk(Renderer renderer, Intersection intersection, Color color);
void HighlightIntersection(Renderer renderer, Intersection intersection, Color color);
void DrawIntersection(Renderer renderer, Intersection intersection);

static inline int QuarterIndex(Intersection* intersection, Point point) {
	int result = 0;

	if (point.y < intersection->position.y) {
		if (point.x < intersection->position.x)
			result = QuarterTopLeft;
		else
			result = QuarterTopRight;
	}
	else {
		if (point.x < intersection->position.x)
			result = QuarterBottomLeft;
		else
			result = QuarterBottomRight;
	}

	return result;
}

static inline Point IntersectionSidewalkCorner(Intersection* intersection, int quarterIndex) {
	Point result = intersection->position;

	extern float sideWalkWidth;
	float distance = (GetIntersectionRoadWidth(*intersection) * 0.5f) + (sideWalkWidth * 0.5f);

	if (quarterIndex == QuarterTopLeft || quarterIndex == QuarterBottomLeft)
		result.x -= distance;
	else
		result.x += distance;

	if (quarterIndex == QuarterTopLeft || quarterIndex == QuarterTopRight)
		result.y -= distance;
	else
		result.y += distance;

	return result;
}