#include "RoadLab.h"
#include "../Renderer.h"
#include "../Road.h"
#include "../Point.h"

float MinimumJunctionDistance = 10.0f * LaneWidth;
Color ValidColor     = Color{0.0f, 0.8f, 0.0f};
Color InvalidColor   = Color{0.8f, 0.0f, 0.0f};
Color HighlightColor = Color{0.8f, 1.0f, 1.0f};

Camera gCamera;
Renderer gRenderer;
bool gRunning;
bool gPreview;

Road gRoadPreview;
Road gRoads[100];
int gRoadN;

Junction gJunctionPreview;
Junction gJunctions[100];
int gJunctionN;

enum PlaceMode {
	PlaceRoad,
	PlaceJunction
};
PlaceMode gPlaceMode = PlaceJunction;

Point MousePosition(Camera camera, HWND window)
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
	gCamera.altitude = 200.0f;
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

Junction* GetJunctionAtPoint(Point point)
{
	Junction* result = 0;
	for (int i = 0; i < gJunctionN; ++i) {
		Junction* junction = gJunctions + i;
		if (IsPointOnJunction(point, *junction)) {
			result = junction;
			break;
		}
	}
	return result;
}

bool IsValidJunctionPoint(Point point)
{
	bool valid = true;
	for (int i = 0; i < gJunctionN; ++i) {
		Junction* junction = gJunctions + i;
		float distance = Distance(point, junction->position);
		if (distance < MinimumJunctionDistance) {
			valid = false;
			break;
		}
	}
	return valid;
}

void RoadLabUpdate(Renderer renderer, Point mouse)
{
	Color black = Color{0.0f, 0.0f, 0.0f};
	ClearScreen(renderer, black);

	for (int i = 0; i < gRoadN; ++i) {
		Road road = gRoads[i];
		DrawRoad(renderer, road);
	}

	Junction* highlightedJunction = GetJunctionAtPoint(mouse);
	for (int i = 0; i < gJunctionN; ++i) {
		Junction* junction = gJunctions + i;
		if (junction == highlightedJunction)
			HighlightJunction(renderer, *junction, HighlightColor);
		else
			DrawJunction(renderer, *junction);
	}

	if (gPlaceMode == PlaceRoad) {
		if (gPreview) {
			Junction* junction2 = GetJunctionAtPoint(mouse);
			if (junction2 && junction2 != gRoadPreview.junction1) {
				gRoadPreview.endPoint2 = junction2->position;
				HighlightRoad(gRenderer, gRoadPreview, ValidColor);
			}
			else {
				gRoadPreview.endPoint2 = mouse;
				HighlightRoad(gRenderer, gRoadPreview, InvalidColor);
			}
		}
	} else if (gPlaceMode == PlaceJunction) {
		bool valid = IsValidJunctionPoint(mouse);
		gJunctionPreview.position = mouse;
		if (valid)
			HighlightJunction(gRenderer, gJunctionPreview, ValidColor);
		else
			HighlightJunction(gRenderer, gJunctionPreview, InvalidColor);
	}
}

void RoadLabClick(Point mouse)
{
	if (gPlaceMode == PlaceRoad) {
		if (gPreview) {
			Junction* junction2 = GetJunctionAtPoint(mouse);
			if (junction2 && gRoadPreview.junction1 != junction2) {
				Assert(gRoadN < 100);
				gRoadPreview.junction2 = junction2;
				gRoadPreview.endPoint2 = junction2->position;
				gRoads[gRoadN] = gRoadPreview;
				gRoadN++;
				gPreview = false;
			}
		} else {
			Junction* junction1 = GetJunctionAtPoint(mouse);
			if (junction1) {
				gRoadPreview.junction1 = junction1;
				gRoadPreview.endPoint1 = junction1->position;
				gRoadPreview.junction2 = 0;
				gRoadPreview.endPoint2 = mouse;
				gPreview = true;
			}
		}
	} else if (gPlaceMode == PlaceJunction) {
		bool valid = IsValidJunctionPoint(gJunctionPreview.position);
		if (valid) {
			Assert(gJunctionN < 100);
			gJunctions[gJunctionN] = gJunctionPreview;
			gJunctionN++;
		}
	}
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

		case WM_KEYUP: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case '1': {
					gPlaceMode = PlaceRoad;
					gPreview = false;
					break;
				};
				case '2': {
					gPlaceMode = PlaceJunction;
					break;
				};
			}
			break;
		}

		case WM_LBUTTONDOWN: {
			Point mouse = MousePosition(gCamera, window);
			RoadLabClick(mouse);
			break;
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

// TODO: Connect junctions to roads!
// TODO: Do not allow roads to cross each other or junctions!
// TODO: Add road textures!
// TODO: Realistic sizes in meters!
// TODO: Separate all the Windows-specific stuff!