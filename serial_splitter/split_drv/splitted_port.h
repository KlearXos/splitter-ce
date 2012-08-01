#ifndef __SPLITTED_PORT_H__
#define __SPLITTED_PORT_H__

#include <windows.h>

BOOL	ReferenceSplittedPortContext(LPVOID openContext);

BOOL	DereferenceSplittedPortContext(LPVOID openContext);

ULONG	PortRead(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength);

ULONG	PortWrite(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes);

ULONG	PortSeek(PVOID pOpenContext, LONG Position, DWORD Type);

BOOL	PortIOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

BOOL	DataReceived(PVOID pOpenContext, PUCHAR	buffer, ULONG dataLength);

#endif