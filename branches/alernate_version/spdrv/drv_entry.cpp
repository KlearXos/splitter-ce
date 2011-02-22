#include "drv_entry.h"

// driver init function
// Identifier - pointer to 
// should return virtual port context
DRIVER_FUNCTION HANDLE
COM_Init (ULONG   Identifier) {
	PSplitterContext ctx;

	ctx = (PSplitterContext)LocalAlloc(sizeof(SplitterContext), LMEM_ZEROINIT);
	if (ctx == NULL) {
		return NULL;
	}

	if (!InitContext(ctx, Identifier)) {
		LocalFree(ctx);
		ctx = NULL;
	}

	return ctx;
}

// do nothing inside
DRIVER_FUNCTION BOOL
COM_PreDeinit(PVOID pInitContext) {
	return Call_COM_PreDeinit((PSplitterContext)pInitContext);
}

DRIVER_FUNCTION BOOL
COM_Deinit(PVOID pInitContext) {
	return DeinitContext((PSplitterContext)pInitContext);
}

DRIVER_FUNCTION HANDLE
COM_Open(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode) {
	PPortOpenContext	port_context;

	port_context = (PPortOpenContext)LocalAlloc(LMEM_ZEROINIT, sizeof(PortOpenContext));
	if (port_context != NULL) {
		if (InitOpenPortContex(port_context)) {
			if (AddOpenPortContextToSplitter((PSplitterContext)pInitContext, port_context)) {
				return (HANDLE)port_context;
			}

			DeinitOpenPortContext(port_context);
		}

		LocalFree(port_context);
	}

	return NULL;
}

DRIVER_FUNCTION BOOL
COM_Close(PVOID pOpenContext) {
	if (RemovePortContextFromSplitter((PPortOpenContext)pOpenContext)) {
		DeinitOpenPortContext((PPortOpenContext)pOpenContext);

		LocalFree(pOpenContext);
		return TRUE;
	}

	return FALSE;
}

DRIVER_FUNCTION ULONG
COM_Read(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	return ReadFromVirtualPort((PPortOpenContext)pOpenContext, pTargetBuffer, BufferLength);
}

DRIVER_FUNCTION ULONG
COM_Write(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	return WriteToVirtualPort((PPortOpenContext)pOpenContext, pSourceBytes, NumberOfBytes);
}

DRIVER_FUNCTION ULONG
COM_Seek(PVOID pOpenContext, LONG Position, DWORD Type) {
	return VirtualPortSeek((PPortOpenContext)pOpenContext, Position, Type);
}

DRIVER_FUNCTION BOOL
COM_PowerUp(PVOID pInitContext) {
	return Call_COM_PowerUp((PSplitterContext)pInitContext);
}

DRIVER_FUNCTION BOOL
COM_PowerDown(PVOID pInitContext) {
	return Call_COM_PowerDown((PSplitterContext)pInitContext);
}

DRIVER_FUNCTION BOOL
COM_IOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
	return VirtualPortIOControl((PPortOpenContext)pOpenContext, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
}


BOOL APIENTRY
DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls((HMODULE)hModule);

		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return (TRUE);
}