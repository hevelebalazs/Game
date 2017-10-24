#pragma once
#include "Bitmap.h"

struct Building {
	float top;
	float left;
	float bottom;
	float right;

	Color color;
	void draw(Bitmap bitmap);
};