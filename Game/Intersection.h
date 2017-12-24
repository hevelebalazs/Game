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

float GetIntersectionRoadWidth(Intersection intersection);
void InitTrafficLights(Intersection* intersection);
void UpdateTrafficLights(Intersection* intersection, float seconds);
void DrawTrafficLights(Renderer renderer, Intersection intersection);

void HighlightIntersection(Renderer renderer, Intersection intersection, Color color);
void DrawIntersection(Renderer renderer, Intersection intersection);