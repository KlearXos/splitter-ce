#ifndef __REAL_PORT_ENV_H_16022011__
#define __REAL_PORT_ENV_H_16022011__

#include <windows.h>

#define PORT_FUNCTION_CALL	__stdcall
#define DECLARE_FUNCTION(x) x##_Pointer	x

typedef HANDLE	(PORT_FUNCTION_CALL *COM_Init_Pointer)(ULONG   Identifier);
typedef BOOL (PORT_FUNCTION_CALL *COM_PreDeinit_Pointer)(PVOID pInitContext);
typedef BOOL (PORT_FUNCTION_CALL *COM_Deinit_Pointer)(PVOID pInitContext);
typedef HANDLE (PORT_FUNCTION_CALL *COM_Open_Pointer)(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode);
typedef BOOL (PORT_FUNCTION_CALL *COM_Close_Pointer)(PVOID pOpenContext);
typedef ULONG (PORT_FUNCTION_CALL *COM_Read_Pointer)(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength);
typedef ULONG (PORT_FUNCTION_CALL *COM_Write_Pointer)(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes);
typedef ULONG (PORT_FUNCTION_CALL *COM_Seek_Pointer)(PVOID pOpenContext, LONG Position, DWORD Type);
typedef BOOL (PORT_FUNCTION_CALL *COM_PowerUp_Pointer)(PVOID pInitContext);
typedef BOOL (PORT_FUNCTION_CALL *COM_PowerDown_Pointer)(PVOID pInitContext);
typedef BOOL (PORT_FUNCTION_CALL *COM_IOControl_Pointer)(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);
    
typedef struct __port_functions {
	DECLARE_FUNCTION(COM_Init);
	DECLARE_FUNCTION(COM_PreDeinit);
	DECLARE_FUNCTION(COM_Deinit);
	DECLARE_FUNCTION(COM_Open);
	DECLARE_FUNCTION(COM_Close);
	DECLARE_FUNCTION(COM_Read);
	DECLARE_FUNCTION(COM_Write);
	DECLARE_FUNCTION(COM_Seek);
	DECLARE_FUNCTION(COM_PowerUp);
	DECLARE_FUNCTION(COM_PowerDown);
	DECLARE_FUNCTION(COM_IOControl);
} PortFunctions, *PPortFunctions;

BOOL	GetDriverFunctions(HINSTANCE driver, PPortFunctions functions);

// mirror functions

HANDLE	COM_Init_Real(PPortFunctions functions, ULONG   Identifier);
BOOL	COM_PreDeinit_Real(PPortFunctions functions, PVOID pInitContext);
BOOL	COM_Deinit_Real(PPortFunctions functions, PVOID pInitContext);
HANDLE	COM_Open_Real(PPortFunctions functions, PVOID pInitContext, DWORD AccessCode, DWORD ShareMode);
BOOL	COM_Close_Real(PPortFunctions functions, PVOID pOpenContext);
ULONG	COM_Read_Real(PPortFunctions functions, PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength);
ULONG	COM_Write_Real(PPortFunctions functions, PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes);
ULONG	COM_Seek_Real(PPortFunctions functions, PVOID pOpenContext, LONG Position, DWORD Type);
BOOL	COM_PowerUp_Real(PPortFunctions functions, PVOID pInitContext);
BOOL	COM_PowerDown_Real(PPortFunctions functions, PVOID pInitContext);
BOOL	COM_IOControl_Real(PPortFunctions functions, PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

#endif