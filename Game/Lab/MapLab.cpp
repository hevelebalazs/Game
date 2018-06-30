#include "MapLab.hpp"

#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Gridmap.hpp"
#include "../Map.hpp"
#include "../Memory.hpp"
#include "../Texture.hpp"
#include "../Type.hpp"

F32 MapTileSide = 50.0f;

#define TileBitmapWidth 1024
#define TileBitmapHeight 1024
#define MaxCachedTileN 64
#define MaxArenaSize (1 * MegaByte)
#define MaxTmpArenaSize (1 * MegaByte)

#define MaxJunctionRowN 20
#define MaxJunctionColN 20
#define MaxJunctionN (MaxJunctionRowN * MaxJunctionColN)
#define MaxRoadN 400

#define MaxTileGenerateWorkListN MaxCachedTileN
#define TileGenerateWorkThreadN 5

struct TileIndex {
	I32 row;
	I32 col;
};

struct CachedTile {
	TileIndex index;
	Bitmap bitmap;
};

struct MapLabState;
struct TileGenerateWork {
	MapLabState* labState;
	TileIndex tileIndex;
	Bitmap* bitmap;
};

struct TileGenerateWorkList {
	TileGenerateWork works[MaxTileGenerateWorkListN];
	volatile I32 workN;
	volatile I32 firstWorkToDo;
	HANDLE semaphore;
};

struct MapLabState {
	Camera camera;
	Canvas canvas;
	B32 running;

	F32 mapLeft;
	F32 mapRight;
	F32 mapTop;
	F32 mapBottom;

	I32 tileRowN;
	I32 tileColN;

	F32 visibleWidth;
	F32 visibleHeight;

	V2 playerPosition;
	V2 playerVelocity;

	CachedTile cachedTiles[MaxCachedTileN];
	I32 cachedTileN;

	TileGenerateWorkList workList;

	MemArena arena;
	MemArena tmpArena;
	Map map;

	Texture grassTexture;
	Texture roadTexture;
	Texture sidewalkTexture;
	Texture stripeTexture;
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

static B32 IsTileIndexValid(MapLabState* labState, TileIndex tileIndex)
{
	B32 isValid = true;
	if (tileIndex.row < 0 || tileIndex.row >= labState->tileRowN)
		isValid = false;
	else if (tileIndex.col < 0 || tileIndex.col >= labState->tileColN)
		isValid = false;
	return isValid;
}

static V2 GetTileCenter(MapLabState* labState, TileIndex tileIndex)
{
	Assert(IsTileIndexValid(labState, tileIndex));
	V2 tileCenter = {};
	tileCenter.x = labState->mapLeft + (tileIndex.col * MapTileSide)  + (0.5f * MapTileSide);
	tileCenter.y = labState->mapTop  + (tileIndex.row * MapTileSide) + (0.5f * MapTileSide);
	return tileCenter;
}

static void GenerateTileBitmap(MapLabState* labState, TileIndex tileIndex, Bitmap* bitmap)
{
	Camera camera = {};
	camera.center = GetTileCenter(labState, tileIndex);
	camera.screenPixelSize.x = (F32)bitmap->width;
	camera.screenPixelSize.y = (F32)bitmap->height;
	camera.unitInPixels = (F32)bitmap->width / MapTileSide;

	Map* map = &labState->map;
	Canvas canvas = {};
	canvas.bitmap = *bitmap;
	canvas.camera = &camera;
	DrawTexturedGroundElems(canvas, map, labState->grassTexture, labState->roadTexture, labState->sidewalkTexture, labState->stripeTexture);
}

static DWORD WINAPI TileGenerateWorkProc(LPVOID parameter)
{
	TileGenerateWorkList* workList = (TileGenerateWorkList*)parameter;
	while (1) {
		if (workList->firstWorkToDo < workList->workN) {
			I32 workIndex = (I32)InterlockedIncrement((volatile U64*)&workList->firstWorkToDo) - 1;
			TileGenerateWork work = workList->works[workIndex];
			GenerateTileBitmap(work.labState, work.tileIndex, work.bitmap);
		} else {
			WaitForSingleObjectEx(workList->semaphore, INFINITE, FALSE);
		}
	}
}

static void PushTileGenerateWork(TileGenerateWorkList* workList, TileGenerateWork work)
{
	Assert(workList->workN < MaxTileGenerateWorkListN);
	workList->works[workList->workN] = work;
	workList->workN++;
	ReleaseSemaphore(workList->semaphore, 1, 0);
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

	labState->mapLeft   = -((MaxJunctionColN + 2) * JunctionGridDistance) * 0.5f;
	labState->mapRight  = +((MaxJunctionColN + 2) * JunctionGridDistance) * 0.5f;
	labState->mapTop    = -((MaxJunctionRowN + 2) * JunctionGridDistance) * 0.5f;
	labState->mapBottom = +((MaxJunctionRowN + 2) * JunctionGridDistance) * 0.5f;

	labState->tileRowN = Floor((labState->mapRight - labState->mapLeft) / (MapTileSide));
	labState->tileColN = Floor((labState->mapBottom - labState->mapTop) / (MapTileSide));

	labState->workList.semaphore = CreateSemaphore(0, 0, MaxTileGenerateWorkListN, 0);
	for (F32 i = 0; i < TileGenerateWorkThreadN; ++i)
		CreateThread(0, 0, TileGenerateWorkProc, &labState->workList, 0, 0);

	labState->visibleWidth  = 300.0f;
	labState->visibleHeight = 150.0f;

	labState->playerPosition.x = (labState->mapLeft + labState->mapRight) * 0.5f;
	labState->playerPosition.y = (labState->mapTop + labState->mapBottom) * 0.5f;

	labState->arena = CreateMemArena(MaxArenaSize);
	labState->tmpArena = CreateMemArena(MaxTmpArenaSize);

	Map* map = &labState->map;
	map->junctions = ArenaPushArray(&labState->arena, Junction, MaxJunctionN);
	map->roads = ArenaPushArray(&labState->arena, Road, MaxRoadN);
	GenerateGridMap(map, MaxJunctionRowN, MaxJunctionColN, MaxRoadN, &labState->tmpArena);

	labState->grassTexture = GrassTexture(8, &labState->tmpArena);
	labState->roadTexture = RandomGreyTexture(8, 100, 150);
	labState->sidewalkTexture = RandomGreyTexture(8, 80, 100);
	labState->stripeTexture = RandomGreyTexture(8, 200, 255);
}

static TileIndex GetTileIndexContainingPoint(MapLabState* labState, V2 point)
{
	TileIndex tileIndex = {};
	tileIndex.col = Floor((point.x - labState->mapLeft) / MapTileSide);
	tileIndex.row = Floor((point.y - labState->mapTop) / MapTileSide);
	return tileIndex;
}

static B32 IsTileCached(MapLabState* labState, TileIndex tileIndex)
{
	B32 isCached = false;
	for (I32 i = 0; i < labState->cachedTileN; ++i) {
		TileIndex cachedTileIndex = labState->cachedTiles[i].index;
		if (cachedTileIndex.row == tileIndex.row && cachedTileIndex.col == tileIndex.col) {
			isCached = true;
			break;
		}
	}
	return isCached;
}

static CachedTile* GetFarthestCachedTile(MapLabState* labState, V2 point)
{
	Assert(labState->cachedTileN > 0);
	CachedTile* cachedTile = 0;
	F32 maxDistance = 0.0f;
	for (I32 i = 0; i < labState->cachedTileN; ++i) {
		TileIndex tileIndex = labState->cachedTiles[i].index;
		V2 tileCenter = GetTileCenter(labState, tileIndex);
		F32 distance = Distance(tileCenter, point);
		if (distance >= maxDistance) {
			maxDistance = distance;
			cachedTile = &labState->cachedTiles[i];
		}
	}
	return cachedTile;
	Assert(cachedTile != 0);
}

static void CacheTile(MapLabState* labState, TileIndex tileIndex)
{
	Assert(!IsTileCached(labState, tileIndex));
	CachedTile* cachedTile = 0;
	if (labState->cachedTileN < MaxCachedTileN) {
		cachedTile = labState->cachedTiles + labState->cachedTileN;
		ResizeBitmap(&cachedTile->bitmap, TileBitmapWidth, TileBitmapHeight);
		labState->cachedTileN++;
	} else {
		cachedTile = GetFarthestCachedTile(labState, labState->playerPosition);
	}
	Assert(cachedTile != 0);
	cachedTile->index = tileIndex;
	TileGenerateWork work = {};
	work.labState = labState;
	work.tileIndex = tileIndex;
	work.bitmap = &cachedTile->bitmap;
	PushTileGenerateWork(&labState->workList, work);
}

static Bitmap* GetCachedTileBitmap(MapLabState* labState, TileIndex tileIndex)
{
	Bitmap* bitmap = 0;
	for (I32 i = 0; i < labState->cachedTileN; ++i) {
		CachedTile* cachedTile = labState->cachedTiles + i;
		if (cachedTile->index.row == tileIndex.row && cachedTile->index.col == tileIndex.col) {
			bitmap = &cachedTile->bitmap;
			break;
		}
	}
	Assert(bitmap != 0);
	return bitmap;
}

static void DrawTile(MapLabState* labState, TileIndex tileIndex)
{
	Canvas canvas = labState->canvas;
	F32 tileLeft   = labState->mapLeft + (tileIndex.col * MapTileSide);
	F32 tileRight  = tileLeft + MapTileSide;
	F32 tileTop    = labState->mapTop + (tileIndex.row * MapTileSide);
	F32 tileBottom = tileTop + MapTileSide;

	if (!IsTileCached(labState, tileIndex))
		CacheTile(labState, tileIndex);
		
	Bitmap* tileBitmap = GetCachedTileBitmap(labState, tileIndex);
	DrawStretchedBitmap(canvas, tileBitmap, tileLeft, tileRight, tileTop, tileBottom);
}

static void MapLabUpdate(MapLabState* labState, V2 mousePosition)
{
	Canvas canvas = labState->canvas;
	V4 backgroundColor = GetColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 tileGridColor = GetColor(1.0f, 1.0f, 0.0f);
	V4 visibleLineColor = GetColor(1.0f, 0.0f, 1.0f);

	labState->playerPosition = (labState->playerPosition + labState->playerVelocity);
	canvas.camera->center = labState->playerPosition;

	TileGenerateWorkList* workList = &labState->workList;
	if (workList->workN == workList->firstWorkToDo) {
		workList->workN = 0;
		workList->firstWorkToDo = 0;
	}

	V2 playerPosition = labState->playerPosition;
	F32 visibleLeft   = playerPosition.x - (labState->visibleWidth * 0.5f);
	F32 visibleRight  = playerPosition.x + (labState->visibleWidth * 0.5f);
	F32 visibleTop    = playerPosition.y - (labState->visibleHeight * 0.5f);
	F32 visibleBottom = playerPosition.y + (labState->visibleHeight * 0.5f);
	V2 visibleTopLeft     = MakePoint(visibleLeft, visibleTop);
	V2 visibleBottomRight = MakePoint(visibleRight, visibleBottom);
	TileIndex topLeftTileIndex     = GetTileIndexContainingPoint(labState, visibleTopLeft);
	TileIndex bottomRightTileIndex = GetTileIndexContainingPoint(labState, visibleBottomRight);
	for (I32 row = topLeftTileIndex.row; row <= bottomRightTileIndex.row; ++row) {
		for (I32 col = topLeftTileIndex.col; col <= bottomRightTileIndex.col; ++col) {
			TileIndex tileIndex = TileIndex{row, col};
			if (IsTileIndexValid(labState, tileIndex))
				DrawTile(labState, tileIndex);
		}
	}

	V4 playerColor = GetColor(1.0f, 0.0f, 0.0f);
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

// TODO: Add map tiles to the game!
// TODO: GroundElemTextures struct?
// TODO: Pull out common Lab functions?
// TODO: Render to screen multithreaded?