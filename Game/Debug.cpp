#include "Debug.hpp"

I32 Log(const I8* format, ...)
{
    static I8 buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);
    OutputDebugStringA(buffer);
    return 0;
}