#include "real_port_provider.h"

#include "context_functions.h"

BOOL
Call_COM_Read(LPVOID ctx, void* pTargetBuffer, ULONG BufferLength, ULONG *bytes_read) {
	PSplitterContext splitter;

	splitter = (PSplitterContext)ctx;

	return ReadFile(splitter->real_open_context, pTargetBuffer, BufferLength, bytes_read, NULL);
}

ULONG
Call_COM_Write(LPVOID ctx, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	return 0;
}

BOOL
Call_COM_PreDeinit(LPVOID ctx) {
	return TRUE;
	//BOOL	result;
	//if (ctx != NULL) {
	//	result = COM_PreDeinit_Real(&ctx->real_port_functions, ctx->real_init_context);
	//} else {
	//	result = FALSE;
	//}

	//return result;
}

BOOL
Call_COM_PowerUp(LPVOID ctx) {
	return TRUE;
	//BOOL	result;

	//if (ctx != NULL) {
	//	result = COM_PowerUp_Real(&ctx->real_port_functions, ctx->real_init_context);
	//} else {
	//	result = FALSE;
	//}

	//return result;
}

BOOL
Call_COM_PowerDown(LPVOID ctx) {
	return TRUE;
	//BOOL	result;

	//if (ctx != NULL) {
	//	result = COM_PowerDown_Real(&ctx->real_port_functions, ctx->real_init_context);
	//} else {
	//	result = FALSE;
	//}

	//return result;
}

BOOL
Call_COM_IOControl(PVOID ctx, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
				   PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
	BOOL	result;

	PSplitterContext splitter;

	splitter = (PSplitterContext)ctx;

	result = DeviceIoControl(splitter->real_open_context, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut, NULL);

	return result;
}