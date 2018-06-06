#pragma once

// #define DEBUG_MODE

#include <Windows.h>
#include <stdio.h>

#ifdef DEBUG_MODE
	#define Assert(value) {if (!(value)) DebugBreak();}
	#define InvalidCodePath DebugBreak()
#else
	#define Assert(value)
	#define InvalidCodePath
#endif
	

int Log(const char* format, ...);