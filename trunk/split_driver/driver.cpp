/*
 *  FILE: driver.cpp
 *
 *  Description in breaf:
 *	This file is a core of "splitting" driver. It's allow multpe opening.
 *  Notes:
 *     For possible issues refer to readme.txt in the root of the project
 *  About:
 *	This file is covered under GNU/GPL license. Feel free to use and modify it.
 * Author: Ation
 * You can contact me: gation@gmail.com
 */

#include "driver.h"

// coredll.dll
extern "C" HINSTANCE LoadDriver(LPCWSTR lpszFileName);
extern "C" HANDLE GetOwnerProcess (void);

BOOL	init_context(PSplitterContext context);
BOOL	deinit_context(PSplitterContext context);

// load_real_driver will also call to COM_Init function
BOOL	load_real_driver(PSplitterContext ctx, PSplitterSettings settings, ULONG Identifier);
BOOL	get_real_driver_functions(PSplitterContext ctx);

// unload_real_driver will also call to COM_Deinit function
BOOL	unload_real_driver(PSplitterContext ctx);

PPortContext	create_port_context(void);
void		free_port_context(PPortContext port_context);

DRIVER_FUNCTION HANDLE
COM_Init (ULONG   Identifier) {
	PSplitterSettings	settings;
	PSplitterContext	ctx;
	BOOL			result;

	// allocate all necessary memory
	settings = (PSplitterSettings)LocalAlloc(LMEM_ZEROINIT, sizeof(SplitterSettings));
	if (settings == NULL) {
		return NULL;
	}

	ctx = (PSplitterContext)LocalAlloc(LMEM_ZEROINIT, sizeof(SplitterContext));
	if (ctx != NULL) {
		// get settings
		result = get_splitter_settings(settings);
		if (result) {
			result = init_context(ctx);
			if (result) {
				// load driver
				result = load_real_driver(ctx, settings, Identifier);
				if (result) {
					LocalFree(settings);
					// init success
					return ctx;
				} else {
					MessageBox(NULL, L"failed to load driver", L"splitter driver", MB_OK | MB_TOPMOST);
				}
				deinit_context(ctx);
			} else {
				MessageBox(NULL, L"failed to init context", L"splitter driver", MB_OK | MB_TOPMOST);
			}
		} else {
			MessageBox(NULL, L"failed to get settings", L"splitter driver", MB_OK | MB_TOPMOST);
		}

		LocalFree(ctx);
	} else {
		MessageBox(NULL, L"failed to init driver", L"splitter driver", MB_OK | MB_TOPMOST);
	}

	LocalFree(settings);

	// init failed
	return NULL;
}

// me do nothing, just call apropriate function
DRIVER_FUNCTION BOOL
COM_PreDeinit(PVOID pInitContext) {
	PSplitterContext	ctx;

	ctx = (PSplitterContext)pInitContext;

	if (ctx->real_port_functions.COM_PreDeinit == NULL) return TRUE;
	return ctx->real_port_functions.COM_PreDeinit(ctx->real_port_init_context);
}

DRIVER_FUNCTION BOOL
COM_Deinit(PVOID pInitContext) {
	PSplitterContext	ctx;

	ctx = (PSplitterContext)pInitContext;

	unload_real_driver(ctx);

	deinit_context(ctx);
	LocalFree(ctx);

	return TRUE;
}

DRIVER_FUNCTION HANDLE
COM_Open(PVOID pInitContext, DWORD AccessCode, DWORD ShareMode) {
	HANDLE			temp_port_handle;
	PPortContext		temp_port_context;
	PSplitterContext	ctx;

	temp_port_context = create_port_context();
	if (temp_port_context == NULL) {
		return INVALID_HANDLE_VALUE;
	}

	ctx = (PSplitterContext)pInitContext;

	WaitForSingleObject(ctx->sem_port_list, INFINITE);
	if (ctx->real_port_handle == INVALID_HANDLE_VALUE) {
		temp_port_handle = ctx->real_port_functions.COM_Open(ctx->real_port_init_context, AccessCode, ShareMode);
		if (temp_port_handle == INVALID_HANDLE_VALUE) {
			ReleaseSemaphore(ctx->sem_port_list, 1, NULL);

			free_port_context(temp_port_context);

			return INVALID_HANDLE_VALUE;
		}
	}

	temp_port_context->splitter_context = pInitContext;

	temp_port_context->prev = ctx->port_list_head;
	if (ctx->port_list_head != NULL) {
		ctx->port_list_head->next = temp_port_context;
	}

	ctx->port_list_head = temp_port_context;

	ReleaseSemaphore(ctx->sem_port_list, 1, NULL);

	return temp_port_context;
}

DRIVER_FUNCTION BOOL
COM_Close(PVOID pOpenContext) {
	PPortContext		port_context;
	PSplitterContext	ctx;

	port_context = (PPortContext)pOpenContext;
	ctx = (PSplitterContext)port_context->splitter_context;

	WaitForSingleObject(ctx->sem_port_list, INFINITE);

	if (port_context->prev != NULL) {
		port_context->prev->next = port_context->next;
	}

	if (port_context->next != NULL) {
		port_context->next->prev = port_context->prev;
	} else {
		ctx->port_list_head = port_context->prev;
	}

	if (ctx->port_list_head == NULL) {
		ctx->real_port_functions.COM_Close(ctx->real_port_handle);
		ctx->real_port_handle = INVALID_HANDLE_VALUE;
	}

	ReleaseSemaphore(ctx->sem_port_list, 1, NULL);

	free_port_context(port_context);

	return TRUE;
}

DRIVER_FUNCTION ULONG
COM_Read(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	ULONG		read;
	ULONG		copy;
	ULONG		result;
#ifndef READ_COMPLETES_IMMEDEATLY
	DWORD		wait_result;
#endif

	PPortContext		port;
	PPortContext		iterator;
	PSplitterContext	ctx;

	port = (PPortContext)pOpenContext;
	ctx = (PSplitterContext)port->splitter_context;

	result = 0;

	while (result != BufferLength) {
		WaitForSingleObject(port->sem_cb, INFINITE);
		copy = port->cb_ctx->pop(pTargetBuffer + result, BufferLength - result);
		ReleaseSemaphore(port->sem_cb, 1, NULL);

		if (copy == 0) {
			// try to read
			WaitForSingleObject(ctx->sem_read, INFINITE);
			read = ctx->real_port_functions.COM_Read(ctx->real_port_handle, pTargetBuffer + result, BufferLength - result);
			if (read == 0) {
				ReleaseSemaphore(ctx->sem_read, 1, NULL);
#ifndef READ_COMPLETES_IMMEDEATLY
				wait_result = WaitForSingleObject(port->ev_data, READ_WAIT_TIMEOUT);
				if (wait_result == WAIT_OBJECT_0) continue;
#endif
				break;
			} else {
				// push data to all other clients
				// foreach exept current port
				WaitForSingleObject(ctx->sem_port_list, INFINITE);
				for (iterator = ctx->port_list_head; iterator != NULL; iterator = iterator->prev) {
					if (iterator == port) continue;

					WaitForSingleObject(iterator->sem_cb, INFINITE);
					iterator->cb_ctx->push(pTargetBuffer + result, read);
					ReleaseSemaphore(iterator->sem_cb, 1, NULL);
#ifndef READ_COMPLETES_IMMEDEATLY
					SetEvent(iterator->ev_data);
#endif
				}
				ReleaseSemaphore(ctx->sem_port_list, 1, NULL);

				result += read;
			}
			ReleaseSemaphore(ctx->sem_read, 1, NULL);
		} else {
			result += copy;
		}
	}

	return result;
}

// XXX - only one client should write to port
DRIVER_FUNCTION ULONG
COM_Write(PVOID pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	PPortContext		port_context;
	PSplitterContext	ctx;

	port_context = (PPortContext)pOpenContext;
	ctx = (PSplitterContext)port_context->splitter_context;

	return ctx->real_port_functions.COM_Write(ctx->real_port_handle, pSourceBytes, NumberOfBytes);
}

// me do nothing, just call apropriate function
DRIVER_FUNCTION ULONG
COM_Seek(PVOID pOpenContext, LONG Position, DWORD Type) {
	PPortContext		port_context;
	PSplitterContext	ctx;

	port_context = (PPortContext)pOpenContext;
	ctx = (PSplitterContext)port_context->splitter_context;

	if (ctx->real_port_functions.COM_Seek == NULL) return 0;

	return ctx->real_port_functions.COM_Seek(ctx->real_port_handle, Position, Type);
}

// me do nothing, just call apropriate function
DRIVER_FUNCTION BOOL
COM_PowerUp(PVOID pInitContext) {
	PSplitterContext	ctx;

	ctx = (PSplitterContext)pInitContext;

	return ctx->real_port_functions.COM_PowerUp(ctx->real_port_init_context);
}

// me do nothing, just call apropriate function
DRIVER_FUNCTION BOOL
COM_PowerDown(PVOID pInitContext) {
	PSplitterContext	ctx;

	ctx = (PSplitterContext)pInitContext;

	return ctx->real_port_functions.COM_PowerDown(ctx->real_port_init_context);
}

DRIVER_FUNCTION BOOL
COM_IOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
	// XXX - only one client should control port
	PPortContext		port_context;
	PSplitterContext	ctx;

	port_context = (PPortContext)pOpenContext;
	ctx = (PSplitterContext)port_context->splitter_context;

	return ctx->real_port_functions.COM_IOControl(ctx->real_port_handle, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
}

BOOL
init_context(PSplitterContext context) {
	memset(context, 0, sizeof(SplitterContext));

	context->real_port_handle = INVALID_HANDLE_VALUE;
	context->sem_port_list = CreateSemaphore(NULL, 1, 1, NULL);
	context->sem_read = CreateSemaphore(NULL, 1, 1, NULL);

	return TRUE;
}

BOOL
deinit_context(PSplitterContext context) {
	// XXX - real port must be closed

	CloseHandle(context->sem_port_list);
	CloseHandle(context->sem_read);

	return TRUE;
}

BOOL
load_real_driver(PSplitterContext ctx, PSplitterSettings settings, ULONG Identifier) {
	ctx->real_port_driver = LoadDriver(settings->real_driver_path);
	if (ctx->real_port_driver != NULL) {
		if (get_real_driver_functions(ctx)) {
			ctx->real_port_init_context = ctx->real_port_functions.COM_Init(Identifier);
			if (ctx->real_port_init_context != NULL) {
				return TRUE;
			} else {
				MessageBox(NULL, L"failed to init real driver", L"splitter driver", MB_OK | MB_TOPMOST);
			}
		} else {
			MessageBox(NULL, L"failed to get driver functions", L"splitter driver", MB_OK | MB_TOPMOST);
		}

		FreeLibrary(ctx->real_port_driver);
	} else {
		MessageBox(NULL, L"failed to load real driver dll", L"splitter driver", MB_OK | MB_TOPMOST);
	}

	return FALSE;
}

BOOL
get_real_driver_functions(PSplitterContext ctx) {
	BOOL	result;

	result = FALSE;

	do {
		GET_FUNCTION(ctx, COM_Init, TRUE);
		GET_FUNCTION(ctx, COM_PreDeinit, FALSE);
		GET_FUNCTION(ctx, COM_Deinit, TRUE);
		GET_FUNCTION(ctx, COM_Open, TRUE);
		GET_FUNCTION(ctx, COM_Close, TRUE);
		GET_FUNCTION(ctx, COM_Read, TRUE);
		GET_FUNCTION(ctx, COM_Write, TRUE);
		GET_FUNCTION(ctx, COM_Seek, FALSE);
		GET_FUNCTION(ctx, COM_PowerUp, TRUE);
		GET_FUNCTION(ctx, COM_PowerDown, TRUE);
		GET_FUNCTION(ctx, COM_IOControl, TRUE);

		result = TRUE;
	} while (0);

	return result;
}

BOOL
unload_real_driver(PSplitterContext ctx) {
	ctx->real_port_functions.COM_Deinit(ctx->real_port_init_context);

	FreeLibrary(ctx->real_port_driver);

	return TRUE;
}

PPortContext
create_port_context(void) {
	PPortContext	result;

	result = (PPortContext)LocalAlloc(LMEM_ZEROINIT, sizeof(PortContext));
	if (result != NULL) {
#ifndef READ_COMPLETES_IMMEDEATLY
		result->ev_data = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (result != NULL) {
#endif
		result->cb_ctx = new CCircBuffer; // also i'll get exception if new failed :)
		if (result->cb_ctx != NULL) {
			if (result->cb_ctx->set_buffer_size(INTERNAL_BUFFER_INITIAL_SIZE)) {
				result->sem_cb = CreateSemaphore(NULL, 1, 1, NULL);
#ifdef STORE_OPENER_ID
				result->opener_id = GetOwnerProcess();
#endif

				return result;
			}
			delete result->cb_ctx;
		}
#ifndef READ_COMPLETES_IMMEDEATLY
			CloseHandle(result->ev_data);
		}
#endif
	}

	return NULL;
}

void
free_port_context(PPortContext port_context) {
	delete port_context->cb_ctx;

#ifndef READ_COMPLETES_IMMEDEATLY
	CloseHandle(port_context->ev_data);
#endif
	CloseHandle(port_context->sem_cb);

	LocalFree(port_context);
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