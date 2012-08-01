// defines driver entry points

#include <windows.h>

#include "driver_entry.h"


DRIVER_FUNCTION HANDLE
COM_Init(ULONG Identifier) {
	return NULL;
}

DRIVER_FUNCTION BOOL
COM_PreDeinit(PVOID pInitContext) {
	return FALSE;
}

DRIVER_FUNCTION BOOL
COM_Deinit(PVOID pInitContext) {
	return FALSE;
}

DRIVER_FUNCTION BOOL
COM_PowerUp(PVOID pInitContext) {
	return FALSE;
}

DRIVER_FUNCTION BOOL
COM_PowerDown(PVOID pInitContext) {
	return FALSE;
}

DRIVER_FUNCTION HANDLE
COM_Open(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode) {
	return NULL;
}

DRIVER_FUNCTION BOOL
Com_PreClose(PVOID pOpenContext) {
	return FALSE;
}

DRIVER_FUNCTION BOOL
COM_Close(PVOID pOpenContext) {
	return FALSE;
}

DRIVER_FUNCTION ULONG
COM_Read(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	return 0;
}

DRIVER_FUNCTION ULONG
COM_Write(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	return 0;
}

DRIVER_FUNCTION ULONG
COM_Seek(PVOID pOpenContext, LONG Position, DWORD Type) {
	return 0;
}

DRIVER_FUNCTION BOOL
COM_IOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
	return FALSE;
}