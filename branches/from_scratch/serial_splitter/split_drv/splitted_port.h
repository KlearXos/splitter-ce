#ifndef __SPLITTED_PORT_H__
#define __SPLITTED_PORT_H__

#include <windows.h>

typedef ULONG (__stdcall *WriteToPortFunctionPointer)(LPVOID context, PUCHAR pSourceBytes, ULONG NumberOfBytes);

typedef BOOL (__stdcall *IOControlPortFunctionPointer)(LPVOID context, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

// creates context of splitted port, totally configured
// output only for write and IO control
// input data go through DataReceived from port reader
LPVOID	CreateSplittedPort(DWORD accessCode, DWORD shareMode, LPVOID splitterContext);

// close context that it will be unavailable for any kind of operations
// context will be deleted when reference count of context will be down to zero
BOOL	CloseSplittedPort(LPVOID splittedPortContext);

// data received from real port, copy to internal buffer and complete 
// reading operation if penfing
BOOL	DataReceived(PVOID pOpenContext, PUCHAR	buffer, ULONG dataLength);

#endif