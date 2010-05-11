#ifndef __DRIVER_SPLITTER_H__
#define __DRIVER_SPLITTER_H__

#include <windows.h>
#include "..\common\splitter_settings.h"
#include "cbuffer.h"

///////////////////////////////////////////////////////////////////////////////
//			driver settings defines section START		     //
///////////////////////////////////////////////////////////////////////////////

// if this macro defined, ReadFile operations will complete immediatly, even 
// if there are no data. otherwise, if internal buffer is empty ReadFile 
// operation will wait for them for READ_WAIT_TIMEOUT.
//#define READ_COMPLETES_IMMEDEATLY

#define	READ_WAIT_TIMEOUT		1000

#define INTERNAL_BUFFER_INITIAL_SIZE	1024

#define STORE_OPENER_ID

///////////////////////////////////////////////////////////////////////////////
//			driver settings defines section END		     //
///////////////////////////////////////////////////////////////////////////////

#define DRIVER_FUNCTION 	extern "C" __declspec(dllexport)
#define PORT_FUNCTION_CALL	__stdcall

#define DECLARE_FUNCTION(x) x##_Pointer	x

#define GET_FUNCTION(ctx, function_name, required) ctx->real_port_functions.function_name = \
	(function_name##_Pointer)GetProcAddress(ctx->real_port_driver, TEXT(#function_name)); \
	if (ctx->real_port_functions.function_name == NULL) { MessageBox(NULL, TEXT(#function_name), L"splitter driver", MB_OK | MB_TOPMOST); if (required) break; }

// context for opened port (e.g. port handle)
typedef struct __port_context {
	struct __port_context	*next;
	struct __port_context	*prev;

	HANDLE			sem_cb;
	CCircBuffer		*cb_ctx;

#ifdef STORE_OPENER_ID
	// opener process ID (the owner)
	HANDLE			opener_id; //GetOwnerProcess
#endif

#ifndef READ_COMPLETES_IMMEDEATLY
	HANDLE			ev_data;
#endif

	PVOID			splitter_context;
} PortContext, *PPortContext;

typedef HANDLE	(PORT_FUNCTION_CALL *COM_Init_Pointer) (ULONG   Identifier);
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

// context of whole splitter (initial context)
typedef struct __splitter_context {
	HANDLE		sem_port_list;
	PPortContext	port_list_head; // next == NULL

	HINSTANCE	real_port_driver;
	PortFunctions	real_port_functions;
	
	HANDLE		real_port_init_context;
	HANDLE		real_port_handle;

	HANDLE		sem_read;
} SplitterContext, *PSplitterContext;

#endif