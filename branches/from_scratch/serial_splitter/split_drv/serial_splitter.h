#ifndef __SERIAL_SPLITTER_H__
#define __SERIAL_SPLITTER_H__

#include <windows.h>

// create context for serial splitter (one for port)
// load and init real driver
HANDLE	InitSerialSplitter(ULONG id);

// deinit real driver, close all client connections
// unload driver
BOOL	DeinitSerialSplitter(PVOID initContext);

// just call proper method from real driver
BOOL	PreDeinit(PVOID initContext);

// just call proper method from real driver
BOOL	PowerUp(PVOID initContext);

// just call proper method from real driver
BOOL	PowerDown(PVOID initContext);

// create splitted port instance. Could be read/write instance, or 
// BUS control instance (XXX currently not supported)
HANDLE	SplitPort(PVOID initContext, DWORD AccessCode, DWORD ShareMode);

// just redirect call to real driver (should be called once)
BOOL	PreClosePort(LPVOID openContext);

BOOL	ClosePort(LPVOID openContext);

// read data from internal buffer if available
// otherwise wait according to user settings
ULONG	PortRead(PVOID openContext, PUCHAR pTargetBuffer, ULONG BufferLength);

// write to real port throug callback
ULONG	PortWrite(PVOID openContext, PUCHAR pSourceBytes, ULONG NumberOfBytes);

// internal port seek. This time is ignored
ULONG	PortSeek(PVOID openContext, LONG Position, DWORD Type);

// only master port could control port driver indirectly
// IOcontrol from all other clients should be ingonred
BOOL	PortIOControl(PVOID openContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

#endif