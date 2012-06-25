// defines driver entry points

#include <windows.h>

#include "driver_entry.h"

DRIVER_FUNCTION HANDLE
COM_Init(ULONG Identifier) {
	// XXX: not implemented
	return NULL;
}

DRIVER_FUNCTION BOOL
COM_PreDeinit(PVOID pInitContext) {
	// XXX: not implemented
	return FALSE;
}

DRIVER_FUNCTION BOOL
COM_Deinit(PVOID pInitContext) {
	// XXX: not implemented
	return FALSE;
}

DRIVER_FUNCTION HANDLE
COM_Open(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode) {
	// XXX: not implemented
	return NULL;
}

DRIVER_FUNCTION BOOL
COM_Close(PVOID pOpenContext) {
	// XXX: not implemented
	return FALSE;
}

DRIVER_FUNCTION ULONG
COM_Read(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	// XXX: not implemented
	return 0;
}

DRIVER_FUNCTION ULONG
COM_Write(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	// XXX: not implemented
	return 0;
}

DRIVER_FUNCTION ULONG
COM_Seek(PVOID pOpenContext, LONG Position, DWORD Type) {
	// XXX: not implemented
	return 0;
}

DRIVER_FUNCTION BOOL
COM_PowerUp(PVOID pInitContext) {
	// XXX: not implemented
	return FALSE;
}

DRIVER_FUNCTION BOOL
COM_PowerDown(PVOID pInitContext) {
	// XXX: not implemented
	return FALSE;
}

DRIVER_FUNCTION BOOL
COM_IOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
	// XXX: not implemented
	return FALSE;
}