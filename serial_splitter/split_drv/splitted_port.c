#include "splitted_port.h"

#include "allocation.h"

#include "cb_c.h"

typedef struct __splitted_port_context {
	BOOL		active;
	LONG		referenceCount;

	DWORD		access_rights;

	// only one read operation is accepted from client in one time
	// other operations should wait
	HANDLE		readSemaphore;

	BOOL		readAborted;

	HANDLE		dataBufferSemaphore;
	HANDLE		dataReceivedEvent;
	void		*dataBuffer;

	HANDLE		settingsSemaphore;
	COMMTIMEOUTS	timeouts;
}SplittedPort, *PSplittedPort;

static BOOL	CouldRead(PSplittedPort	context);

// reference context to prevent deleting
static BOOL	ReferenceSplittedPortContext(PSplittedPort context);

// dereference context, if reference count is 0 - delete context
static BOOL	DereferenceSplittedPortContext(PSplittedPort context);

static void LockReading(PSplittedPort context);
static void UnlockReading(PSplittedPort context);

static void LockDataBuffer(PSplittedPort context);
static void ReleaseDataBuffer(PSplittedPort context);

static BOOL		HaveDataToRead(PSplittedPort context);
static ULONG	ReadDataToBuffer(PSplittedPort context, PUCHAR buffer, ULONG toRead);

static void LockPortSettings(PSplittedPort context);
static void ReleasePortSettings(PSplittedPort context);

static void AbortRead(PSplittedPort context);
static void ClearAbortRead(PSplittedPort context);
static BOOL ReadAborted(PSplittedPort context);

static void		GetTimeouts(PSplittedPort context, COMMTIMEOUTS *timeouts);

BOOL
DataReceived(PVOID pOpenContext, PUCHAR	buffer, ULONG dataLength) {
	PSplittedPort	context = (PSplittedPort)pOpenContext;

	if (!ReferenceSplittedPortContext(context)) {
		// could not reference port, it could be closed
		return FALSE;
	}

	LockDataBuffer(context);

	cb_push(context->dataBuffer, buffer, dataLength);
	SetEvent(context->dataReceivedEvent);

	ReleaseDataBuffer(context);


	DereferenceSplittedPortContext(context);

	return TRUE;
}

ULONG
PortRead(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	PSplittedPort	context = (PSplittedPort)pOpenContext;

	DWORD	error = ERROR_SUCCESS;
	ULONG	result = (ULONG)-1;

	COMMTIMEOUTS	timeouts;
	DWORD			total_timeout;

	DWORD			elapsed_time;

	DWORD			wait_result;
	DWORD			wait_time;
	DWORD			startTickCount;

	if (NULL == context) {
		SetLastError(ERROR_INVALID_HANDLE);
		return (ULONG)-1;
	}

	if ((NULL == pTargetBuffer) || (0 == BufferLength)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return (ULONG)-1;
	}

	// reference
	if (!ReferenceSplittedPortContext(context)) {
		// could not reference port, it could be closed
		SetLastError(ERROR_INVALID_HANDLE);
		return (ULONG)-1;
	}

	GetTimeouts(context, &timeouts);

	if (timeouts.ReadTotalTimeoutMultiplier == MAXDWORD) {
		total_timeout = MAXDWORD;
	} else {
		if (timeouts.ReadTotalTimeoutMultiplier == 0) {
			total_timeout = timeouts.ReadTotalTimeoutConstant;
		} else if (timeouts.ReadTotalTimeoutMultiplier < BufferLength) {
			if (BufferLength < (MAXDWORD - timeouts.ReadTotalTimeoutConstant)/timeouts.ReadTotalTimeoutMultiplier) {
				total_timeout = timeouts.ReadTotalTimeoutConstant +
					timeouts.ReadTotalTimeoutMultiplier * BufferLength;
			} else {
				total_timeout = MAXDWORD;
			}
		} else {
			if (timeouts.ReadTotalTimeoutMultiplier < (MAXDWORD - timeouts.ReadTotalTimeoutConstant)/BufferLength) {
				total_timeout = timeouts.ReadTotalTimeoutConstant +
					timeouts.ReadTotalTimeoutMultiplier * BufferLength;
			} else {
				total_timeout = MAXDWORD;
			}
		}
	}

	if (CouldRead(context)) {
		result = 0;

		startTickCount = GetTickCount();

		// lock for read
		LockReading(context);

		while (result != BufferLength) {
			LockDataBuffer(context);

			if (HaveDataToRead(context)) {
				// read data
				result += ReadDataToBuffer(context, pTargetBuffer + result, BufferLength - result);

				ReleaseDataBuffer(context);
			} else {
				ResetEvent(context->dataReceivedEvent);

				ReleaseDataBuffer(context);
				// wait
				if (total_timeout == 0) {
					if (timeouts.ReadIntervalTimeout == MAXDWORD) {
						/* A value of MAXDWORD, combined with zero values for both 
						  the ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier 
						  members, specifies that the read operation is to return 
						  immediately with the bytes that have already been received, 
						  even if no bytes have been received.
						*/
						break;
					} else if (timeouts.ReadIntervalTimeout == 0) {
						// no timeouts in use. wait for ever
						wait_time = INFINITE;
					} else {
						// only interval timeout is used
						// note: interval timeout is used after first byte received
						wait_time = (result == 0) ? INFINITE : timeouts.ReadIntervalTimeout;
					}
				} else if ((timeouts.ReadIntervalTimeout == MAXDWORD) && 
					(timeouts.ReadTotalTimeoutMultiplier == MAXDWORD) &&
					(timeouts.ReadTotalTimeoutConstant != 0) &&
					(timeouts.ReadTotalTimeoutConstant < MAXDWORD)) {
					if (result != 0) {
						break;
					} else {
						wait_time = timeouts.ReadTotalTimeoutConstant;
					}
				} else {
					elapsed_time = GetTickCount() - startTickCount;
					if (elapsed_time > total_timeout) {
						/* we are timed out */
						error = ERROR_TIMEOUT;
						break;
					}

					wait_time = total_timeout - elapsed_time;
					if (result && (timeouts.ReadIntervalTimeout != 0)) {
						if (wait_time > timeouts.ReadIntervalTimeout) {
							wait_time = timeouts.ReadIntervalTimeout;
						}
					}
				}

				wait_result = WaitForSingleObject(context->dataReceivedEvent, 
					wait_time);
				if (wait_result == WAIT_TIMEOUT) {
					error = ERROR_TIMEOUT;
					break;
				}

				if (ReadAborted(context)) {
					error = ERROR_CANCELLED;
					break;
				}
			}
		}

		// unlock from read
		UnlockReading(context);
	} else {
		error = ERROR_INVALID_ACCESS;
	}

	// unreference
	DereferenceSplittedPortContext(context);

	SetLastError(error);

	return result;
}

BOOL
CouldRead(PSplittedPort	context) {
	return context->access_rights & GENERIC_READ;
}

// reference context to prevent deleting
BOOL	ReferenceSplittedPortContext(PSplittedPort context) {
	LONG	count;
	LONG	nextCount;
	BOOL	result = FALSE;

	while (context->active) {
		count = context->referenceCount;
		nextCount = count + 1;
		if (InterlockedCompareExchange(&context->referenceCount, nextCount, count) == count) {
			result = TRUE;
			break;
		}
	}

	return result;
}

// dereference context, if reference count is 0 - delete context
BOOL	DereferenceSplittedPortContext(PSplittedPort context) {
	LONG	count;
	LONG	nextCount;

	do {
		count = context->referenceCount;
		if (0 == count) {
			break;
		}

		nextCount = count - 1;

		if (InterlockedCompareExchange(&context->referenceCount, nextCount, count) == count) {
			break;
		}
	} while (TRUE);

	if ((!context->active) && (0 == context->referenceCount)) {
		FreeMemory(context);
	}

	return TRUE;
}

void LockReading(PSplittedPort context) {
	WaitForSingleObject(context->readSemaphore, INFINITE);
}

void UnlockReading(PSplittedPort context) {
	ReleaseSemaphore(context->readSemaphore, 1, NULL);
}

void
LockDataBuffer(PSplittedPort context) {
	WaitForSingleObject(context->dataBufferSemaphore, INFINITE);
}

void
ReleaseDataBuffer(PSplittedPort context) {
	ReleaseSemaphore(context->dataBufferSemaphore, 1, NULL);
}

BOOL
HaveDataToRead(PSplittedPort context) {
	return !cb_isempty(context->dataBuffer);
}

ULONG
ReadDataToBuffer(PSplittedPort context, PUCHAR buffer, ULONG toRead) {
	return cb_pop(context->dataBuffer, buffer, toRead);
}

void
GetTimeouts(PSplittedPort context, COMMTIMEOUTS *timeouts) {
	LockPortSettings(context);

	memcpy(timeouts, &context->timeouts, sizeof(COMMTIMEOUTS));

	ReleasePortSettings(context);
}

void
LockPortSettings(PSplittedPort context) {
	WaitForSingleObject(context->settingsSemaphore, INFINITE);
}

void
ReleasePortSettings(PSplittedPort context) {
	ReleaseSemaphore(context->settingsSemaphore, 1, NULL);
}

void
AbortRead(PSplittedPort context) {
	context->readAborted = TRUE;
	SetEvent(context->dataReceivedEvent);
}

void
ClearAbortRead(PSplittedPort context) {
	context->readAborted = FALSE;
}

BOOL
ReadAborted(PSplittedPort context) {
	return context->readAborted;
}