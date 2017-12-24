#pragma once

#include <stdlib.h>

struct MemArena {
	char* baseAddress;
	unsigned int usedSize;
	unsigned int maxSize;
};

inline MemArena CreateMemArena(unsigned int maxSize) {
	MemArena result = {};
	result.baseAddress = new char[maxSize];
	result.maxSize = maxSize;
	result.usedSize = 0;

	return result;
}

inline void* ArenaAlloc(MemArena* arena, unsigned int size) {
	if (arena->usedSize + size <= arena->maxSize) {
		char* result = arena->baseAddress + arena->usedSize;
		arena->usedSize += size;

		return result;
	}
	else {
		return 0;
	}
}

inline unsigned int GetArenaSize(MemArena* arena) {
	return arena->usedSize;
}

inline void SetArenaSize(MemArena* arena, unsigned int size) {
	arena->usedSize = size;
}

// TODO: only pop if the address is smaller than the current one?
inline void ArenaPopTo(MemArena* arena, void* address) {
	arena->usedSize = (unsigned int)((char*)address - arena->baseAddress);
}

#define ArenaPushType(arena, type) ((type*)ArenaAlloc((arena), sizeof(type)))
#define ArenaPushArray(arena, type, size) ((type*)ArenaAlloc((arena), (size) * sizeof(type)))
#define ArenaPush(arena, type, variable) {\
			type* address = (type*)ArenaAlloc(arena, sizeof(variable)); \
			*address = (variable); }
#define ArenaPushVar(arena, variable) {  \
			char* address = (char*)ArenaAlloc((arena), sizeof(variable)); \
		    memcpy((address), &(variable), sizeof(variable)); }