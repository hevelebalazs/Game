#include "RoadLab.h"
#include "../Renderer.h"
#include "../Road.h"
#include "../Point.h"

Camera gCamera;
Renderer gRenderer;
bool gRunning;
bool gClicked;
Road gRoad;

Point MousePosition(Camera camera, HWND window) {
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	Point point = {};
	point.x = (float)cursorPoint.x;
	point.y = (float)cursorPoint.y;

	point = PixelToCoord(camera, point);

	return point;
}

void ResizeCamera(Camera* camera, int width, int height)
{
	camera->screenSize = Point{(float)width, (float)height};
	camera->center = PointProd(0.5f, camera->screenSize);
}

void ResizeBitmap(Bitmap* bitmap, int width, int height)
{
	if (bitmap->memory)
		delete bitmap->memory;

	bitmap->width = width;
	bitmap->height = height;

	BITMAPINFOHEADER* header = &bitmap->info.bmiHeader;
	header->biSize = sizeof(*header);
	header->biWidth = bitmap->width;
	header->biHeight = -bitmap->height;
	header->biPlanes = 1;
	header->biBitCount = 32;
	header->biCompression = BI_RGB;

	int bytesPerPixel = 4;
	int bitmapMemorySize = (bitmap->width * bitmap->height) * bytesPerPixel;

	bitmap->memory = (void*)(new char[bitmapMemorySize]);
}

static void RoadLabResize(int width, int height)
{
	ResizeCamera(&gCamera, width, height);

	ResizeBitmap(&gRenderer.bitmap, width, height);
	gRenderer.camera = &gCamera;
	gCamera.altitude = 100.0f;
}

void RoadLabInit(int windowWidth, int windowHeight)
{
	RoadLabResize(windowWidth, windowHeight);
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

void RoadLabUpdate(Renderer renderer, Point mouse)
{
	Color white = Color{1.0f, 1.0f, 1.0f};
	ClearScreen(renderer, white);

	if (gClicked) {
		gRoad.endPoint2 = mouse;
		DrawRoad(gRenderer, gRoad);
	}
}

void RoadLabClick(Point mouse)
{
	gClicked = true;
	gRoad.endPoint1 = mouse;
}

LRESULT CALLBACK RoadLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	switch(message) {
		case WM_SIZE: {
			RECT clientRect;
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			RoadLabResize(width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC context = BeginPaint(window, &paint);

			RECT clientRect;
			GetClientRect(window, &clientRect);

			RoadLabBlit(gRenderer, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_LBUTTONDOWN: {
			Point mouse = MousePosition(gCamera, window);
			RoadLabClick(mouse);
		}

		case WM_SETCURSOR: {
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}

		case WM_DESTROY: {
			gRunning = false;
			break;
		}

		case WM_CLOSE: {
			gRunning = false;
			break;
		}

		default: {
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}

	return result;
}

void RoadLab(HINSTANCE instance)
{
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
	RoadLabInit(width, height);

	MSG message = {};

	gRunning = true;
	while (gRunning) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}
		
		Point mouse = MousePosition(gCamera, window);
		RoadLabUpdate(gRenderer, mouse);

		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);

		RoadLabBlit(gRenderer, context, rect);

		ReleaseDC(window, context);
	}
}

// TODO: Realistic sizes in meters!
// TODO: Separate all the Windows-specific stuff!