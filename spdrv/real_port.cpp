#include "real_port.h"

#define COM_Init_R		TRUE
#define COM_PreDeinit_R FALSE
#define COM_Deinit_R	TRUE
#define COM_Open_R		TRUE
#define COM_Close_R		TRUE
#define COM_Read_R		TRUE 
#define COM_Write_R		TRUE
#define COM_Seek_R		FALSE
#define COM_PowerUp_R	TRUE
#define COM_PowerDown_R	TRUE
#define COM_IOControl_R	TRUE

#define GET_FUNCTION(real_port_functions, real_port_driver, function_name, required) real_port_functions->function_name = \
	(function_name##_Pointer)GetProcAddress(real_port_driver, TEXT(#function_name)); \
	if (real_port_functions->function_name == NULL) { if (required) break; }

#define CHECK(functions, function_name) if ((functions == NULL) || ((functions->function_name == NULL) && (function_name##_R)) ) DebugBreak()

BOOL
GetDriverFunctions(HINSTANCE driver, PPortFunctions functions) {
	BOOL result = FALSE;

	do {
		GET_FUNCTION(functions, driver, COM_Init, COM_Init_R);

		GET_FUNCTION(functions, driver, COM_PreDeinit, COM_PreDeinit_R);
		GET_FUNCTION(functions, driver, COM_Deinit, COM_Deinit_R);
		GET_FUNCTION(functions, driver, COM_Open, COM_Open_R);
		GET_FUNCTION(functions, driver, COM_Close, COM_Close_R);
		GET_FUNCTION(functions, driver, COM_Read, COM_Read_R);
		GET_FUNCTION(functions, driver, COM_Write, COM_Write_R);
		GET_FUNCTION(functions, driver, COM_Seek, COM_Seek_R);
		GET_FUNCTION(functions, driver, COM_PowerUp, COM_PowerUp_R);
		GET_FUNCTION(functions, driver, COM_PowerDown, COM_PowerDown_R);
		GET_FUNCTION(functions, driver, COM_IOControl, COM_IOControl_R);

		result = TRUE;
	} while (0);

	return result;
}

HANDLE
COM_Init_Real(PPortFunctions functions, ULONG   Identifier) {
	HANDLE	result;
#ifdef DEBUG
	CHECK(functions, COM_Init);
#endif

	if (functions->COM_Init != NULL) {
		result = functions->COM_Init(Identifier);
	} else {
		result = NULL;
	}

	return result;
}

BOOL
COM_PreDeinit_Real(PPortFunctions functions, PVOID pInitContext) {
	BOOL	result;
#ifdef DEBUG
	CHECK(functions, COM_PreDeinit);
#endif

	if (functions->COM_PreDeinit != NULL) {
		result = functions->COM_PreDeinit(pInitContext);
	} else {
		result = FALSE;
	}

	return result;
}

BOOL
COM_Deinit_Real(PPortFunctions functions, PVOID pInitContext) {
	BOOL	result;
#ifdef DEBUG
	CHECK(functions, COM_Deinit);
#endif

	if (functions->COM_Deinit != NULL) {
		result = functions->COM_Deinit(pInitContext);
	} else {
		result = FALSE;
	}

	return result;
}

HANDLE
COM_Open_Real(PPortFunctions functions, PVOID pInitContext, DWORD AccessCode, DWORD ShareMode) {
	HANDLE	result;
#ifdef DEBUG
	CHECK(functions, COM_Open);
#endif

	if (functions->COM_Open != NULL) {
		result = functions->COM_Open(pInitContext, AccessCode, ShareMode);
	} else {
		result = NULL;
	}

	return result;
}

BOOL
COM_Close_Real(PPortFunctions functions, PVOID pOpenContext) {
	BOOL	result;
#ifdef DEBUG
	CHECK(functions, COM_Close);
#endif

	if (functions->COM_Close != NULL) {
		result = functions->COM_Close(pOpenContext);
	} else {
		result = FALSE;
	}

	return result;
}

ULONG
COM_Read_Real(PPortFunctions functions, PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	ULONG	result;
#ifdef DEBUG
	CHECK(functions, COM_Read);
#endif

	if (functions->COM_Read != NULL) {
		result = functions->COM_Read(pOpenContext, pTargetBuffer, BufferLength);
	} else {
		result = 0;
	}

	return result;
}

ULONG
COM_Write_Real(PPortFunctions functions, PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	ULONG	result;
#ifdef DEBUG
	CHECK(functions, COM_Write);
#endif

	if (functions->COM_Write != NULL) {
		result = functions->COM_Write(pOpenContext, pSourceBytes, NumberOfBytes);
	} else {
		result = 0;
	}

	return result;
}

ULONG
COM_Seek_Real(PPortFunctions functions, PVOID pOpenContext, LONG Position, DWORD Type) {
	ULONG	result;
#ifdef DEBUG
	CHECK(functions, COM_Seek);
#endif

	if (functions->COM_Seek != NULL) {
		result = functions->COM_Seek(pOpenContext, Position, Type);
	} else {
		result = 0;
	}

	return result;
}

BOOL
COM_PowerUp_Real(PPortFunctions functions, PVOID pInitContext) {
	BOOL	result;
#ifdef DEBUG
	CHECK(functions, COM_PowerUp);
#endif

	if (functions->COM_PowerUp != NULL) {
		result = functions->COM_PowerUp(pInitContext);
	} else {
		result = FALSE;
	}

	return result;
}

BOOL
COM_PowerDown_Real(PPortFunctions functions, PVOID pInitContext) {
	BOOL	result;
#ifdef DEBUG
	CHECK(functions, COM_PowerDown);
#endif

	if (functions->COM_PowerDown != NULL) {
		result = functions->COM_PowerDown(pInitContext);
	} else {
		result = FALSE;
	}

	return result;
}

BOOL
COM_IOControl_Real(PPortFunctions functions, PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
				   PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
	BOOL	result;

#ifdef DEBUG
	CHECK(functions, COM_IOControl);
#endif

	if (functions->COM_IOControl != NULL) {
		result = functions->COM_IOControl(pOpenContext, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
	} else {
		result = FALSE;
	}

	return result;
}