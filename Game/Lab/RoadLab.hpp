#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Draw.hpp"
#include "../GridMap.hpp"
#include "../Map.hpp"
#include "../Math.hpp"
#include "../Memory.hpp"
#include "../Path.hpp"
#include "../Road.hpp"
#include "../Type.hpp"

F32 PathLineWidth = 0.25f * LaneWidth;
V4 ValidColor     = MakeColor(0.0f, 0.8f, 0.0f);
V4 InvalidColor   = MakeColor(0.8f, 0.0f, 0.0f);
V4 HighlightColor = MakeColor(0.8f, 1.0f, 1.0f);
V4 PathColor      = MakeColor(0.0f, 0.8f, 0.8f);
V4 PathColor2     = MakeColor(0.0f, 1.0f, 1.0f);

enum RoadLabMode 
{
	RoadPlacingMode,
	JunctionPlacingMode,
	RoadPathBuildingMode,
	SidewalkPathBuildingMode
};

#define RoadLabMaxJunctionRowN 10
#define RoadLabMaxJunctionColN 10
#define RoadLabMaxJunctionN (RoadLabMaxJunctionRowN * RoadLabMaxJunctionColN)
#define RoadLabMaxRoadN 100
#define RoadLabMaxPathNodeN 100
#define RoadLabMemArenaSize (1 * MegaByte)

struct RoadLabState 
{
	Camera camera;
	Canvas canvas;
	B32 running;
	B32 isPreviewOn;

	Road roadPreview;
	Junction junctionPreview;

	Road roads[RoadLabMaxRoadN];
	Junction junctions[RoadLabMaxJunctionN];
	Map map;

	PathNode pathNodes[RoadLabMaxPathNodeN];
	PathPool pathPool;

	MemArena memArena;
	Junction* pathJunction1;
	I32 pathJunctionCorner1;
	Junction* pathJunction2;
	I32 PathJunctionCorner2;
	PathNode* firstPathNode;

	RoadLabMode labMode;

	B32 isCameraMoved;
	V2 cameraMoveDragPoint;
};
static RoadLabState gRoadLabState;

static void func RoadLabResize(RoadLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 50.0f;
}

static void func RoadLabBlit(Canvas* canvas, HDC context, RECT rect)
{
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	Bitmap bitmap = canvas->bitmap;
	BITMAPINFO bitmapInfo = GetBitmapInfo(&bitmap);
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
				  0, 0, width, height,
				  bitmap.memory,
				  &bitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static B32 func CanJunctionBePlacedAtPoint(Map* map, V2 point)
{
	B32 valid = true;
	for (I32 i = 0; i < map->junctionN; ++i) 
	{
		Junction* junction = map->junctions + i;
		F32 distance = Distance(point, junction->position);
		if (distance < MinimumJunctionDistance) 
		{
			valid = false;
			break;
		}
	}
	return valid;
}

static void func HighlightJunctionCorner(Canvas* canvas, Junction* junction, I32 cornerIndex, V4 color)
{
	F32 radius = LaneWidth * 0.25f;
	V2 corner = GetJunctionCorner(junction, cornerIndex);
	F32 left   = corner.x - radius;
	F32 right  = corner.x + radius;
	F32 top    = corner.y - radius;
	F32 bottom = corner.y + radius;
	DrawRectLRTB(canvas, left, right, top, bottom, color);
}

static void func RoadLabUpdate(RoadLabState* labState, V2 mouse)
{
	if (labState->isCameraMoved) 
	{
		V2 mousePositionDifference = (labState->cameraMoveDragPoint - mouse);
		Camera* camera = &labState->camera;
		camera->center = (camera->center + mousePositionDifference);
	}

	Map* map = &labState->map;
	Canvas* canvas = &labState->canvas;
	V4 black = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, black);

	F32 seconds = 0.1f;
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		UpdateTrafficLights(&map->junctions[i], seconds);
	}

	for (I32 i = 0; i < map->roadN; ++i)
	{
		DrawRoadSidewalk(canvas, &map->roads[i]);
	}
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		DrawJunctionSidewalk(canvas, &map->junctions[i]);
	}

	for (I32 i = 0; i < map->roadN; ++i)
	{
		DrawRoad(canvas, &map->roads[i]);
	}

	for (I32 i = 0; i < map->junctionN; ++i)
	{
		DrawJunction(canvas, &map->junctions[i]);
	}

	if (labState->labMode == RoadPlacingMode) 
	{
		Junction* junctionAtMouse = GetJunctionAtPoint(map, mouse);
		if (junctionAtMouse)
		{
			HighlightJunction(canvas, junctionAtMouse, HighlightColor);
		}

		if (labState->isPreviewOn) 
		{
			Road* roadPreview = &labState->roadPreview;
			Junction* junction2 = junctionAtMouse;
			if (junction2 && junction2 != roadPreview->junction1) 
			{
				roadPreview->endPoint2 = junction2->position;
				HighlightRoad(canvas, roadPreview, ValidColor);
			} 
			else 
			{
				roadPreview->endPoint2 = mouse;
				HighlightRoad(canvas, roadPreview, InvalidColor);
			}
		}
	} 
	else if (labState->labMode == JunctionPlacingMode) 
	{
		Junction* junctionPreview = &labState->junctionPreview;
		B32 valid = CanJunctionBePlacedAtPoint(map, mouse);
		junctionPreview->position = mouse;
		if (valid)
		{
			DrawJunctionPlaceholder(canvas, junctionPreview, ValidColor);
		}
		else
		{
			DrawJunctionPlaceholder(canvas, junctionPreview, InvalidColor);
		}
	} 
	else if (labState->labMode == RoadPathBuildingMode) 
	{
		Junction* junctionAtMouse = GetJunctionAtPoint(&labState->map, mouse);
		if (junctionAtMouse)
		{
			HighlightJunction(canvas, junctionAtMouse, HighlightColor);
		}

		if (labState->pathJunction1) 
		{
			HighlightJunction(canvas, labState->pathJunction1, PathColor2);
			if (junctionAtMouse) 
			{
				if (junctionAtMouse == labState->pathJunction1) 
				{
					labState->firstPathNode = 0;
				} 
				else if (junctionAtMouse == labState->pathJunction2) 
				{
				} 
				else 
				{
					labState->pathJunction2 = junctionAtMouse;
					ResetPathPool(&labState->pathPool);
					labState->firstPathNode = ConnectElems(
						&labState->map,
						GetJunctionElem(labState->pathJunction1),
						GetJunctionElem(labState->pathJunction2),
						&labState->memArena,
						&labState->pathPool
					);
				}
			} 
			else 
			{
				labState->pathJunction2 = 0;
				labState->firstPathNode = 0;
			}

			if (labState->pathJunction2 && labState->pathJunction2 != labState->pathJunction1) 
			{
				HighlightJunction(canvas, labState->pathJunction2, PathColor2);
				if (labState->firstPathNode)
				{
					DrawBezierPath(canvas, labState->firstPathNode, PathColor, PathLineWidth);
				}
			}
		}
	} 
	else if (labState->labMode == SidewalkPathBuildingMode) 
	{
		Junction* junctionAtMouse = GetJunctionAtPoint(map, mouse);
		if (junctionAtMouse && junctionAtMouse->roadN > 0) 
		{
			I32 cornerIndex = GetClosestJunctionCornerIndex(junctionAtMouse, mouse);
			HighlightJunctionCorner(canvas, junctionAtMouse, cornerIndex, HighlightColor);
		}

		Junction* junction1 = labState->pathJunction1;
		if (junction1 && junction1->roadN > 0) 
		{
			I32 corner1 = labState->pathJunctionCorner1;
			HighlightJunctionCorner(canvas, junction1, corner1, PathColor);
			if (junctionAtMouse) 
			{
				labState->pathJunction2 = junctionAtMouse;
				I32 cornerAtMouse = GetClosestJunctionCornerIndex(junctionAtMouse, mouse);
				ResetPathPool(&labState->pathPool);
				labState->firstPathNode = ConnectPedestrianElems(
					&labState->map,
					GetJunctionSidewalkElem(junction1),
					corner1,
					GetJunctionSidewalkElem(junctionAtMouse),
					cornerAtMouse,
					&labState->memArena,
					&labState->pathPool
				);
			} 
			else 
			{
				labState->pathJunction2 = 0;
				labState->firstPathNode = 0;
			}
		} 
		else 
		{
			labState->pathJunction2 = 0;
			labState->firstPathNode = 0;
		}
		
		if (labState->firstPathNode)
			DrawPath(canvas, labState->firstPathNode, PathColor, PathLineWidth);
	}

	for (I32 i = 0; i < map->junctionN; ++i)
	{
		DrawTrafficLights(canvas, &map->junctions[i]);
	}
}

static void func UpdateJunction(Junction* junction)
{
	CalculateStopDistances(junction);
	InitTrafficLights(junction);
	for (I32 i = 0; i < junction->roadN; ++i)
	{
		GenerateCrossing(junction->roads[i]);
	}
}

static void func RoadLabClick(RoadLabState* labState, V2 mouse)
{
	Map* map = &labState->map;
	if (labState->labMode == RoadPlacingMode) 
	{
		Road* roadPreview = &labState->roadPreview;
		if (labState->isPreviewOn) 
		{
			Junction* junction1 = roadPreview->junction1;
			Junction* junction2 = GetJunctionAtPoint(map, mouse);
			if (junction2 && junction1 != junction2) 
			{
				Assert(map->roadN < RoadLabMaxRoadN);
				Road* road = map->roads + map->roadN;
				ConnectJunctions(junction1, junction2, road);
				map->roadN++;
				labState->isPreviewOn = false;
				UpdateJunction(junction1);
				UpdateJunction(junction2);
				GenerateCrossing(road);
			}
		} 
		else 
		{
			Junction* junction1 = GetJunctionAtPoint(map, mouse);
			if (junction1) 
			{
				roadPreview->junction1 = junction1;
				roadPreview->endPoint1 = junction1->position;
				roadPreview->junction2 = 0;
				roadPreview->endPoint2 = mouse;
				labState->isPreviewOn = true;
			}
		}
	} 
	else if (labState->labMode == JunctionPlacingMode) 
	{
		Junction junctionPreview = labState->junctionPreview;
		B32 valid = CanJunctionBePlacedAtPoint(map, junctionPreview.position);
		if (valid) 
		{
			Assert(map->junctionN < RoadLabMaxJunctionN);
			map->junctions[map->junctionN] = junctionPreview;
			map->junctionN++;
		}
	} 
	else if (labState->labMode == RoadPathBuildingMode) 
	{
		labState->pathJunction1 = GetJunctionAtPoint(&labState->map, mouse);
		labState->firstPathNode = 0;
		ResetPathPool(&labState->pathPool);
	} 
	else if (labState->labMode == SidewalkPathBuildingMode) 
	{
		Junction* junction = GetJunctionAtPoint(&labState->map, mouse);
		labState->pathJunction1 = junction;
		if (junction && junction->roadN > 0)
		{
			labState->pathJunctionCorner1 = GetClosestJunctionCornerIndex(junction, mouse);
		}
		labState->pathJunction2 = 0;
	}
}

static LRESULT CALLBACK func RoadLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	RoadLabState* labState = &gRoadLabState;
	LRESULT result = 0;

	switch (message) 
	{
		case WM_SIZE: 
		{
			RECT clientRect;
			GetClientRect(window, &clientRect);
			I32 width = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			RoadLabResize(labState, width, height);
			break;
		}
		case WM_PAINT: 
		{
			PAINTSTRUCT paint;
			HDC context = BeginPaint(window, &paint);

			RECT clientRect;
			GetClientRect(window, &clientRect);

			RoadLabBlit(&labState->canvas, context, clientRect);

			EndPaint(window, &paint);
			break;
		}
		case WM_KEYUP: 
		{
			WPARAM keyCode = wparam;

			switch (keyCode) 
			{
				case '1':
					labState->labMode = RoadPlacingMode;
					labState->isPreviewOn = false;
					break;
				case '2':
					labState->labMode = JunctionPlacingMode;
					break;
				case 'P':
					labState->labMode = RoadPathBuildingMode;
					ResetPathPool(&labState->pathPool);
					break;
				case 'O':
					labState->labMode = SidewalkPathBuildingMode;
					ResetPathPool(&labState->pathPool);
					break;
				case 'G':
					Map* map = &labState->map;
					I32 junctionRowN = RoadLabMaxJunctionRowN;
					I32 junctionColN = RoadLabMaxJunctionColN;
					I32 roadN = RoadLabMaxRoadN;
					MemArena* tmpArena = &labState->memArena;
					GenerateGridMap(map, junctionRowN, junctionColN, roadN, tmpArena);
					break;
			}
			break;
		}
		case WM_LBUTTONDOWN: 
		{
			V2 mouse = GetMousePosition(&labState->camera, window);
			RoadLabClick(labState, mouse);
			break;
		}
		case WM_RBUTTONDOWN: 
		{
			labState->isCameraMoved = true;
			labState->cameraMoveDragPoint = GetMousePosition(&labState->camera, window);
			break;
		}
		case WM_RBUTTONUP: 
		{
			labState->isCameraMoved = false;
			break;
		}
		case WM_MOUSEWHEEL: 
		{
			I16 wheelDeltaParam = GET_WHEEL_DELTA_WPARAM(wparam);
			if (wheelDeltaParam > 0)
				labState->camera.unitInPixels *= 1.10f;
			else if (wheelDeltaParam < 0)
				labState->camera.unitInPixels /= 1.10f;
			break;
		}
		case WM_SETCURSOR: 
		{
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}
		case WM_DESTROY: 
		{
			labState->running = false;
			break;
		}
		case WM_CLOSE: 
		{
			labState->running = false;
			break;
		}
		default: 
		{
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}

	return result;
}

static void func RoadLabInit(RoadLabState* labState, I32 windowWidth, I32 windowHeight)
{
	RoadLabResize(labState, windowWidth, windowHeight);
	labState->labMode = JunctionPlacingMode;
	Map* map = &labState->map;
	map->roads = labState->roads;
	map->junctions = labState->junctions;
	map->buildings = 0;

	map->roadN = 0;
	map->junctionN = 0;
	map->buildingN = 0;

	PathPool* pathPool = &labState->pathPool;
	pathPool->maxNodeCount = RoadLabMaxPathNodeN;
	pathPool->nodes = labState->pathNodes;
	pathPool->firstFreeNode = 0;
	pathPool->nodeCount = 0;

	Camera* camera = &labState->camera;
	camera->unitInPixels = 10.0f;

	labState->memArena = CreateMemArena(RoadLabMemArenaSize);
}

static void func RoadLab(HINSTANCE instance)
{
	RoadLabState* labState = &gRoadLabState;

	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = RoadLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "RoadLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"RoadLab",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instance,
		0
	);
	Assert(window != 0);

	RECT rect = {};
	GetClientRect(window, &rect);
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	RoadLabInit(labState, width, height);

	MSG message = {};

	labState->running = true;
	while (labState->running) 
	{
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}
		
		V2 mouse = GetMousePosition(&labState->camera, window);
		RoadLabUpdate(labState, mouse);
  
		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		RoadLabBlit(&labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}