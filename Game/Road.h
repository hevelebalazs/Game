#include "Point.h"
#include "Bitmap.h"
#include <Windows.h>

struct Road{
	Point endPoint1;
	Point endPoint2;
	float width;
	
	void draw(Bitmap bitmap);
};