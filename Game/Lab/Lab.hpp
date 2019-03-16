#pragma once

#include <Windows.h>

#include "../Draw.hpp"

static V2 func GetMousePosition(Camera* camera, HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	V2 point = {};
	point.x = F32(cursorPoint.x);
	point.y = F32(cursorPoint.y);

	point = PixelToUnit(camera, point);

	return point;
}
