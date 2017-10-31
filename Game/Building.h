#pragma once
#include "Renderer.h"

struct Building {
	float top;
	float left;
	float bottom;
	float right;

	Color color;
	void draw(Renderer renderer);
};