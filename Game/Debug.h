#pragma once

#include <Windows.h>
#include <stdio.h>

#define Assert(value) {if (!(value)) DebugBreak();}
#define InvalidCodePath DebugBreak()

int Log(const char* format, ...);