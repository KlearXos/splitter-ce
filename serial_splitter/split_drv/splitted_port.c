#include "splitted_port.h"

typedef struct __splitted_port_context {
	DWORD		access_rights;
}SplittedPort, *PSplittedPort;

static BOOL	CouldRead(PSplittedPort	context);

ULONG
PortRead(PVOID pOpenContext, PUCHAR pTargetBuffer, ULONG BufferLength) {
	PSplittedPort	context = (PSplittedPort)pOpenContext;

	DWORD	error = ERROR_SUCCESS;

	if (NULL == context) {
		SetLastError(ERROR_INVALID_HANDLE);
		return (ULONG)-1;
	}

	if (!CouldRead(context)) {
		SetLastError(ERROR_INVALID_ACCEL_HANDLE);
		return (ULONG)-1;
	}

	if ((NULL == pTargetBuffer) || (0 == BufferLength)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return (ULONG)-1;
	}

	return 0;
}

BOOL
CouldRead(PSplittedPort	context) {
	return context->access_rights & GENERIC_READ;
}