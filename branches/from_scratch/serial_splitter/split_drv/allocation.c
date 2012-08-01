#include <windows.h>

#include "allocation.h"

void*
AllocateMemory(size_t size) {
	return LocalAlloc(LMEM_ZEROINIT, size);
}

void
FreeMemory(void* p) {
	LocalFree(p);
}