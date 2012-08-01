#ifndef __SERIAL_SPLITTER_H__
#define __SERIAL_SPLITTER_H__

#include <windows.h>

HANDLE	InitSerialSplitter(ULONG id);

BOOL	DeinitSerialSplitter(PVOID initContext);

BOOL	PreDeinit(PVOID initContext);

BOOL	PowerUp(PVOID initContext);

BOOL	PowerDown(PVOID initContext);

#endif