#include "serial_driver.h"

#include <windows.h>

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

SerialDriver::SerialDriver(const PortFunctions &functions) : m_portFunctions(functions) {
}

SerialDriver::~SerialDriver() {
}

#define GET_FUNCTION(real_port_functions, real_port_driver, function_name) real_port_functions.function_name = \
	(function_name##_Pointer)GetProcAddress(real_port_driver, TEXT(#function_name)); \
	if (real_port_functions.function_name == NULL) { if (function_name##_R) break; }


SerialDriver*
SerialDriver::GetDriver(HINSTANCE driverLib) {
	PortFunctions	functions;
	SerialDriver*	result = NULL;

	memset(&functions, 0, sizeof(functions));

	do {
		GET_FUNCTION(functions, driverLib, COM_Init);
		GET_FUNCTION(functions, driverLib, COM_PreDeinit);
		GET_FUNCTION(functions, driverLib, COM_Deinit);
		GET_FUNCTION(functions, driverLib, COM_Open);
		GET_FUNCTION(functions, driverLib, COM_Close);
		GET_FUNCTION(functions, driverLib, COM_Read);
		GET_FUNCTION(functions, driverLib, COM_Write);
		GET_FUNCTION(functions, driverLib, COM_Seek);
		GET_FUNCTION(functions, driverLib, COM_PowerUp);
		GET_FUNCTION(functions, driverLib, COM_PowerDown);
		GET_FUNCTION(functions, driverLib, COM_IOControl);

		result = new SerialDriver(functions);
	} while (0);

	return result;
}

HANDLE
SerialDriver::Init(ULONG Identifier) const {
	return (m_portFunctions.COM_Init != NULL)
		? m_portFunctions.COM_Init(Identifier) 
		: NULL;
}

BOOL
SerialDriver::PreDeinit(PVOID pInitContext) const {
	return (m_portFunctions.COM_PreDeinit != NULL)
		? m_portFunctions.COM_PreDeinit(pInitContext) 
		: FALSE;
}

BOOL
SerialDriver::Deinit(PVOID pInitContext) const {
	return (m_portFunctions.COM_Deinit != NULL)
		? m_portFunctions.COM_Deinit(pInitContext) 
		: FALSE;
}

HANDLE
SerialDriver::Open(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode) const {
	return (m_portFunctions.COM_Open != NULL)
		? m_portFunctions.COM_Open(pInitContext, AccessCode, ShareMode)
		: NULL;
}

BOOL
SerialDriver::Close(PVOID pOpenContext) const {
	return (m_portFunctions.COM_Close != NULL)
		? m_portFunctions.COM_Close(pOpenContext)
		: FALSE;
}

ULONG
SerialDriver::Read(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) const {
	return (m_portFunctions.COM_Read != NULL)
		? m_portFunctions.COM_Read(pOpenContext, pTargetBuffer, BufferLength)
		: 0;
}

ULONG
SerialDriver::Write(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) const {
	return (m_portFunctions.COM_Write != NULL)
		? m_portFunctions.COM_Write(pOpenContext, pSourceBytes, NumberOfBytes)
		: 0;
}

ULONG
SerialDriver::Seek(PVOID pOpenContext, LONG Position, DWORD Type) const {
	return (m_portFunctions.COM_Seek != NULL)
		? m_portFunctions.COM_Seek(pOpenContext, Position, Type)
		: 0;
}

BOOL
SerialDriver::PowerUp(PVOID pInitContext) const {
	return (m_portFunctions.COM_PowerUp != NULL)
		? m_portFunctions.COM_PowerUp(pInitContext)
		: FALSE;
}

BOOL
SerialDriver::PowerDown(PVOID pInitContext) const {
	return (m_portFunctions.COM_PowerDown != NULL)
		? m_portFunctions.COM_PowerDown(pInitContext)
		: FALSE;
}

BOOL
SerialDriver::IOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
	PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) const {
	return (m_portFunctions.COM_IOControl != NULL)
		? m_portFunctions.COM_IOControl(pOpenContext, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut)
		: FALSE;
}