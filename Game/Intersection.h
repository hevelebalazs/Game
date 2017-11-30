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

	void Start();
	void Update(float seconds);
	void Draw(Renderer renderer);
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

	float GetRoadWidth();

	void InitTrafficLights();
	void UpdateTrafficLights(float seconds);
	void DrawTrafficLights(Renderer renderer);

	void Highlight(Renderer renderer, Color color);
	void Draw(Renderer renderer);
};