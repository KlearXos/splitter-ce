#include "serial_splitter.h"

#ifndef _UNDER_TESTING
#include <devload.h>
#else
#define DEVACCESS_BUSNAMESPACE          FILE_WRITE_ATTRIBUTES
#endif

#include "splitter_settings.h"
#include "serial_port_driver.h"
#include "allocation.h"

struct __splitted_port_context;

typedef struct __splitter_context {
	// loaded real driver
	PSerialDriverContext	serialDriver;

	// result of real driver init
	HANDLE					initHandle;
}SplitterContext, *PSplitterContext;

////////////////    static functions


///////////////////////////////

HANDLE
InitSerialSplitter(ULONG id) {
	PSplitterSettings	settings;
	PSerialDriverContext	serialDriverContext;

	PSplitterContext		context;

	DWORD			error = ERROR_SUCCESS;

	context = AllocateMemory(sizeof(SplitterContext));
	if (NULL == context) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	settings = GetSplitterSettings((wchar_t*)id);
	if (NULL != settings) {
		serialDriverContext = LoadSerialDriver(settings->driverPath);

		FreeSplitterSettings(settings);

		if (NULL != serialDriverContext) {
			context->serialDriver = serialDriverContext;

			// init real driver
			context->initHandle = context->serialDriver->driverFunctions.COM_Init(id);

			if (NULL != context->initHandle) {
				SetLastError(ERROR_SUCCESS);
				return (HANDLE)context;
			} else {
				error = GetLastError();
			}

			UnloadSerialDriver(serialDriverContext);
		} else {
			error = GetLastError();
		}
	} else {
		error = GetLastError();
	}

	FreeMemory((void*)context);

	SetLastError(error);

	return NULL;
}

BOOL
DeinitSerialSplitter(PVOID initContext) {
	DWORD		error = ERROR_SUCCESS;

	PSplitterContext	context = (PSplitterContext)initContext;

	if (NULL == initContext) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	context->serialDriver->driverFunctions.COM_Deinit(context->initHandle);

	UnloadSerialDriver(context->serialDriver);

	FreeMemory((void*)context);

	SetLastError(error);
	return TRUE;
}

BOOL
PreDeinit(PVOID initContext) {
	PSplitterContext	context = (PSplitterContext)initContext;

	if (NULL == context) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (NULL != context->serialDriver->driverFunctions.COM_PreDeinit) {
		return context->serialDriver->driverFunctions.COM_PreDeinit(context->initHandle);
	}

	return FALSE;
}

BOOL	PowerUp(PVOID initContext) {
	PSplitterContext	context = (PSplitterContext)initContext;

	if (NULL == context) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (NULL != context->serialDriver->driverFunctions.COM_PowerUp) {
		return context->serialDriver->driverFunctions.COM_PowerUp(context->initHandle);
	}

	return FALSE;
}

BOOL	PowerDown(PVOID initContext) {
	PSplitterContext	context = (PSplitterContext)initContext;

	if (NULL == context) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (NULL != context->serialDriver->driverFunctions.COM_PowerDown) {
		return context->serialDriver->driverFunctions.COM_PowerDown(context->initHandle);
	}

	return FALSE;
}

HANDLE
SplitPort(PVOID initContext, DWORD AccessCode, DWORD ShareMode) {
	PSplitterContext	context = (PSplitterContext)initContext;

	HANDLE	result = NULL;

	if (AccessCode & DEVACCESS_BUSNAMESPACE) {
		// open buss access handle
	} else {
		// open read/write port
	}

	return result;
}

BOOL
PreClosePort(LPVOID openContext) {
	return FALSE;
}

BOOL
ClosePort(LPVOID openContext) {
	return FALSE;
}

ULONG
PortWrite(PVOID openContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	return 0;
}

BOOL
PortIOControl(PVOID openContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
			  PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
				  return FALSE;
}