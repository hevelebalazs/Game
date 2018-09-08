#include "CombatLab.hpp"

#include "../Draw.hpp"

struct CombatLabState {
	Camera camera;
	Canvas canvas;
	B32 running;

	V2 playerPosition;
	V2 playerVelocity;
};
CombatLabState gCombatLabState;

static void CombatLabResize(CombatLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 20.0f;
}

static void CombatLabBlit(Canvas canvas, HDC context, RECT rect)
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

static LRESULT CALLBACK CombatLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	CombatLabState* labState = &gCombatLabState;
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			CombatLabResize(labState, width, height);
			break;
		}
		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			CombatLabBlit(labState->canvas, context, clientRect);

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
			switch (keyCode) {
				case 'W': {
					labState->playerVelocity.y = -1.0f;
					break;
				}
				case 'S': {
					labState->playerVelocity.y = +1.0f;
					break;
				}
				case 'A': {
					labState->playerVelocity.x = -1.0f;
					break;
				}
				case 'D': {
					labState->playerVelocity.x = +1.0f;
					break;
				}
			}
			break;
		}
		case WM_KEYUP: {
			WPARAM keyCode = wparam;
			switch (keyCode) {
				case 'W': 
				case 'S': {
					labState->playerVelocity.y = 0.0f;
					break;
				}
				case 'A':
				case 'D':{
					labState->playerVelocity.x = 0.0f;
					break;
				}
			}
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

static void CombatLabInit(CombatLabState* labState, I32 windowWidth, I32 windowHeight)
{
	labState->running = true;

	CombatLabResize(labState, windowWidth, windowHeight);
}

static void CombatLabUpdate(CombatLabState* labState)
{
	F32 seconds = 0.05f;
	labState->playerPosition = labState->playerPosition + seconds * labState->playerVelocity;

	Canvas canvas = labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 gridColor = MakeColor(0.3f, 0.3f, 0.0f);
	F32 gridDistance = 10.0f;

	Camera* camera = &labState->camera;
	camera->center = labState->playerPosition;
	F32 left   = CameraLeftSide(camera);
	F32 right  = CameraRightSide(camera);
	F32 top    = CameraTopSide(camera);
	F32 bottom = CameraBottomSide(camera);

	I32 firstCol = Floor(left / gridDistance);
	I32 lastCol  = Floor(right / gridDistance);
	for (I32 col = firstCol; col <= lastCol; ++col) {
		F32 x = (col * gridDistance);
		V2 point1 = MakePoint(x, top);
		V2 point2 = MakePoint(x, bottom);
		Bresenham(canvas, point1, point2, gridColor);
	}

	I32 firstRow = Floor(top / gridDistance);
	I32 lastRow  = Floor(bottom /gridDistance);
	for (I32 row = firstRow; row <= lastRow; ++row) {
		F32 y = (row * gridDistance);
		V2 point1 = MakePoint(left, y);
		V2 point2 = MakePoint(right, y);
		Bresenham(canvas, point1, point2, gridColor);
	}

	F32 playerSide = 1.0f;
	V4 playerColor = MakeColor(0.5f, 0.0f, 0.0f);
	F32 playerX	= labState->playerPosition.x;
	F32 playerY = labState->playerPosition.y;
	F32 playerLeft   = playerX - playerSide;
	F32 playerRight  = playerX + playerSide;
	F32 playerTop    = playerY - playerSide;
	F32 playerBottom = playerY + playerSide;
	DrawRect(canvas, playerLeft, playerRight, playerTop, playerBottom, playerColor);
}

void CombatLab(HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = CombatLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "CombatLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"CombatLab",
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

	CombatLabState* labState = &gCombatLabState;

	RECT rect = {};
	GetClientRect(window, &rect);
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	CombatLabInit(labState, width, height);

	MSG message = {};
	while (labState->running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		CombatLabUpdate(labState);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		CombatLabBlit(labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}

// TODO: Draw a circle instead of a square?
// TODO: Better physics?