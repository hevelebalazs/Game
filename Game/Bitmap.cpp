#include "Bitmap.h"

Color Color::Random() {
	Color randomColor = {
		(float)rand() / (float)RAND_MAX,
		(float)rand() / (float)RAND_MAX,
		(float)rand() / (float)RAND_MAX
	};

	return randomColor;
}