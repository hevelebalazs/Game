#include "Map.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void Map::draw(HDC context) {
	HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
	RECT rect = { 0, 0, width, height };

	FillRect(context, &rect, brush);

	for (int i = 0; i < intersectionCount; ++i)
		intersections[i].draw(context);
	for (int i = 0; i < roadCount; ++i)
		roads[i].draw(context);
}

