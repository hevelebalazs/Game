#include "Building.h"

void Building::draw(Renderer renderer) {
	renderer.drawRect(
		top, left, bottom, right,
		color
	);
}