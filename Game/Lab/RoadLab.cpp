#include "RoadLab.hpp"

#include "../GridMap.hpp"
#include "../Map.hpp"
#include "../Memory.hpp"
#include "../Path.hpp"
#include "../Point.hpp"
#include "../Renderer.hpp"
#include "../Road.hpp"

float PathLineWidth = 0.25f * LaneWidth;
Color ValidColor     = Color{0.0f, 0.8f, 0.0f};
Color InvalidColor   = Color{0.8f, 0.0f, 0.0f};
Color HighlightColor = Color{0.8f, 1.0f, 1.0f};
Color PathColor      = Color{0.0f, 0.8f, 0.8f};

enum RoadLabMode {
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

struct RoadLabState {
	Camera camera;
	Renderer renderer;
	bool running;
	bool isPreviewOn;

	Road roadPreview;
	Junction junctionPreview;

	Road roads[RoadLabMaxRoadN];
	Junction junctions[RoadLabMaxJunctionN];
	Map map;

	PathNode pathNodes[RoadLabMaxPathNodeN];
	PathPool pathPool;

	MemArena memArena;
	Junction* pathJunction1;
	int pathJunctionCorner1;
	Junction* pathJunction2;
	int PathJunctionCorner2;
	PathNode* firstPathNode;

	RoadLabMode labMode;

	bool isCameraMoved;
	Point cameraMoveDragPoint;
};
RoadLabState gRoadLabState;

static Point GetMousePosition(Camera camera, HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	Point point = {};
	point.x = (float)cursorPoint.x;
	point.y = (float)cursorPoint.y;

	point = PixelToUnit(camera, point);

	return point;
}

static void RoadLabResize(RoadLabState* labState, int width, int height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Renderer* renderer = &labState->renderer;
	ResizeBitmap(&renderer->bitmap, width, height);
	renderer->camera = camera;
	camera->unitInPixels = 50.0f;
}

static void RoadLabBlit(Renderer renderer, HDC context, RECT rect)
{
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	Bitmap bitmap = renderer.bitmap;
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
				  0, 0, width, height,
				  bitmap.memory,
				  &bitmap.info,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static bool CanJunctionBePlacedAtPoint(Map* map, Point point)
{
	bool valid = true;
	for (int i = 0; i < map->junctionN; ++i) {
		Junction* junction = map->junctions + i;
		float distance = Distance(point, junction->position);
		if (distance < MinimumJunctionDistance) {
			valid = false;
			break;
		}
	}
	return valid;
}

static void HighlightJunctionCorner(Renderer renderer, Junction* junction, int cornerIndex, Color color)
{
	float radius = LaneWidth * 0.25f;
	Point corner = GetJunctionCorner(junction, cornerIndex);
	float left   = corner.x - radius;
	float right  = corner.x + radius;
	float top    = corner.y - radius;
	float bottom = corner.y + radius;
	DrawRect(renderer, top, left, bottom, right, color);
}

static void RoadLabUpdate(RoadLabState* labState, Point mouse)
{
	if (labState->isCameraMoved) {
		Point mousePositionDifference = PointDiff(labState->cameraMoveDragPoint, mouse);
		Camera* camera = &labState->camera;
		camera->center = PointSum(camera->center, mousePositionDifference);
	}

	Map* map = &labState->map;
	Renderer renderer = labState->renderer;
	Color black = Color{0.0f, 0.0f, 0.0f};
	ClearScreen(renderer, black);

	float seconds = 0.1f;
	for (int i = 0; i < map->junctionN; ++i)
		UpdateTrafficLights(&map->junctions[i], seconds);

	for (int i = 0; i < map->roadN; ++i)
		DrawRoadSidewalk(renderer, &map->roads[i]);
	for (int i = 0; i < map->junctionN; ++i)
		DrawJunctionSidewalk(renderer, &map->junctions[i]);

	for (int i = 0; i < map->roadN; ++i)
		DrawRoad(renderer, &map->roads[i]);

	for (int i = 0; i < map->junctionN; ++i)
		DrawJunction(renderer, &map->junctions[i]);

	if (labState->labMode == RoadPlacingMode) {
		Junction* junctionAtMouse = GetJunctionAtPoint(map, mouse);
		if (junctionAtMouse)
			HighlightJunction(renderer, junctionAtMouse, HighlightColor);
		if (labState->isPreviewOn) {
			Road* roadPreview = &labState->roadPreview;
			Junction* junction2 = junctionAtMouse;
			if (junction2 && junction2 != roadPreview->junction1) {
				roadPreview->endPoint2 = junction2->position;
				HighlightRoad(renderer, roadPreview, ValidColor);
			} else {
				roadPreview->endPoint2 = mouse;
				HighlightRoad(renderer, roadPreview, InvalidColor);
			}
		}
	} else if (labState->labMode == JunctionPlacingMode) {
		Junction* junctionPreview = &labState->junctionPreview;
		bool valid = CanJunctionBePlacedAtPoint(map, mouse);
		junctionPreview->position = mouse;
		if (valid)
			DrawJunctionPlaceholder(renderer, junctionPreview, ValidColor);
		else
			DrawJunctionPlaceholder(renderer, junctionPreview, InvalidColor);
	} else if (labState->labMode == RoadPathBuildingMode) {
		Junction* junctionAtMouse = GetJunctionAtPoint(&labState->map, mouse);
		if (junctionAtMouse)
			HighlightJunction(renderer, junctionAtMouse, HighlightColor);
		if (labState->pathJunction1) {
			HighlightJunction(renderer, labState->pathJunction1, PathColor);
			if (junctionAtMouse) {
				if (junctionAtMouse == labState->pathJunction1) {
					labState->firstPathNode = 0;
				} else if (junctionAtMouse == labState->pathJunction2) {
				} else {
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
			} else {
				labState->pathJunction2 = 0;
				labState->firstPathNode = 0;
			}

			if (labState->pathJunction2 && labState->pathJunction2 != labState->pathJunction1) {
				HighlightJunction(renderer, labState->pathJunction2, PathColor);
				if (labState->firstPathNode)
					DrawBezierPath(renderer, labState->firstPathNode, PathColor, PathLineWidth);
			}
		}
	} else if (labState->labMode == SidewalkPathBuildingMode) {
		Junction* junctionAtMouse = GetJunctionAtPoint(map, mouse);
		if (junctionAtMouse && junctionAtMouse->roadN > 0) {
			int cornerIndex = GetClosestJunctionCornerIndex(junctionAtMouse, mouse);
			HighlightJunctionCorner(renderer, junctionAtMouse, cornerIndex, HighlightColor);
		}
		Junction* junction1 = labState->pathJunction1;
		if (junction1 && junction1->roadN > 0) {
			int corner1 = labState->pathJunctionCorner1;
			HighlightJunctionCorner(renderer, junction1, corner1, PathColor);
			if (junctionAtMouse) {
				labState->pathJunction2 = junctionAtMouse;
				int cornerAtMouse = GetClosestJunctionCornerIndex(junctionAtMouse, mouse);
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
			} else {
				labState->pathJunction2 = 0;
				labState->firstPathNode = 0;
			}
		} else {
			labState->pathJunction2 = 0;
			labState->firstPathNode = 0;
		}
		
		if (labState->firstPathNode)
			DrawPath(renderer, labState->firstPathNode, PathColor, PathLineWidth);
	}

	for (int i = 0; i < map->junctionN; ++i)
		DrawTrafficLights(renderer, &map->junctions[i]);
}

static void UpdateJunction(Junction* junction)
{
	CalculateStopDistances(junction);
	InitTrafficLights(junction);
	for (int i = 0; i < junction->roadN; ++i)
		GenerateCrossing(junction->roads[i]);
}

static void RoadLabClick(RoadLabState* labState, Point mouse)
{
	Map* map = &labState->map;
	if (labState->labMode == RoadPlacingMode) {
		Road* roadPreview = &labState->roadPreview;
		if (labState->isPreviewOn) {
			Junction* junction1 = roadPreview->junction1;
			Junction* junction2 = GetJunctionAtPoint(map, mouse);
			if (junction2 && junction1 != junction2) {
				Assert(map->roadN < RoadLabMaxRoadN);
				Road* road = map->roads + map->roadN;
				ConnectJunctions(junction1, junction2, road);
				map->roadN++;
				labState->isPreviewOn = false;
				UpdateJunction(junction1);
				UpdateJunction(junction2);
				GenerateCrossing(road);
			}
		} else {
			Junction* junction1 = GetJunctionAtPoint(map, mouse);
			if (junction1) {
				roadPreview->junction1 = junction1;
				roadPreview->endPoint1 = junction1->position;
				roadPreview->junction2 = 0;
				roadPreview->endPoint2 = mouse;
				labState->isPreviewOn = true;
			}
		}
	} else if (labState->labMode == JunctionPlacingMode) {
		Junction junctionPreview = labState->junctionPreview;
		bool valid = CanJunctionBePlacedAtPoint(map, junctionPreview.position);
		if (valid) {
			Assert(map->junctionN < RoadLabMaxJunctionN);
			map->junctions[map->junctionN] = junctionPreview;
			map->junctionN++;
		}
	} else if (labState->labMode == RoadPathBuildingMode) {
		labState->pathJunction1 = GetJunctionAtPoint(&labState->map, mouse);
		labState->firstPathNode = 0;
		ResetPathPool(&labState->pathPool);
	} else if (labState->labMode == SidewalkPathBuildingMode) {
		Junction* junction = GetJunctionAtPoint(&labState->map, mouse);
		labState->pathJunction1 = junction;
		if (junction && junction->roadN > 0)
			labState->pathJunctionCorner1 = GetClosestJunctionCornerIndex(junction, mouse);
		labState->pathJunction2 = 0;
	}
}

static LRESULT CALLBACK RoadLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	RoadLabState* labState = &gRoadLabState;
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			RECT clientRect;
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			RoadLabResize(labState, width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC context = BeginPaint(window, &paint);

			RECT clientRect;
			GetClientRect(window, &clientRect);

			RoadLabBlit(labState->renderer, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_KEYUP: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case '1': {
					labState->labMode = RoadPlacingMode;
					labState->isPreviewOn = false;
					break;
				};
				case '2': {
					labState->labMode = JunctionPlacingMode;
					break;
				};
				case 'P': {
					labState->labMode = RoadPathBuildingMode;
					ResetPathPool(&labState->pathPool);
					break;
				}
				case 'O': {
					labState->labMode = SidewalkPathBuildingMode;
					ResetPathPool(&labState->pathPool);
					break;
				}
				case 'G': {
					Map* map = &labState->map;
					int junctionRowN = RoadLabMaxJunctionRowN;
					int junctionColN = RoadLabMaxJunctionColN;
					int roadN = RoadLabMaxRoadN;
					MemArena* tmpArena = &labState->memArena;
					GenerateGridMap(map, junctionRowN, junctionColN, roadN, tmpArena);
					break;
				}
			}
			break;
		}

		case WM_LBUTTONDOWN: {
			Point mouse = GetMousePosition(labState->camera, window);
			RoadLabClick(labState, mouse);
			break;
		}

		case WM_RBUTTONDOWN: {
			labState->isCameraMoved = true;
			labState->cameraMoveDragPoint = GetMousePosition(labState->camera, window);
			break;
		};

		case WM_RBUTTONUP: {
			labState->isCameraMoved = false;
			break;
		}

		case WM_MOUSEWHEEL: {
			short wheelDeltaParam = GET_WHEEL_DELTA_WPARAM(wparam);
			if (wheelDeltaParam > 0)
				labState->camera.unitInPixels *= 1.10f;
			else if (wheelDeltaParam < 0)
				labState->camera.unitInPixels /= 1.10f;
			break;
		}

		case WM_SETCURSOR: {
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}

		case WM_DESTROY: {
			labState->running = false;
			break;
		}

		case WM_CLOSE: {
			labState->running = false;
			break;
		}

		default: {
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}

	return result;
}

static void RoadLabInit(RoadLabState* labState, int windowWidth, int windowHeight)
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

void RoadLab(HINSTANCE instance)
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
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	RoadLabInit(labState, width, height);

	MSG message = {};

	labState->running = true;
	while (labState->running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}
		
		Point mouse = GetMousePosition(labState->camera, window);
		RoadLabUpdate(labState, mouse);
  
		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		RoadLabBlit(labState->renderer, context, rect);
		ReleaseDC(window, context);
	}
}