#include "Memory.hpp"

MemArena CreateMemArena(U32 maxSize)
{
	MemArena result = {};
	result.baseAddress = new I8[maxSize];
	result.maxSize = maxSize;
	result.usedSize = 0;

	return result;
}

void ArenaReset(MemArena* arena)
{
	arena->usedSize = 0;
}

void* ArenaAlloc(MemArena* arena, U32 size)
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

U32 GetArenaSize(MemArena* arena)
{
	return arena->usedSize;
}

void SetArenaSize(MemArena* arena, U32 size)
{
	arena->usedSize = size;
}

// TODO: only pop if the address is smaller than the current one?
void ArenaPopTo(MemArena* arena, void* address)
{
	arena->usedSize = (U32)((I8*)address - arena->baseAddress);
}
