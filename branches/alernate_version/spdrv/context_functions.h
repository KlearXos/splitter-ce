#ifndef __CONTEXT_FUNCTIONS_H_16022011__
#define __CONTEXT_FUNCTIONS_H_16022011__

#include <windows.h>

// direct driver load feature is not supported currently
// to enable this feature using DIRECT_DRIVER_LOAD_ENABLED should be defined

#include "real_port.h"
#include "open_port_context.h"
#include "..\common\splitter_settings.h"

#define SPLITTER_STATE_FREE					0
#define SPLITTER_STATE_IN_USE				1
#define SPLITTER_STATE_DELETE_PENDING		2

typedef struct __port_list_item {
	struct __port_list_item		*next;
	struct __port_list_item		*prev;

	PPortOpenContext			port_context;
} PortListItem, *PPortListItem;

// global splitter context
typedef struct __splitter_context {
	DWORD		context_state;

#ifdef DIRECT_DRIVER_LOAD_ENABLED
	// original driver
	HINSTANCE		*real_driver;
	PortFunctions	real_port_functions;
#endif

	// we are not loading driver. let device manager do it
	//HANDLE		real_init_context;
	HANDLE		real_open_context;

	// virtual drivers list
	HANDLE			sem_port_list;
	// next == NULL	
	PPortListItem	list_head;

	// settings
	SplitterSettings	settings;

	// real port reader
	HANDLE		waiter_thread;
	HANDLE		waiter_stop_event;

	void		*reader_buffer;
	DWORD		readef_buffer_size;

} SplitterContext, *PSplitterContext;

// nonzero (TRUE) if success
// 0 if failed
BOOL	InitContext(PSplitterContext ctx, ULONG Identifier);

BOOL	DeinitContext(PSplitterContext ctx);

BOOL	AddOpenPortContextToSplitter(PSplitterContext ctx, PPortOpenContext port_context); 
BOOL	RemovePortContextFromSplitter(PPortOpenContext port_context);

// nonzero (TRUE) if context could be deleted
// zero if not
BOOL	ClearContext(PSplitterContext ctx);

// move this functions to separate file

#endif