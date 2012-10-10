// defines driver entry points

#include <windows.h>

#include "driver_entry.h"

#include "serial_splitter.h"
#include "splitted_port.h"

DRIVER_FUNCTION HANDLE
COM_Init(ULONG Identifier) {
	return InitSerialSplitter(Identifier);
}

DRIVER_FUNCTION BOOL
COM_PreDeinit(PVOID pInitContext) {
	return PreDeinit(pInitContext);
}

DRIVER_FUNCTION BOOL
COM_Deinit(PVOID pInitContext) {
	return DeinitSerialSplitter(pInitContext);
}

DRIVER_FUNCTION BOOL
COM_PowerUp(PVOID pInitContext) {
	return PowerUp(pInitContext);;
}

DRIVER_FUNCTION BOOL
COM_PowerDown(PVOID pInitContext) {
	return PowerDown(pInitContext);
}

DRIVER_FUNCTION HANDLE
COM_Open(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode) {
	return SplitPort(pInitContext, AccessCode, ShareMode);
}

DRIVER_FUNCTION BOOL
Com_PreClose(PVOID pOpenContext) {
	return PreClosePort(pOpenContext);
}

DRIVER_FUNCTION BOOL
COM_Close(PVOID pOpenContext) {
	return ClosePort(pOpenContext);
}

DRIVER_FUNCTION ULONG
COM_Read(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	return PortRead(pOpenContext, pTargetBuffer, BufferLength);
}

DRIVER_FUNCTION ULONG
COM_Write(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	return PortWrite(pOpenContext, pSourceBytes, NumberOfBytes);
}

DRIVER_FUNCTION ULONG
COM_Seek(PVOID pOpenContext, LONG Position, DWORD Type) {
	return PortSeek(pOpenContext, Position, Type);
}

DRIVER_FUNCTION BOOL
COM_IOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
	return PortIOControl(pOpenContext, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
}