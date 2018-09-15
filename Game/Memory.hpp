#pragma once

#include <stdlib.h>

#include "Debug.hpp"
#include "Type.hpp"

#define Byte (8)
#define KiloByte (1024 * Byte)
#define MegaByte (1024 * KiloByte)
#define GigaByte (1024 * MegaByte)

struct MemArena 
{
	I8* baseAddress;
	U32 usedSize;
	U32 maxSize;
};

MemArena CreateMemArena(U32 maxSize);
void ArenaReset(MemArena* arena);
void* ArenaAlloc(MemArena* arena, U32 size);
U32 GetArenaSize(MemArena* arena);
void SetArenaSize(MemArena* arena, U32 size);
void ArenaPopTo(MemArena* arena, void* address);

#define ArenaPushType(arena, type) ((type*)ArenaAlloc((arena), sizeof(type)))
#define ArenaPushArray(arena, type, size) ((type*)ArenaAlloc((arena), (size) * sizeof(type)))
#define ArenaPush(arena, type, variable) {\
			type* address = (type*)ArenaAlloc(arena, sizeof(variable)); \
			*address = (variable); }
#define ArenaPushVar(arena, variable) {  \
			I8* address = (I8*)ArenaAlloc((arena), sizeof(variable)); \
		    memcpy((address), &(variable), sizeof(variable)); }