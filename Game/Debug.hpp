#pragma once

#define DEBUG_MODE

#include <Windows.h>
#include <stdio.h>

#include "Type.hpp"

#define func

#ifdef DEBUG_MODE
	#define Assert(value) {if (!(value)) DebugBreak ();}
	#define Verify(call) {if (!(call)) DebugBreak ();}
#else
	#define Assert(value)
    #define Verify(call) (call)
#endif

I32 func Log (const I8* format, ...)
{
    static I8 buffer[1024];
    va_list args;
    va_start (args, format);
    _vsnprintf_s (buffer, sizeof (buffer), format, args);
    va_end (args);
    OutputDebugStringA (buffer);
    return 0;
}