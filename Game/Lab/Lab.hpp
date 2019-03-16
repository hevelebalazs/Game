#pragma once

#include <Windows.h>

#include "../Draw.hpp"

static Vec2 func GetMousePosition(Camera* camera, HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	Vec2 point = {};
	point.x = (Real32)cursorPoint.x;
	point.y = (Real32)cursorPoint.y;

	point = PixelToUnit(camera, point);

	return point;
}
