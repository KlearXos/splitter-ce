#include "serial_splitter.h"

#include "splitter_settings.h"
#include "serial_port_driver.h"
#include "allocation.h"

typedef struct __splitter_context {
	// loaded real driver
	PSerialDriverContext	serialDriver;

	// result of real driver init
	HANDLE					initHandle;
}SplitterContext, *PSplitterContext;

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