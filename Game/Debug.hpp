#pragma once

#define DEBUG_MODE

#include <Windows.h>
#include <stdio.h>

#include "Type.hpp"

#ifdef DEBUG_MODE
	#define Assert(value) {if (!(value)) DebugBreak();}
	#define Verify(call) {if (!(call)) DebugBreak();}
	#define InvalidCodePath DebugBreak()
#else
	#define Assert(value)
    #define Verify(call) (call)
	#define InvalidCodePath
#endif
	

I32 Log(const I8* format, ...);