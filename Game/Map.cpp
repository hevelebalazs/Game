#include "Map.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void Map::draw(HDC context) {
	HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
	RECT rect = { 0, 0, (LONG)width, (LONG)height };

	FillRect(context, &rect, brush);

	for (int i = 0; i < intersectionCount; ++i)
		intersections[i].draw(context);
	for (int i = 0; i < roadCount; ++i)
		roads[i].draw(context);
}