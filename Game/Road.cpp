#include "Road.h"
#include <stdio.h>

void Road::draw(HDC context){
	HBRUSH brushGrey = CreateSolidBrush(RGB(128, 128, 128));

	RECT rect;
	if (endPoint1.x == endPoint2.x)
		rect = { (LONG)(endPoint1.x - width / 2), (LONG)(endPoint1.y), (LONG)(endPoint2.x + width / 2), (LONG)(endPoint2.y) };
	else if (endPoint1.y == endPoint2.y)
		rect = { (LONG)(endPoint1.x), (LONG)(endPoint1.y - width / 2), (LONG)(endPoint2.x), (LONG)(endPoint2.y + width / 2)};
	else
		printf("Road.draw: invalid endpoints!");

	FillRect(context, &rect, brushGrey);
}