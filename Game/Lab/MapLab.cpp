#include "MapLab.hpp"

#include "../Debug.hpp"
#include "../Gridmap.hpp"
#include "../Map.hpp"
#include "../Memory.hpp"
#include "../Renderer.hpp"
#include "../Texture.hpp"

float MapTileSide = 50.0f;

#define TileBitmapWidth 1024  
#define TileBitmapHeight 1024
#define MaxCachedTileN 16
#define MaxArenaSize (1 * MegaByte)
#define MaxTmpArenaSize (1 * MegaByte)

#define MaxJunctionRowN 10
#define MaxJunctionColN 10
#define MaxJunctionN (MaxJunctionRowN * MaxJunctionColN)
#define MaxRoadN 100

struct TileIndex {
	int row;
	int col;
};

struct CachedTile {
	TileIndex index;
	Bitmap bitmap;
};

struct MapLabState {
	Camera camera;
	Renderer renderer;
	bool running;

	float mapLeft;
	float mapRight;
	float mapTop;
	float mapBottom;

	int tileRowN;
	int tileColN;

	float visibleWidth;
	float visibleHeight;

	Point playerPosition;
	Point playerVelocity;

	CachedTile cachedTiles[MaxCachedTileN];
	int cachedTileN;

	MemArena arena;
	MemArena tmpArena;
	Map map;

	Texture roadTexture;
};
MapLabState gMapLabState;

static void MapLabResize(MapLabState* labState, int width, int height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Renderer* renderer = &labState->renderer;
	ResizeBitmap(&renderer->bitmap, width, height);
	renderer->camera = camera;
	camera->unitInPixels = 1.0f;
}

static void MapLabBlit(Renderer renderer, HDC context, RECT rect)
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

static LRESULT CALLBACK MapLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	MapLabState* labState = &gMapLabState;
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.bottom;
			MapLabResize(labState, width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			MapLabBlit(labState->renderer, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_SETCURSOR: {
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}

		case WM_KEYDOWN: {
			WPARAM keyCode = wparam;
			switch(keyCode) {
				case 'W': {
					labState->playerVelocity.y = -1.0f;
					break;
				}
				case 'S': {
					labState->playerVelocity.y = 1.0f;
					break;
				}
				case 'A': {
					labState->playerVelocity.x = -1.0f;
					break;
				}
				case 'D': {
					labState->playerVelocity.x = 1.0f;
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
			short wheelDeltaParam = GET_WHEEL_DELTA_WPARAM(wparam);
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

static void MapLabInit(MapLabState* labState, int windowWidth, int windowHeight)
{
	labState->running = true;

	MapLabResize(labState, windowWidth, windowHeight);
	Camera* camera = &labState->camera;
	float left   = CameraLeftSide(camera);
	float right  = CameraRightSide(camera);
	float top    = CameraTopSide(camera);
	float bottom = CameraBottomSide(camera);

	labState->mapLeft   = left   + (right  - left)   * 0.05f;
	labState->mapRight  = right  + (left   - right)  * 0.05f;
	labState->mapTop    = top    + (bottom - top)    * 0.05f;
	labState->mapBottom = bottom + (top    - bottom) * 0.05f;

	labState->tileRowN = 10;
	labState->tileColN = 20;

	labState->visibleWidth  = (right - left) * 0.1f;
	labState->visibleHeight = (bottom - top) * 0.1f;

	labState->playerPosition.x = (left + right) * 0.5f;
	labState->playerPosition.y = (top + bottom) * 0.5f;

	labState->arena = CreateMemArena(MaxArenaSize);
	labState->tmpArena = CreateMemArena(MaxTmpArenaSize);

	Map* map = &labState->map;
	map->junctions = ArenaPushArray(&labState->arena, Junction, MaxJunctionN);
	map->roads = ArenaPushArray(&labState->arena, Road, MaxRoadN);
	GenerateGridMap(map, MaxJunctionRowN, MaxJunctionColN, MaxRoadN, &labState->tmpArena);

	labState->roadTexture = RandomGreyTexture(8, 100, 150);
}

static TileIndex GetTileIndexContainingPoint(MapLabState* labState, Point point)
{
	TileIndex tileIndex = {};
	tileIndex.col = Floor((point.x - labState->mapLeft) / MapTileSide);
	tileIndex.row = Floor((point.y - labState->mapTop) / MapTileSide);
	return tileIndex;
}

static bool IsTileIndexValid(MapLabState* labState, TileIndex tileIndex)
{
	bool isValid = true;
	if (tileIndex.row < 0 || tileIndex.row >= labState->tileRowN)
		isValid = false;
	else if (tileIndex.col < 0 || tileIndex.col >= labState->tileColN)
		isValid = false;
	return isValid;
}

static Point GetTileCenter(MapLabState* labState, TileIndex tileIndex)
{
	Assert(IsTileIndexValid(labState, tileIndex));
	Point tileCenter = {};
	tileCenter.x = labState->mapLeft + (tileIndex.col * MapTileSide)  + (0.5f * MapTileSide);
	tileCenter.y = labState->mapTop  + (tileIndex.row * MapTileSide) + (0.5f * MapTileSide);
	return tileCenter;
}

static void GenerateTileBitmap(MapLabState* labState, TileIndex tileIndex, Bitmap* bitmap)
{
	Camera camera = {};
	camera.center = GetTileCenter(labState, tileIndex);
	camera.screenPixelSize.x = (float)bitmap->width;
	camera.screenPixelSize.y = (float)bitmap->height;
	camera.unitInPixels = (float)bitmap->width / MapTileSide;

	Renderer renderer = {};
	renderer.bitmap = *bitmap;
	renderer.camera = &camera;
	DrawGroundElems(renderer, &labState->map);
	Map* map = &labState->map;
	for (int i = 0; i < map->roadN; ++i)
		DrawTexturedRoad(renderer, &map->roads[i], labState->roadTexture);
}

static bool IsTileCached(MapLabState* labState, TileIndex tileIndex)
{
	bool isCached = false;
	for (int i = 0; i < labState->cachedTileN; ++i) {
		TileIndex cachedTileIndex = labState->cachedTiles[i].index;
		if (cachedTileIndex.row == tileIndex.row && cachedTileIndex.col == tileIndex.col) {
			isCached = true;
			break;
		}
	}
	return isCached;
}

static CachedTile* GetFarthestCachedTile(MapLabState* labState, Point point)
{
	Assert(labState->cachedTileN > 0);
	CachedTile* cachedTile = 0;
	float maxDistance = 0.0f;
	for (int i = 0; i < labState->cachedTileN; ++i) {
		TileIndex tileIndex = labState->cachedTiles[i].index;
		Point tileCenter = GetTileCenter(labState, tileIndex);
		float distance = Distance(tileCenter, point);
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
	GenerateTileBitmap(labState, tileIndex, &cachedTile->bitmap);
}

static Bitmap* GetCachedTileBitmap(MapLabState* labState, TileIndex tileIndex)
{
	Bitmap* bitmap = 0;
	for (int i = 0; i < labState->cachedTileN; ++i) {
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
	Renderer renderer = labState->renderer;
	float tileLeft   = labState->mapLeft + (tileIndex.col * MapTileSide);
	float tileRight  = tileLeft + MapTileSide;
	float tileTop    = labState->mapTop + (tileIndex.row * MapTileSide);
	float tileBottom = tileTop + MapTileSide;

	if (!IsTileCached(labState, tileIndex))
		CacheTile(labState, tileIndex);
		
	Bitmap* tileBitmap = GetCachedTileBitmap(labState, tileIndex);
	DrawStretchedBitmap(renderer, tileBitmap, tileLeft, tileRight, tileTop, tileBottom);
}

static void MapLabUpdate(MapLabState* labState, Point mousePosition)
{
	Renderer renderer = labState->renderer;
	Color backgroundColor = GetColor(0.0f, 0.0f, 0.0f);
	ClearScreen(renderer, backgroundColor);

	Color tileGridColor = GetColor(1.0f, 1.0f, 0.0f);
	Color visibleLineColor = GetColor(1.0f, 0.0f, 1.0f);

	labState->playerPosition = PointSum(labState->playerPosition, labState->playerVelocity);
	renderer.camera->center = labState->playerPosition;

	Point playerPosition = labState->playerPosition;
	float visibleLeft   = playerPosition.x - (labState->visibleWidth * 0.5f);
	float visibleRight  = playerPosition.x + (labState->visibleWidth * 0.5f);
	float visibleTop    = playerPosition.y - (labState->visibleHeight * 0.5f);
	float visibleBottom = playerPosition.y + (labState->visibleHeight * 0.5f);
	Point visibleTopLeft     = Point{visibleLeft, visibleTop};
	Point visibleBottomRight = Point{visibleRight, visibleBottom};
	TileIndex topLeftTileIndex     = GetTileIndexContainingPoint(labState, visibleTopLeft);
	TileIndex bottomRightTileIndex = GetTileIndexContainingPoint(labState, visibleBottomRight);
	for (int row = topLeftTileIndex.row; row <= bottomRightTileIndex.row; ++row) {
		for (int col = topLeftTileIndex.col; col <= bottomRightTileIndex.col; ++col) {
			TileIndex tileIndex = TileIndex{row, col};
			if (IsTileIndexValid(labState, tileIndex))
				DrawTile(labState, tileIndex);
		}
	}

	Color playerColor = GetColor(1.0f, 0.0f, 0.0f);
	float playerRadius = 1.0f;
	float playerLeft   = playerPosition.x - playerRadius;
	float playerRight  = playerPosition.x + playerRadius;
	float playerTop    = playerPosition.y - playerRadius;
	float playerBottom = playerPosition.y + playerRadius;
	DrawRect(renderer, playerTop, playerLeft, playerBottom, playerRight, playerColor);
}

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
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	MapLabInit(labState, width, height);

	MSG message = {};
	while (labState->running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		Point mousePosition = GetMousePosition(labState->camera, window);
		MapLabUpdate(labState, mousePosition);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		MapLabBlit(gMapLabState.renderer, context, rect);
		ReleaseDC(window, context);
	}
}

// TODO: Replace color codes with world texture!
	// TODO: Draw textured bitmap or apply texture afterward?
// TODO: Renderer refactoring
// TODO: Rename to MapTileLab?
// TODO: Pull out common Lab functions?
// TODO: Multithreading?