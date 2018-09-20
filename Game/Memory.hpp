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

static MemArena CreateMemArena(U32 maxSize)
{
	MemArena result = {};
	result.baseAddress = new I8[maxSize];
	result.maxSize = maxSize;
	result.usedSize = 0;

	return result;
}

static void ArenaReset(MemArena* arena)
{
	arena->usedSize = 0;
}

static void* ArenaAlloc(MemArena* arena, U32 size)
{
	Assert(arena->usedSize + size <= arena->maxSize);
	if (arena->usedSize + size <= arena->maxSize) 
	{
		I8* result = arena->baseAddress + arena->usedSize;
		arena->usedSize += size;
		return result;
	}
	else
	{
		return 0;
	}
}

static U32 GetArenaSize(MemArena* arena)
{
	return arena->usedSize;
}

static void SetArenaSize(MemArena* arena, U32 size)
{
	arena->usedSize = size;
}

// TODO: only pop if the address is smaller than the current one?
static void ArenaPopTo(MemArena* arena, void* address)
{
	arena->usedSize = (U32)((I8*)address - arena->baseAddress);
}


#define ArenaPushType(arena, type) ((type*)ArenaAlloc((arena), sizeof(type)))
#define ArenaPushArray(arena, type, size) ((type*)ArenaAlloc((arena), (size) * sizeof(type)))
#define ArenaPush(arena, type, variable) {\
			type* address = (type*)ArenaAlloc(arena, sizeof(variable)); \
			*address = (variable); }
#define ArenaPushVar(arena, variable) {  \
			I8* address = (I8*)ArenaAlloc((arena), sizeof(variable)); \
		    memcpy((address), &(variable), sizeof(variable)); }