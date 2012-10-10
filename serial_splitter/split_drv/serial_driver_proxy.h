#ifndef __SERIAL_DRIVER_PROXY_H__
#define __SERIAL_DRIVER_PROXY_H__

// functions in this file are used by any client of real port driver, except splitter context itself

#include <windows.h>

ULONG	WriteToRealPort(PVOID portContext, PUCHAR buffer, ULONG bufferLength);

BOOL	PreCloseRealPort(PVOID portContext);

BOOL	CloseRealPort(PVOID portContext, PVOID closedSplittedPortContext);

BOOL	IOControlRealPort(PVOID portContext, DWORD code, PBYTE bufferIn,DWORD bufferInLength,
    PBYTE bufferOut, DWORD bufferOutLength, PDWORD actualOutLength);

#endif