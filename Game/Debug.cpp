#include "Debug.h"

int Log(const char* format, ...)
{
    static char buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);
    OutputDebugStringA(buffer);
    return 0;
}