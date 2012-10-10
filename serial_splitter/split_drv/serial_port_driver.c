#include "serial_port_driver.h"

#include "allocation.h"

#define COM_Init_R		TRUE
#define COM_PreDeinit_R FALSE
#define COM_Deinit_R	TRUE
#define COM_Open_R		TRUE
#define COM_PreClose_R	FALSE
#define COM_Close_R		TRUE
#define COM_Read_R		TRUE 
#define COM_Write_R		TRUE
#define COM_Seek_R		FALSE
#define COM_PowerUp_R	TRUE
#define COM_PowerDown_R	TRUE
#define COM_IOControl_R	TRUE

extern HINSTANCE LoadDriver(LPCWSTR lpszFileName);

#define GET_FUNCTION(real_port_functions, real_port_driver, function_name) real_port_functions->function_name = \
	(function_name##_Pointer)GetProcAddress(real_port_driver, TEXT(#function_name)); \
	if (real_port_functions->function_name == NULL) { if (function_name##_R) break; }

static HINSTANCE	LoadSerialDriverLib(wchar_t	*path);
static void			UnloadSerialDriverLib(HINSTANCE lib);

static BOOL		GetSerialDriverFunctions(HINSTANCE driverLib, PPortFunctions functions);

PSerialDriverContext	LoadSerialDriver(wchar_t	*path) {
	PSerialDriverContext	result = AllocateMemory(sizeof(SerialDriverContext));

	if (NULL != result) {
		result->driverLib = LoadSerialDriverLib(path);
		if (NULL != result->driverLib) {
			if (GetSerialDriverFunctions(result->driverLib, &result->driverFunctions)) {
				return result;
			}

			UnloadSerialDriverLib(result->driverLib);
		}

		FreeMemory((void*)result);
	}

	return NULL;
}


void
UnloadSerialDriver(PSerialDriverContext context) {
	if (NULL != context) {
		if (NULL != context->driverLib) {
			UnloadSerialDriverLib(context->driverLib);
		}

		FreeMemory((void*)context);
	}
}

HINSTANCE
LoadSerialDriverLib(wchar_t	*path) {
	return LoadDriver(path);
}

void
UnloadSerialDriverLib(HINSTANCE lib) {
	FreeLibrary(lib);
}

BOOL
GetSerialDriverFunctions(HINSTANCE driverLib, PPortFunctions functions) {
	do {
		GET_FUNCTION(functions, driverLib, COM_Init);
		GET_FUNCTION(functions, driverLib, COM_PreDeinit);
		GET_FUNCTION(functions, driverLib, COM_Deinit);
		GET_FUNCTION(functions, driverLib, COM_Open);
		GET_FUNCTION(functions, driverLib, COM_PreClose);
		GET_FUNCTION(functions, driverLib, COM_Close);
		GET_FUNCTION(functions, driverLib, COM_Read);
		GET_FUNCTION(functions, driverLib, COM_Write);
		GET_FUNCTION(functions, driverLib, COM_Seek);
		GET_FUNCTION(functions, driverLib, COM_PowerUp);
		GET_FUNCTION(functions, driverLib, COM_PowerDown);
		GET_FUNCTION(functions, driverLib, COM_IOControl);

		if ((NULL != functions->COM_PreClose) && (NULL == functions->COM_PreDeinit)) {
			// If XXX_PreClose (Device Manager) is present, XXX_PreDeinit must 
			// also be present or the driver will not load.
			break;
		}

		return TRUE;
	} while (0);

	return FALSE;
}