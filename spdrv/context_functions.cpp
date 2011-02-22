#include "context_functions.h"
#include "real_port_provider.h"

#define MAX_READER_BUFFER_SIZE	1024

BOOL	GetSettings(PSplitterContext ctx, ULONG Identifier);

BOOL	InitContextResources(PSplitterContext ctx, ULONG Identifier);
BOOL	DeinitContextResources(PSplitterContext ctx);

BOOL	ConnectToDriver(PSplitterContext ctx);
BOOL	DisconnectFromDriver(PSplitterContext ctx);

BOOL	StartSplitter(PSplitterContext ctx);
BOOL	StopSplitter(PSplitterContext ctx);

BOOL	CreateSystemObjects(PSplitterContext ctx);
BOOL	DeleteSystemObjects(PSplitterContext ctx);

BOOL	OpenRealPort(PSplitterContext ctx);
BOOL	CloseRealPort(PSplitterContext ctx);
BOOL	ConfigRealPort(PSplitterContext ctx);

DWORD WINAPI	WaiterThread(PSplitterContext ctx);

DWORD	GetRxDataSize(PSplitterContext ctx);
DWORD	ReadRxData(PSplitterContext ctx, DWORD data_size);

BOOL	SetRealPortWaitMask(PSplitterContext ctx, DWORD mask);

BOOL	WaitRealComEvent(PSplitterContext ctx, DWORD *mask);

BOOL
InitContext(PSplitterContext ctx, ULONG Identifier) {
	if (ctx == NULL) {
		return FALSE;
	}

	if (InitContextResources(ctx, Identifier)) {
		if (ConnectToDriver(ctx)) {
			return TRUE;
		}

		DeinitContextResources(ctx);
	}


	return FALSE;
}

BOOL
DeinitContext(PSplitterContext ctx) {
	DisconnectFromDriver(ctx);

	DeinitContextResources(ctx);

	return TRUE;
}

// load splitter settings
BOOL
GetSettings(PSplitterContext ctx, ULONG Identifier) {
	BOOL	result;
	memset(&ctx->settings, 0, sizeof(ctx->settings));

	result = LoadSplitterSettings(&ctx->settings);

	return result;
}

// allocate memory, create system objects
BOOL
InitContextResources(PSplitterContext ctx, ULONG Identifier) {
	ctx->list_head = NULL;
	ctx->waiter_stop_event = NULL;
	ctx->waiter_thread = NULL;

#ifdef DIRECT_DRIVER_LOAD_ENABLED
#error Not implemented
#else

	ctx->real_open_context = INVALID_HANDLE_VALUE;

#endif
	ctx->list_head = NULL;

	if (GetSettings(ctx, Identifier)) {
		if (CreateSystemObjects(ctx)) {
			return TRUE;
		}
	}

	return FALSE;
}

BOOL
DeinitContextResources(PSplitterContext ctx) {
	DeleteSystemObjects(ctx);

	return TRUE;
}

BOOL
ConnectToDriver(PSplitterContext ctx) {
	if (OpenRealPort(ctx)) {
		if (ConfigRealPort(ctx)) {
			if (StartSplitter(ctx)) {
				return TRUE;
			}
		}
		
		CloseRealPort(ctx);
	}
	return FALSE;
}

BOOL
DisconnectFromDriver(PSplitterContext ctx) {
	StopSplitter(ctx);

	CloseRealPort(ctx);

	return TRUE;
}

BOOL
StartSplitter(PSplitterContext ctx) {
	ctx->reader_buffer = LocalAlloc(LMEM_ZEROINIT, MAX_READER_BUFFER_SIZE);
	if (ctx->reader_buffer != NULL) {
		ctx->readef_buffer_size = MAX_READER_BUFFER_SIZE;

		ctx->waiter_stop_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (ctx->waiter_stop_event != NULL) {
			ctx->waiter_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WaiterThread, (LPVOID)ctx, 0, NULL);
			if (ctx->waiter_thread != NULL) {
				return TRUE;
			}

			CloseHandle(ctx->waiter_stop_event);
		}

		LocalFree(ctx->reader_buffer);
	}

	return FALSE;
}

BOOL
StopSplitter(PSplitterContext ctx) {
	SetEvent(ctx->waiter_stop_event);
	SetRealPortWaitMask(ctx, 0);

	WaitForSingleObject(ctx->waiter_thread, INFINITE);

	CloseHandle(ctx->waiter_stop_event);
	CloseHandle(ctx->waiter_thread);

	LocalFree(ctx->reader_buffer);

#ifdef DEBUG
	ctx->waiter_thread		= (HANDLE)0xdeadfeed;
	ctx->waiter_stop_event	= (HANDLE)0xdeadfeed;
	ctx->reader_buffer		= (HANDLE)0xdeadfeed;
#endif

	return TRUE;
}

BOOL
CreateSystemObjects(PSplitterContext ctx) {
	ctx->sem_port_list = CreateSemaphore(NULL, 1, 1, NULL);
	if (ctx->sem_port_list != NULL) {
		return TRUE;
	}

	return FALSE;
}

BOOL
DeleteSystemObjects(PSplitterContext ctx) {
	CloseHandle(ctx->sem_port_list);

	return TRUE;
}

DWORD WINAPI
WaiterThread(PSplitterContext ctx) {
	BOOL	result;
	DWORD	event_mask;
	DWORD	data_size;

	PPortListItem list_item;

	while (WaitForSingleObject(ctx->waiter_stop_event, 0) != WAIT_OBJECT_0) {
		event_mask = 0;
		// 
		result = WaitRealComEvent(ctx, &event_mask);

		if (result && (event_mask != 0)) {
			// react on event
			WaitForSingleObject(ctx->sem_port_list, INFINITE);

			// notify each port about events
			list_item = ctx->list_head;

			while (list_item != NULL) {
				RealPortEventOccured(list_item->port_context, event_mask);
				list_item = list_item->prev;
			}

			ReleaseSemaphore(ctx->sem_port_list, 1, NULL);

			if (event_mask & EV_RXCHAR | EV_RXFLAG) {
				// new data available
				while ((data_size = GetRxDataSize(ctx)) != 0) {
					if (data_size > MAX_READER_BUFFER_SIZE) {
						data_size = MAX_READER_BUFFER_SIZE;
					}

					data_size = ReadRxData(ctx, data_size);
					// add data to ports
					WaitForSingleObject(ctx->sem_port_list, INFINITE);

					list_item = ctx->list_head;
					while (list_item != NULL) {
						DataReceived(list_item->port_context, ctx->reader_buffer, data_size);
						list_item = list_item->prev;
					}

					ReleaseSemaphore(ctx->sem_port_list, 1, NULL);
				}
			}
		} else {
			break;
		}
	}

	return 0;
}

BOOL
OpenRealPort(PSplitterContext ctx) {
#ifdef DIRECT_DRIVER_LOAD_ENABLED
#error Not implemented
#else
	wchar_t port_name[] = L"\\$device\\COMxxx";

	if (ctx->settings.real_port_number < 10) {
		wsprintf(port_name, L"COM%d:", ctx->settings.real_port_number);
	} else {
		wsprintf(&port_name[12], L"%d", ctx->settings.real_port_number);
	}

	ctx->real_open_context = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 
	    0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	return (ctx->real_open_context != INVALID_HANDLE_VALUE);
#endif
}

BOOL
CloseRealPort(PSplitterContext ctx) {
#ifdef DIRECT_DRIVER_LOAD_ENABLED
#error Not implemented
#else
	CloseHandle(ctx->real_open_context);

#ifdef DEBUG
	ctx->real_open_context = (HANDLE)0xdeadfeed;
#endif
	return TRUE;
#endif
}

BOOL
ConfigRealPort(PSplitterContext ctx) {
	DWORD	wait_mask;

	wait_mask = EV_TXEMPTY | EV_RXFLAG | EV_RXCHAR | EV_RLSD | EV_RING | EV_ERR 
	    | EV_DSR | EV_CTS | EV_BREAK;

	// XXX
	return SetRealPortWaitMask(ctx, wait_mask);
}

// XXX - no error handling
DWORD
GetRxDataSize(PSplitterContext ctx) {
	SERIAL_DEV_STATUS	device_status;
	DWORD				bytes_returned;
	BOOL				result;

	memset(&device_status, 0, sizeof(device_status));

	bytes_returned = 0;
	result = Call_COM_IOControl((LPVOID)ctx, IOCTL_SERIAL_GET_COMMSTATUS, 
	    NULL, 0, (PBYTE)&device_status, sizeof(device_status), &bytes_returned);
	if (result && (bytes_returned == sizeof(device_status))) {
		return device_status.ComStat.cbInQue;
	}

	return 0;
}

// XXX - no error handling
DWORD
ReadRxData(PSplitterContext ctx, DWORD data_size) {
	DWORD	bytes_read;
	BOOL	result;

	bytes_read = 0;
	if (data_size != 0) {
		// XXX
		result = Call_COM_Read((LPVOID)ctx, ctx->reader_buffer, data_size, &bytes_read);
		if (!result) {
			bytes_read = 0;
		}
	}

	return bytes_read;
}

BOOL
AddOpenPortContextToSplitter(PSplitterContext ctx, PPortOpenContext port_context) {
	PPortListItem	list_item;

	list_item = (PPortListItem)LocalAlloc(LMEM_ZEROINIT, sizeof(PortListItem));
	if (list_item == NULL) {
		return FALSE;
	}

	list_item->port_context = port_context;
	port_context->splitter_context = (void*)ctx;
	port_context->list_position = (void*)list_item;

	WaitForSingleObject(ctx->sem_port_list, INFINITE);

	list_item->prev = ctx->list_head;
	ctx->list_head->next = list_item;
	ctx->list_head = list_item;

	ReleaseSemaphore(ctx->sem_port_list, 1, NULL);

	return TRUE;
}

BOOL
RemovePortContextFromSplitter(PPortOpenContext port_context) {
	PSplitterContext	ctx;
	PPortListItem		list_item;

	list_item = (PPortListItem)port_context->list_position;
	ctx = (PSplitterContext)port_context->splitter_context;

	if (list_item != NULL ) {

		WaitForSingleObject(ctx->sem_port_list, INFINITE);

		if (list_item->prev != NULL) {
			list_item->prev->next = list_item->next;
		}

		if (list_item->next != NULL) {
			list_item->next->prev = list_item->prev;
		} else {
			ctx->list_head = list_item->prev;
		}

		ReleaseSemaphore(ctx->sem_port_list, 1, NULL);

		LocalFree(list_item);
		port_context->list_position = NULL;
	}

	return TRUE;
}

BOOL
SetRealPortWaitMask(PSplitterContext ctx, DWORD mask) {
	return Call_COM_IOControl(ctx, IOCTL_SERIAL_SET_WAIT_MASK, (PBYTE)&mask, sizeof(DWORD), NULL, 0, NULL);
}

BOOL
WaitRealComEvent(PSplitterContext ctx, DWORD *mask) {
	DWORD	bytes_returned;
	BOOL	result;

	bytes_returned = 0;

	result = Call_COM_IOControl(ctx, IOCTL_SERIAL_WAIT_ON_MASK, NULL, 0, (PBYTE)mask, sizeof(DWORD), &bytes_returned);

	if (bytes_returned != sizeof(DWORD)) {
		result = FALSE;
	}

	return result;
}