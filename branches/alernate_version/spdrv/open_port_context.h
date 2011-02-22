#ifndef __OPEN_PORT_CONTEXT_16022011__
#define __OPEN_PORT_CONTEXT_16022011__

#include <windows.h>

#include "cbuffer.h"

typedef struct __port_open_context {
	void	*splitter_context;
	void	*list_position;

	// only one reading is possible
	HANDLE	sem_reading;
	HANDLE	event_data;
	// ring buffer semafor
	HANDLE	sem_buffer;
	// ring buffer
	CCircBuffer	*buffer;

	BOOL		read_aborted;
	// event
	HANDLE	event_wait;
	DWORD	*event_wait_destination;

	// event mask
	DWORD	wait_mask;

	COMMTIMEOUTS	virtual_port_timeouts;
} PortOpenContext, *PPortOpenContext;

BOOL	InitOpenPortContex(PPortOpenContext port_context);

BOOL	DeinitOpenPortContext(PPortOpenContext port_context);

ULONG	ReadFromVirtualPort(PPortOpenContext pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength);
ULONG	WriteToVirtualPort(PPortOpenContext pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes);
ULONG	VirtualPortSeek(PPortOpenContext pOpenContext, LONG Position, DWORD Type);

BOOL	VirtualPortIOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

BOOL	RealPortEventOccured(PPortOpenContext port_context, DWORD event_mask);

BOOL	DataReceived(PPortOpenContext port_contex, void *buffer, DWORD data_size);

#endif