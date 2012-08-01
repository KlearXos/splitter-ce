#ifndef __SERIAL_PORT_DRIVER_H__
#define __SERIAL_PORT_DRIVER_H__

#include <windows.h>

#define DECLARE_FUNCTION(x) x##_Pointer	x

typedef HANDLE	(__stdcall *COM_Init_Pointer)(ULONG   Identifier);
typedef BOOL	(__stdcall *COM_PreDeinit_Pointer)(PVOID pInitContext);
typedef BOOL	(__stdcall *COM_Deinit_Pointer)(PVOID pInitContext);
typedef HANDLE	(__stdcall *COM_Open_Pointer)(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode);
typedef BOOL	(__stdcall *COM_PreClose_Pointer)(PVOID pOpenContext);
typedef BOOL	(__stdcall *COM_Close_Pointer)(PVOID pOpenContext);
typedef ULONG	(__stdcall *COM_Read_Pointer)(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength);
typedef ULONG	(__stdcall *COM_Write_Pointer)(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes);
typedef ULONG	(__stdcall *COM_Seek_Pointer)(PVOID pOpenContext, LONG Position, DWORD Type);
typedef BOOL	(__stdcall *COM_PowerUp_Pointer)(PVOID pInitContext);
typedef BOOL	(__stdcall *COM_PowerDown_Pointer)(PVOID pInitContext);
typedef BOOL	(__stdcall *COM_IOControl_Pointer)(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
	PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

typedef struct __port_functions {
	DECLARE_FUNCTION(COM_Init);
	DECLARE_FUNCTION(COM_PreDeinit);
	DECLARE_FUNCTION(COM_Deinit);
	DECLARE_FUNCTION(COM_Open);
	DECLARE_FUNCTION(COM_PreClose);
	DECLARE_FUNCTION(COM_Close);
	DECLARE_FUNCTION(COM_Read);
	DECLARE_FUNCTION(COM_Write);
	DECLARE_FUNCTION(COM_Seek);
	DECLARE_FUNCTION(COM_PowerUp);
	DECLARE_FUNCTION(COM_PowerDown);
	DECLARE_FUNCTION(COM_IOControl);
} PortFunctions, *PPortFunctions;

typedef struct __serial_driver {
	HINSTANCE		driverLib;
	PortFunctions	driverFunctions;
}SerialDriverContext, *PSerialDriverContext;

PSerialDriverContext	LoadSerialDriver(wchar_t	*path);

void					UnloadSerialDriver(PSerialDriverContext context);

#endif