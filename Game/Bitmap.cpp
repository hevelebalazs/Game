#include "Bitmap.h"

Color Color::random() {
	Color randomColor = {
		(float)rand() / (float)RAND_MAX,
		(float)rand() / (float)RAND_MAX,
		(float)rand() / (float)RAND_MAX
	};

	return randomColor;
}