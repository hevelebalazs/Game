#include "Building.h"

void Building::draw(Bitmap bitmap) {
	bitmap.drawRect(
		top, left, bottom, right,
		color
	);
}