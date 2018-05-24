#include "RoadLab.h"

#include "../Map.h"
#include "../Memory.h"
#include "../Path.h"
#include "../Point.h"
#include "../Renderer.h"
#include "../Road.h"

float MinimumJunctionDistance = 10.0f * LaneWidth;
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

#define RoadLabMaxJunctionN 100
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

struct JunctionGridPosition {
	int row;
	int col;
};

enum JunctionGridDirection {
	JunctionGridUp,
	JunctionGridRight,
	JunctionGridDown,
	JunctionGridLeft,
	JunctionGridDirectionN
};

static JunctionGridDirection GetRandomJunctionGridDirection()
{
	JunctionGridDirection result = (JunctionGridDirection)IntRandom(0, JunctionGridDirectionN - 1);
	return result;
}

static JunctionGridPosition GetNextJunctionGridPosition(JunctionGridPosition startPosition, JunctionGridDirection direction)
{
	JunctionGridPosition endPosition = startPosition;

	if (direction == JunctionGridUp)
		endPosition.row--;
	else if (direction == JunctionGridDown)
		endPosition.row++;

	if (direction == JunctionGridLeft)
		endPosition.col--;
	else if (direction == JunctionGridRight)
		endPosition.col++;

	return endPosition;
}

static bool AreJunctionsConnected(Junction* junction1, Junction* junction2)
{
	bool areConnected = false;
	for (int i = 0; i < junction1->roadN; ++i) {
		Road* road = junction1->roads[i];
		if (road->junction1 == junction1 && road->junction2 == junction2)
			areConnected = true;
		else if (road->junction1 == junction2 && road->junction2 == junction1)
			areConnected = true;

		if (areConnected)
			break;
	}
	return areConnected;
}

static void ReindexJunction(Junction* oldJunction, Junction* newJunction)
{
	for (int i = 0; i < oldJunction->roadN; ++i) {
		Road* road = oldJunction->roads[i];
		if (road->junction1 == oldJunction)
			road->junction1 = newJunction;
		else if (road->junction2 == oldJunction)
			road->junction2 = newJunction;
		else
			InvalidCodePath;
	}
}

static void GenerateMap(RoadLabState* labState)
{
	InitRandom();
	Map* map = &labState->map;
	map->roadCount = 0;
	map->junctionCount = 0;
	map->roadCount = 0;
	
	Camera* camera = &labState->camera;
	float left = 0.0f;
	float top  = 0.0f;
	int rowN = 10;
	int colN = 10;

	float junctionX = left;
	float junctionY = top;

	MemArena* arena = &labState->memArena;
	int junctionN = rowN * colN;
	map->junctionCount = junctionN;
	Junction* junctionGrid = map->junctions;

	int roadN = rowN * colN;
	map->roadCount = roadN;
	Road* roads = map->roads;

	float JunctionGridDistance = MinimumJunctionDistance * 1.5f;
	float MaxJunctionDistanceFromOrigin = MinimumJunctionDistance * 0.5f;
	for (int row = 0; row < rowN; ++row) {
		for (int col = 0; col < colN; ++col) {
			Junction* junction = junctionGrid + (row * colN) + col;
			junction->position.x = junctionX + RandomBetween(-MaxJunctionDistanceFromOrigin, MaxJunctionDistanceFromOrigin);
			junction->position.y = junctionY + RandomBetween(-MaxJunctionDistanceFromOrigin, MaxJunctionDistanceFromOrigin);
			junction->roadN = 0;
			junctionX += JunctionGridDistance;
		}
		junctionY += JunctionGridDistance;
		junctionX = left + RandomBetween(-MaxJunctionDistanceFromOrigin, MaxJunctionDistanceFromOrigin);
	}

	JunctionGridPosition* connectedJunctionPositions = ArenaPushArray(arena, JunctionGridPosition, rowN * colN);
	int connectedJunctionN = 0;

	JunctionGridPosition startPosition = {};
	startPosition.row = IntRandom(0, rowN - 1);
	startPosition.col = IntRandom(0, colN - 1);
	connectedJunctionPositions[connectedJunctionN] = startPosition;
	connectedJunctionN++;

	int createdRoadN = 0;
	while (1) {
		int positionIndex = IntRandom(0, connectedJunctionN - 1);
		JunctionGridPosition junctionPosition = connectedJunctionPositions[positionIndex];
		JunctionGridDirection direction = GetRandomJunctionGridDirection();
		while (1) {
			Junction* junction = junctionGrid + (colN * junctionPosition.row) + junctionPosition.col;
			JunctionGridPosition nextPosition = GetNextJunctionGridPosition(junctionPosition, direction);
			Assert(junctionPosition.row != nextPosition.row || junctionPosition.col != nextPosition.col);
			if (nextPosition.row < 0 || nextPosition.row >= rowN)
				break;
			if (nextPosition.col < 0 || nextPosition.col >= colN)
				break;

			Junction* nextJunction = junctionGrid + (colN * nextPosition.row) + nextPosition.col;
			if (AreJunctionsConnected(junction, nextJunction))
				break;

			bool isNextJunctionConnected = (nextJunction->roadN > 0);
			if (!isNextJunctionConnected) {
				connectedJunctionPositions[connectedJunctionN] = nextPosition;
				connectedJunctionN++;
			}

			ConnectJunctions(junction, nextJunction, roads + createdRoadN);
			createdRoadN++;
			if (createdRoadN == roadN)
				break;

			junction = nextJunction;
			junctionPosition = nextPosition;

			if (isNextJunctionConnected) {
				if (IntRandom(0, 2) == 0)
					break;
			} else {
				if (IntRandom(0, 5) == 0)
					break;
			}
		}

		if (createdRoadN == roadN)
			break;
	}

	ArenaPopTo(arena, connectedJunctionPositions);

	// NOTE: remove empty junctions
	int newJunctionN = 0;
	for (int i = 0; i < junctionN; ++i) {
		Junction* junction = junctionGrid + i;
		if (junction->roadN > 0) {
			Junction* newJunction = junctionGrid + newJunctionN;
			*newJunction = *junction;
			ReindexJunction(junction, newJunction);
			newJunctionN++;
		}
	}

	junctionN = newJunctionN;
	map->junctionCount = newJunctionN;

	for (int i = 0; i < junctionN; ++i) {
		Junction* junction = junctionGrid + i;
		if (junction->roadN > 0) {
			CalculateStopDistances(junction);
			InitTrafficLights(junction);
		}
	}

	for (int i = 0; i < roadN; ++i) {
		Road* road = roads + i;
		GenerateCrossing(road);
	}
}

static Point GetMousePosition(Camera camera, HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	Point point = {};
	point.x = (float)cursorPoint.x;
	point.y = (float)cursorPoint.y;

	point = PixelToCoord(camera, point);

	return point;
}

static void RoadLabResize(RoadLabState* labState, int width, int height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Renderer* renderer = &labState->renderer;
	ResizeBitmap(&renderer->bitmap, width, height);
	renderer->camera = camera;
	camera->altitude = 200.0f;
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
	for (int i = 0; i < map->junctionCount; ++i) {
		Junction* junction = map->junctions + i;
		float distance = Distance(point, junction->position);
		if (distance < MinimumJunctionDistance) {
			valid = false;
			break;
		}
	}
	return valid;
}

static void DrawJunctionPlaceholder(Renderer renderer, Junction* junction, Color color)
{
	Point position = junction->position;
	float side = LaneWidth + SidewalkWidth;
	DrawRect(
		renderer,
		position.y - side, position.x - side,
		position.y + side, position.x + side,
		color
	);

	side = LaneWidth;
	DrawRect(
		renderer,
		position.y - side, position.x - side,
		position.y + side, position.x + side,
		color
	);
}

static void SafeDrawJunctionSidewalk(Renderer renderer, Junction* junction)
{
	if (junction->roadN >= 1)
		DrawJunctionSidewalk(renderer, junction);
}

static void SafeDrawJunction(Renderer renderer, Junction* junction)
{
	if (junction->roadN >= 1)
		DrawJunction(renderer, junction);
	else
		DrawJunctionPlaceholder(renderer, junction, RoadColor);
}

static void SafeHighlightJunction(Renderer renderer, Junction* junction, Color color)
{
	if (junction->roadN >= 1)
		HighlightJunction(renderer, junction, color);
	else
		DrawJunctionPlaceholder(renderer, junction, color);
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
	for (int i = 0; i < map->junctionCount; ++i)
		UpdateTrafficLights(map->junctions + i, seconds);

	for (int i = 0; i < map->roadCount; ++i)
		DrawRoadSidewalk(renderer, map->roads + i);
	for (int i = 0; i < map->junctionCount; ++i)
		SafeDrawJunctionSidewalk(renderer, map->junctions + i);

	for (int i = 0; i < map->roadCount; ++i)
		DrawRoad(renderer, map->roads + i);

	for (int i = 0; i < map->junctionCount; ++i)
		SafeDrawJunction(renderer, map->junctions + i);

	if (labState->labMode == RoadPlacingMode) {
		Junction* junctionAtMouse = GetJunctionAtPoint(map, mouse);
		if (junctionAtMouse)
			SafeHighlightJunction(renderer, junctionAtMouse, HighlightColor);
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
			SafeHighlightJunction(renderer, junctionAtMouse, HighlightColor);
		if (labState->pathJunction1) {
			SafeHighlightJunction(renderer, labState->pathJunction1, PathColor);
			if (junctionAtMouse) {
				if (junctionAtMouse == labState->pathJunction1) {
					labState->firstPathNode = 0;
				} else if (junctionAtMouse == labState->pathJunction2) {
				} else {
					labState->pathJunction2 = junctionAtMouse;
					ResetPathPool(&labState->pathPool);
					labState->firstPathNode = ConnectElems(
						&labState->map,
						JunctionElem(labState->pathJunction1),
						JunctionElem(labState->pathJunction2),
						&labState->memArena,
						&labState->pathPool
					);
				}
			} else {
				labState->pathJunction2 = 0;
				labState->firstPathNode = 0;
			}

			if (labState->pathJunction2 && labState->pathJunction2 != labState->pathJunction1) {
				SafeHighlightJunction(renderer, labState->pathJunction2, PathColor);
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
					JunctionSidewalkElem(junction1),
					corner1,
					JunctionSidewalkElem(junctionAtMouse),
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

	for (int i = 0; i < map->junctionCount; ++i)
		DrawTrafficLights(renderer, map->junctions + i);
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
				Assert(map->roadCount < RoadLabMaxRoadN);
				Road* road = map->roads + map->roadCount;
				ConnectJunctions(junction1, junction2, road);
				map->roadCount++;
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
			Assert(map->junctionCount < RoadLabMaxJunctionN);
			map->junctions[map->junctionCount] = junctionPreview;
			map->junctionCount++;
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

	switch(message) {
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
					GenerateMap(labState);
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
			if (wheelDeltaParam > 0.0f)
				labState->camera.altitude /= 1.10f;
			else if (wheelDeltaParam < 0.0f)
				labState->camera.altitude *= 1.10f;
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

	map->roadCount = 0;
	map->junctionCount = 0;
	map->buildingCount = 0;

	map->width  = float(windowWidth);
	map->height = float(windowHeight);

	PathPool* pathPool = &labState->pathPool;
	pathPool->maxNodeCount = RoadLabMaxPathNodeN;
	pathPool->nodes = labState->pathNodes;
	pathPool->firstFreeNode = 0;
	pathPool->nodeCount = 0;

	Camera* camera = &labState->camera;
	camera->altitude = 500.0f;

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

	if (!RegisterClass(&winClass)) {
		OutputDebugStringA("Failed to register window class!\n");
		return;
	}

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

	if (!window) {
		OutputDebugStringA("Failed to create window!\n");
		return;
	}

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

// TODO: Remove "safe" junction functions?
// TODO: Use a memory arena everywhere!
// TODO: Realistic sizes in meters!
// TODO: Separate all the Windows-specific stuff!
// TODO: Merge changes to the game!
//   TODO: Update GridMap.cpp!
// TODO: Fix [R1] todos!
// NOTE: Textures are delayed to a later project!