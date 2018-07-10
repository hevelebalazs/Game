#include "MapLab.hpp"

#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Gridmap.hpp"
#include "../Map.hpp"
#include "../Memory.hpp"
#include "../Texture.hpp"
#include "../Type.hpp"

#define MaxArenaSize (1 * MegaByte)
#define MaxTmpArenaSize (1 * MegaByte)

#define MaxJunctionRowN 20
#define MaxJunctionColN 20
#define MaxJunctionN (MaxJunctionRowN * MaxJunctionColN)
#define MaxRoadN 400

struct MapLabState;

struct MapLabState {
	Camera camera;
	Canvas canvas;
	B32 running;

	F32 visibleWidth;
	F32 visibleHeight;

	V2 playerPosition;
	V2 playerVelocity;

	MemArena arena;
	MemArena tmpArena;
	Map map;

	MapTextures mapTextures;
};
MapLabState gMapLabState;

static void MapLabResize(MapLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 1.0f;
}

static void MapLabBlit(Canvas canvas, HDC context, RECT rect)
{
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	Bitmap bitmap = canvas.bitmap;
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
		          0, 0, width, height,
		          bitmap.memory,
		          &bitmap.info,
		          DIB_RGB_COLORS,
		          SRCCOPY
	);
}

static LRESULT CALLBACK MapLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	MapLabState* labState = &gMapLabState;
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.bottom;
			MapLabResize(labState, width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			MapLabBlit(labState->canvas, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_SETCURSOR: {
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}

		case WM_KEYDOWN: {
			F32 speed = 0.3f;

			WPARAM keyCode = wparam;
			switch(keyCode) {
				case 'W': {
					labState->playerVelocity.y = -speed;
					break;
				}
				case 'S': {
					labState->playerVelocity.y = speed;
					break;
				}
				case 'A': {
					labState->playerVelocity.x = -speed;
					break;
				}
				case 'D': {
					labState->playerVelocity.x = speed;
					break;
				}
			}
			break;
		}

		case WM_KEYUP: {
			WPARAM keyCode = wparam;
			switch(keyCode) {
				case 'W':
				case 'S': {
					labState->playerVelocity.y = 0.0f;
					break;
				}
				case 'A':
				case 'D': {
					labState->playerVelocity.x = 0.0f;
					break;
				}
			}
			break;
		}

		case WM_MOUSEWHEEL: {
			I16 wheelDeltaParam = GET_WHEEL_DELTA_WPARAM(wparam);
			if (wheelDeltaParam > 0)
				labState->camera.unitInPixels *= 1.10f;
			else if (wheelDeltaParam < 0)
				labState->camera.unitInPixels /= 1.10f;
			break;
		}

		case WM_DESTROY:
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

static void MapLabInit(MapLabState* labState, I32 windowWidth, I32 windowHeight)
{
	labState->running = true;

	MapLabResize(labState, windowWidth, windowHeight);
	Camera* camera = &labState->camera;
	F32 left   = CameraLeftSide(camera);
	F32 right  = CameraRightSide(camera);
	F32 top    = CameraTopSide(camera);
	F32 bottom = CameraBottomSide(camera);

	labState->arena = CreateMemArena(MaxArenaSize);
	labState->tmpArena = CreateMemArena(MaxTmpArenaSize);

	Map* map = &labState->map;
	map->junctions = ArenaPushArray(&labState->arena, Junction, MaxJunctionN);
	map->roads = ArenaPushArray(&labState->arena, Road, MaxRoadN);
	GenerateGridMap(map, MaxJunctionRowN, MaxJunctionColN, MaxRoadN, &labState->tmpArena);

	{
	map->workList.semaphore = CreateSemaphore(0, 0, MaxGenerateMapTileWorkListN, 0);
	for (I32 i = 0; i < GenerateMapTileWorkThreadN; ++i)
		CreateThread(0, 0, GenerateMapTileWorkProc, &map->workList, 0, 0);
	}

	labState->visibleWidth  = 300.0f;
	labState->visibleHeight = 150.0f;

	labState->playerPosition.x = (map->left + map->right) * 0.5f;
	labState->playerPosition.y = (map->top + map->bottom) * 0.5f;

	GenerateMapTextures(&labState->mapTextures, &labState->tmpArena);
}

static void MapLabUpdate(MapLabState* labState, V2 mousePosition)
{
	Canvas canvas = labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 tileGridColor = MakeColor(1.0f, 1.0f, 0.0f);
	V4 visibleLineColor = MakeColor(1.0f, 0.0f, 1.0f);

	labState->playerPosition = (labState->playerPosition + labState->playerVelocity);
	canvas.camera->center = labState->playerPosition;
	
	Map* map = &labState->map;
	GenerateMapTileWorkList* workList = &map->workList;
	if (workList->workN == workList->firstWorkToDo) {
		workList->workN = 0;
		workList->firstWorkToDo = 0;
	}

	V2 playerPosition = labState->playerPosition;
	F32 left   = playerPosition.x - (labState->visibleWidth * 0.5f);
	F32 right  = playerPosition.x + (labState->visibleWidth * 0.5f);
	F32 top    = playerPosition.y - (labState->visibleHeight * 0.5f);
	F32 bottom = playerPosition.y + (labState->visibleHeight * 0.5f);
	DrawVisibleMapTiles(canvas, map, left, right, top, bottom, &labState->mapTextures);

	V4 playerColor = MakeColor(1.0f, 0.0f, 0.0f);
	F32 playerRadius = 1.0f;
	F32 playerLeft   = playerPosition.x - playerRadius;
	F32 playerRight  = playerPosition.x + playerRadius;
	F32 playerTop    = playerPosition.y - playerRadius;
	F32 playerBottom = playerPosition.y + playerRadius;
	DrawRect(canvas, playerLeft, playerRight, playerTop, playerBottom, playerColor);
}

static V2 GetMousePosition(Camera* camera, HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	V2 point = {};
	point.x = (F32)cursorPoint.x;
	point.y = (F32)cursorPoint.y;
	point = PixelToUnit(camera, point);

	return point;
}

void MapLab(HINSTANCE instance)
{
	MapLabState* labState = &gMapLabState;

	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = MapLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "MapLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"MapLab",
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
	MapLabInit(labState, width, height);

	MSG message = {};
	while (labState->running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		V2 mousePosition = GetMousePosition(&labState->camera, window);
		MapLabUpdate(labState, mousePosition);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		MapLabBlit(gMapLabState.canvas, context, rect);
		ReleaseDC(window, context);
	}
}

// [TODO: Render to screen multithreaded?]
	// TODO: Subpixel rendering to screen?
// TODO: Pull out common Lab functions?
// TODO: Eliminate GameAssets?