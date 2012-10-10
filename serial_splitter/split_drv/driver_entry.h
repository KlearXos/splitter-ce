#ifndef __SPLIT_DRIVER_ENTRY_H__
#define __SPLIT_DRIVER_ENTRY_H__

#define DRIVER_FUNCTION		__declspec(dllexport)

DRIVER_FUNCTION HANDLE
COM_Init(ULONG Identifier);

DRIVER_FUNCTION BOOL
COM_PreDeinit(PVOID pInitContext);

DRIVER_FUNCTION BOOL
COM_Deinit(PVOID pInitContext);

DRIVER_FUNCTION BOOL
COM_PowerUp(PVOID pInitContext);

DRIVER_FUNCTION BOOL
COM_PowerDown(PVOID pInitContext) ;

DRIVER_FUNCTION HANDLE
COM_Open(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode);

DRIVER_FUNCTION BOOL
Com_PreClose(PVOID pOpenContext);

DRIVER_FUNCTION BOOL
COM_Close(PVOID pOpenContext);

DRIVER_FUNCTION ULONG
COM_Read(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength);

DRIVER_FUNCTION ULONG
COM_Write(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes);

DRIVER_FUNCTION ULONG
COM_Seek(PVOID pOpenContext, LONG Position, DWORD Type);

DRIVER_FUNCTION BOOL
COM_IOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

#endif