#pragma once

#include "Road.h"
#include "Renderer.h"

struct Road;

enum TrafficLightColor {
	TrafficLight_Red,
	TrafficLight_Yellow,
	TrafficLight_Green
};

struct TrafficLight {
	static float radius;
	static float switchTime;
	static float yellowTime;

	Point position;
	TrafficLightColor color;
	float timeLeft;
};

struct Intersection {
	// TODO: rename this to "position" for consistency
	Point coordinate;

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

float GetIntersectionRoadWidth(Intersection intersection);
void InitTrafficLights(Intersection* intersection);
void UpdateTrafficLights(Intersection* intersection, float seconds);
void DrawTrafficLights(Renderer renderer, Intersection intersection);

void HighlightIntersection(Renderer renderer, Intersection intersection, Color color);
void DrawIntersection(Renderer renderer, Intersection intersection);