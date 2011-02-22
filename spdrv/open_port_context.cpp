#include "open_port_context.h"

#include "real_port_provider.h"

#define READ_TIMEOUT				250
#define READ_TIMEOUT_MULTIPLIER		10
#define READ_TIMEOUT_CONSTANT		100
#define WRITE_TIMEOUT_MULTIPLIER	10
#define WRITE_TIMEOUT_CONSTANT		100

#define DEFAULT_RX_BUFFER_SIZE		4096

BOOL	CreatePortSystemObjects(PPortOpenContext port_context);
BOOL	ClosePortSystemObjects(PPortOpenContext port_context);

BOOL	WaitForVirtualCommEvent(PPortOpenContext port_context, DWORD *out);

BOOL	InitDefaultPortSettings(PPortOpenContext port_context);

BOOL
InitOpenPortContex(PPortOpenContext port_context) {
	port_context->splitter_context = NULL;
	port_context->list_position = NULL;
	port_context->wait_mask = 0;
	port_context->event_wait_destination = NULL;
	port_context->sem_reading = NULL;
	port_context->sem_buffer = NULL;
	port_context->buffer = NULL;
	port_context->read_aborted = FALSE;

	if (CreatePortSystemObjects(port_context)) {
		if (InitDefaultPortSettings(port_context)) {
			port_context->buffer = new CCircBuffer();
			if (port_context->buffer != NULL) {
				if (port_context->buffer->set_buffer_size(DEFAULT_RX_BUFFER_SIZE)) {
					return TRUE;
				}

				delete port_context->buffer;
			}
		}
		ClosePortSystemObjects(port_context);
	}

	return FALSE;
}

BOOL
DeinitOpenPortContext(PPortOpenContext port_context) {
	// abort any possible read
	port_context->read_aborted = TRUE;
	SetEvent(port_context->event_data);

	RealPortEventOccured(port_context, 0);

	ClosePortSystemObjects(port_context);

	delete port_context->buffer;

	return TRUE;
}

BOOL
CreatePortSystemObjects(PPortOpenContext port_context) {
	port_context->event_wait = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (port_context->event_wait != NULL) {
		port_context->sem_buffer = CreateSemaphore(NULL, 1, 1, NULL);
		if (port_context->sem_buffer != NULL) {
			port_context->event_data = CreateEvent(NULL, FALSE, FALSE, NULL);
			if (port_context->event_data != NULL) {
				port_context->sem_reading = CreateSemaphore(NULL, 1, 1, NULL);
				if (port_context->sem_reading != NULL) {
					return TRUE;
				}

				CloseHandle(port_context->event_data);
			}

			CloseHandle(port_context->sem_buffer);
		}

		CloseHandle(port_context->event_wait);
	}

	return FALSE;
}

BOOL
ClosePortSystemObjects(PPortOpenContext port_context) {
	CloseHandle(port_context->event_wait);
	CloseHandle(port_context->sem_buffer);
	CloseHandle(port_context->event_data);
	CloseHandle(port_context->sem_reading);

	return TRUE;
}

BOOL
RealPortEventOccured(PPortOpenContext port_context, DWORD event_mask) {
	DWORD	target_events;
	DWORD	*destination;

	target_events = event_mask & port_context->wait_mask;
	if ((target_events != 0) || (event_mask == 0)) {
		do {
			destination = port_context->event_wait_destination;
			if (destination == NULL) {
				break;
			}

		} while(InterlockedCompareExchangePointer(&(port_context->event_wait_destination), NULL, destination) != destination);

		if (destination != NULL) {
			*destination = target_events;
			SetEvent(port_context->event_wait);
		}
	}

	return TRUE;
}

BOOL
WaitForVirtualCommEvent(PPortOpenContext port_context, DWORD *out) {
	volatile DWORD		wait_result;
	DWORD				*old_value;

	wait_result = 0;
	old_value = (DWORD*)InterlockedCompareExchangePointer(&(port_context->event_wait_destination), &wait_result, NULL);
	if (old_value != NULL) {
		return FALSE;
	}

	WaitForSingleObject(port_context->event_wait, INFINITE);
	*out = wait_result;

	return TRUE;
}


// NOTE: timeouts
ULONG
ReadFromVirtualPort(PPortOpenContext pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	DWORD			start_time;
	DWORD			elapsed_time;

	ULONG			result;
	ULONG			copied;

	DWORD			wait_result;
	DWORD			wait_time;

	DWORD			error;

	COMMTIMEOUTS	current_timeouts;
	DWORD			total_timeout;

	WaitForSingleObject(pOpenContext->sem_reading, INFINITE);

	if (pOpenContext->read_aborted) {
		ReleaseSemaphore(pOpenContext->sem_reading, 1, NULL);

		SetLastError(ERROR_CANCELLED);
		return 0;
	}

	memcpy(&current_timeouts, &pOpenContext->virtual_port_timeouts, sizeof(current_timeouts));

	if (current_timeouts.ReadTotalTimeoutMultiplier == MAXDWORD) {
		total_timeout = MAXDWORD;
	} else {
		if (current_timeouts.ReadTotalTimeoutMultiplier == 0) {
			total_timeout = current_timeouts.ReadTotalTimeoutConstant;
		} else if (current_timeouts.ReadTotalTimeoutMultiplier < BufferLength) {
			if (BufferLength < (MAXDWORD - current_timeouts.ReadTotalTimeoutConstant)/current_timeouts.ReadTotalTimeoutMultiplier) {
				total_timeout = current_timeouts.ReadTotalTimeoutConstant +
					current_timeouts.ReadTotalTimeoutMultiplier * BufferLength;
			} else {
				total_timeout = MAXDWORD;
			}
		} else {
			if (current_timeouts.ReadTotalTimeoutMultiplier < (MAXDWORD - current_timeouts.ReadTotalTimeoutConstant)/BufferLength) {
				total_timeout = current_timeouts.ReadTotalTimeoutConstant +
					current_timeouts.ReadTotalTimeoutMultiplier * BufferLength;
			} else {
				total_timeout = MAXDWORD;
			}
		}
	}

	result = 0;
	error = ERROR_SUCCESS;

	start_time = GetTickCount();

	while (result != BufferLength) {
		WaitForSingleObject(pOpenContext->sem_buffer, INFINITE);

		if (pOpenContext->buffer->is_empty()) {
			ReleaseSemaphore(pOpenContext->sem_buffer, 1, NULL);
			
			if (total_timeout == 0) {
				if (current_timeouts.ReadIntervalTimeout == MAXDWORD) {
					/* A value of MAXDWORD, combined with zero values for both 
					  the ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier 
					  members, specifies that the read operation is to return 
					  immediately with the bytes that have already been received, 
					  even if no bytes have been received.
					*/
					break;
				} else if (current_timeouts.ReadIntervalTimeout == 0) {
					// no timeouts in use. wait for ever
					wait_time = INFINITE;
				} else {
					// only interval timeout is used
					// note: interval timeout is used after first byte received
					wait_time = (result == 0) ? INFINITE : current_timeouts.ReadIntervalTimeout;
				}
			} else if ((current_timeouts.ReadIntervalTimeout == MAXDWORD) && 
			    (current_timeouts.ReadTotalTimeoutMultiplier == MAXDWORD) &&
			    (current_timeouts.ReadTotalTimeoutConstant != 0) &&
			    (current_timeouts.ReadTotalTimeoutConstant < MAXDWORD)) {
				if (result != 0) {
					break;
				} else {
					wait_time = current_timeouts.ReadTotalTimeoutConstant;
				}
			} else {
				elapsed_time = GetTickCount() - start_time;
				if (elapsed_time > total_timeout) {
					/* we are timed out */
					error = ERROR_TIMEOUT;
					break;
				}

				wait_time = total_timeout - elapsed_time;
				if (result && (current_timeouts.ReadIntervalTimeout != 0)) {
					if (wait_time > current_timeouts.ReadIntervalTimeout) {
						wait_time = current_timeouts.ReadIntervalTimeout;
					}
				}
			}

			wait_result = WaitForSingleObject(pOpenContext->event_data, 
			    wait_time);
			if (wait_result == WAIT_TIMEOUT) {
				error = ERROR_TIMEOUT;
				break;
			}

			if (pOpenContext->read_aborted) {
				error = ERROR_CANCELLED;
				break;
			}

		} else {
			copied = pOpenContext->buffer->pop(pTargetBuffer + result, BufferLength - result);
			ReleaseSemaphore(pOpenContext->sem_buffer, 1, NULL);

			result += copied;
		}
	}

	ReleaseSemaphore(pOpenContext->sem_reading, 1, NULL);

	SetLastError(error);
	return result;
}

ULONG
WriteToVirtualPort(PPortOpenContext pOpenContext, PUCHAR pSourceBytes, ULONG NumberOfBytes) {
	return Call_COM_Write(pOpenContext->splitter_context, pSourceBytes, NumberOfBytes);
}

ULONG
VirtualPortSeek(PPortOpenContext pOpenContext, LONG Position, DWORD Type) {
	return (ULONG)-1;
}

BOOL
VirtualPortIOControl(PVOID pOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut) {
// XXX not shure what to do here. additional research and debug required
	BOOL	result;

	PPortOpenContext port_context = (PPortOpenContext)pOpenContext;

	result = TRUE;

	switch (dwCode) {
	//case IOCTL_SERIAL_SET_BREAK_ON:
	//case IOCTL_SERIAL_SET_BREAK_OFF:
	//case IOCTL_SERIAL_SET_DTR:
	//case IOCTL_SERIAL_CLR_DTR:
	//case IOCTL_SERIAL_SET_RTS:
	//case IOCTL_SERIAL_CLR_RTS:
	//case IOCTL_SERIAL_SET_XOFF:
	//case IOCTL_SERIAL_SET_XON:
	case IOCTL_SERIAL_GET_WAIT_MASK:
		if ( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) ||
		    (NULL == pdwActualOut) ) {
			SetLastError (ERROR_INVALID_PARAMETER);
			result = FALSE;
			break;
		}

		*(DWORD *)pBufOut = port_context->wait_mask;

		break;
	case IOCTL_SERIAL_SET_WAIT_MASK:
		if ( (dwLenIn < sizeof(DWORD)) || (pBufIn == NULL)) {
			SetLastError (ERROR_INVALID_PARAMETER);
			result = FALSE;
			break;
		}

		port_context->wait_mask = *(DWORD *)pBufIn;
		RealPortEventOccured(port_context, 0);

		break;
	case IOCTL_SERIAL_WAIT_ON_MASK:
		if ((dwLenOut < sizeof(DWORD) || (pBufOut == NULL) || (pdwActualOut == NULL))) {
			SetLastError (ERROR_INVALID_PARAMETER);
			result = FALSE;
			break;
		}

		result = WaitForVirtualCommEvent(port_context, (DWORD *)pBufOut);
		break;
	//case IOCTL_SERIAL_GET_COMMSTATUS:
	//case IOCTL_SERIAL_GET_MODEMSTATUS:
	//case IOCTL_SERIAL_GET_PROPERTIES:
	case IOCTL_SERIAL_SET_TIMEOUTS:
		if ((dwLenIn < sizeof(COMMTIMEOUTS)) || (pBufIn == NULL)) {
			SetLastError (ERROR_INVALID_PARAMETER);
			result = FALSE;
			break;
		}

		memcpy(&port_context->virtual_port_timeouts, (COMMTIMEOUTS*)pBufIn, sizeof(port_context->virtual_port_timeouts));
		break;
	case IOCTL_SERIAL_GET_TIMEOUTS:
		if ((dwLenOut < sizeof(COMMTIMEOUTS)) || (pBufOut == NULL)) {
			SetLastError (ERROR_INVALID_PARAMETER);
			result = FALSE;
			break;
		}

		memcpy(pBufOut, &port_context->virtual_port_timeouts, sizeof(port_context->virtual_port_timeouts));

		break;
	//case IOCTL_SERIAL_PURGE:
	case IOCTL_SERIAL_SET_QUEUE_SIZE:
		break;
	//case IOCTL_SERIAL_IMMEDIATE_CHAR:
	//case IOCTL_SERIAL_GET_DCB:
	//case IOCTL_SERIAL_SET_DCB:
	//case IOCTL_SERIAL_ENABLE_IR:
	//case IOCTL_SERIAL_DISABLE_IR:
	default:
		result = Call_COM_IOControl(port_context->splitter_context, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
		break;
	}

	return result;
}

BOOL
DataReceived(PPortOpenContext port_context, void *buffer, DWORD data_size) {
	DWORD	free_space;
	DWORD	buffer_size;

	if ((buffer != NULL) && (data_size != 0)) {
		WaitForSingleObject(port_context->sem_buffer, INFINITE);

		buffer_size = port_context->buffer->get_buffer_size();
		if (data_size > buffer_size) {
			// some data will not be written to buffer
			buffer = (PVOID)((PUCHAR)buffer + data_size - buffer_size);
			data_size = buffer_size;
		}

		free_space = port_context->buffer->get_free_space_size();
		if (free_space < data_size) {
			port_context->buffer->del(data_size - free_space);
		}

		port_context->buffer->push(buffer, data_size);

		ReleaseSemaphore(port_context->sem_buffer, 1, NULL);

		SetEvent(port_context->event_data);
	}

	return TRUE;
}

BOOL
InitDefaultPortSettings(PPortOpenContext port_context) {
	// set default timeouts
	port_context->virtual_port_timeouts.ReadIntervalTimeout = READ_TIMEOUT;
	port_context->virtual_port_timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_CONSTANT;
	port_context->virtual_port_timeouts.ReadTotalTimeoutMultiplier = READ_TIMEOUT_MULTIPLIER;
	port_context->virtual_port_timeouts.WriteTotalTimeoutConstant = WRITE_TIMEOUT_CONSTANT;
	port_context->virtual_port_timeouts.WriteTotalTimeoutMultiplier = WRITE_TIMEOUT_MULTIPLIER;

	return TRUE;
}