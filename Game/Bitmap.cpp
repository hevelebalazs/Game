#include "Bitmap.h"
#include "Math.h"

Color RandomColor() {
	Color randomColor = {
		RandomBetween(0.0f, 1.0f),
		RandomBetween(0.0f, 1.0f),
		RandomBetween(0.0f, 1.0f)
	};

	return randomColor;
}