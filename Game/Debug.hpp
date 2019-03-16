#pragma once

#define DEBUG_MODE

#include <Windows.h>
#include <stdio.h>

#include "Type.hpp"

#define func

#ifdef DEBUG_MODE
	#define Assert(value) {if(!(value)) DebugBreak();}
	#define Verify(call) {if(!(call)) DebugBreak();}
#else
	#define Assert(value)
    #define Verify(call) (call)
#endif