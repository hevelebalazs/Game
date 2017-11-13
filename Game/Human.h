#pragma once

#include "Point.h"
#include "Renderer.h"
#include "Map.h"
#include "Building.h"
#include "Path.h"

struct Human {
	static float moveSpeed;

	Map* map;

	Point position;

	Path movePath;
	// TODO: does this belong to this struct?
	//       or should it be passed to Update?
	PathHelper* moveHelper;
	PathNode* moveNode;
	Building* moveTargetBuilding;
	Point moveStartPoint;
	Point moveEndPoint;
	float moveTotalSeconds;
	float moveSeconds;

	void InitMovement();

	void Update(float seconds);

	void Draw(Renderer renderer);
};
