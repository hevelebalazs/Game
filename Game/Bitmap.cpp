#include "Bitmap.h"

Color RandomColor() {
	Color randomColor = {
		(float)rand() / (float)RAND_MAX,
		(float)rand() / (float)RAND_MAX,
		(float)rand() / (float)RAND_MAX
	};

	return randomColor;
}