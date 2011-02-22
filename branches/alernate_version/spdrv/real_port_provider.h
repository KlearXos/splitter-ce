#ifndef __REAL_PORT_PROVIDER_H_17022011__
#define __REAL_PORT_PROVIDER_H_17022011__

#include <windows.h>

//HANDLE	Call_COM_Init(LPVOID ctx);
BOOL	Call_COM_PreDeinit(LPVOID ctx);
//BOOL	Call_COM_Deinit(PPortFunctions functions, PVOID pInitContext);
//HANDLE	Call_COM_Open(PPortFunctions functions, PVOID pInitContext, DWORD AccessCode, DWORD ShareMode);
//BOOL	Call_COM_Close(pOpenContext ctx);
BOOL	Call_COM_Read(LPVOID ctx, void* pTargetBuffer, ULONG BufferLength, ULONG *bytes_read);
ULONG	Call_COM_Write(LPVOID ctx, PUCHAR pSourceBytes, ULONG NumberOfBytes);
//ULONG	Call_COM_Seek(PVOID pOpenContext, LONG Position, DWORD Type);
BOOL	Call_COM_PowerUp(LPVOID ctx);
BOOL	Call_COM_PowerDown(LPVOID ctx);
BOOL	Call_COM_IOControl(PVOID ctx, DWORD dwCode, PBYTE pBufIn,DWORD dwLenIn,
    PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);

///////////////////////////////////////////////////////////////////////////////
//                                 pedgser.h
///////////////////////////////////////////////////////////////////////////////

//  @struct SERIAL_DEV_STATUS | Structure used to get communication
//		status.
//	@xref <f ttt_IOControl> <f IOCTL_SERIAL_GET_COMMSTATUS>
typedef struct _SERIAL_DEV_STATUS {
	DWORD	Errors;			//@field mask of error type
		//		@tab2	Value |	Meaning
		//		@tab2	CE_BREAK | The hardware detected a break
		//			condition.
		//		@tab2	CE_FRAME | The hardware detected a framing
		//			error.
		//		@tab2	CE_IOE | An I/O error occurred during
		//			communications with the device.
		//		@tab2	CE_MODE | The requested
		//			mode is not supported, or the hCommDev parameter is
		//			invalid. If this value is specified, it is the only valid
		//			error.
		//		@tab2	CE_OVERRUN | A character-buffer overrun has
		//			occurred. The next character is lost.
		//		@tab2	CE_RXOVER | An input buffer overflow has occurred.
		//			There is either no room in the input buffer, or a
		//			character was received after the end-of-file (EOF)
		//			character.
		//		@tab2	CE_RXPARITY | The hardware detected a parity error.
		//		@tab2	CE_TXFULL | The application tried to transmit a
		//			character, but the output buffer was full.
		//		@tab2	CE_DNS | The parallel device is not selected.
		//		@tab2	CE_PTO | A time-out occurred on a parallel device.
		//		@tab2	CE_OOP | The parallel device signaled that it is out
		//			of paper.
	COMSTAT	ComStat;		//@field ComStat structure.
} SERIAL_DEV_STATUS, *PSERIAL_DEV_STATUS;
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                            windev.h
///////////////////////////////////////////////////////////////////////////////
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0

//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define IOCTL_SERIAL_GET_WAIT_MASK      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 9,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_WAIT_MASK      CTL_CODE(FILE_DEVICE_SERIAL_PORT,10,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_WAIT_ON_MASK       CTL_CODE(FILE_DEVICE_SERIAL_PORT,11,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_COMMSTATUS     CTL_CODE(FILE_DEVICE_SERIAL_PORT,12,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_TIMEOUTS       CTL_CODE(FILE_DEVICE_SERIAL_PORT,15,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_TIMEOUTS       CTL_CODE(FILE_DEVICE_SERIAL_PORT,16,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_QUEUE_SIZE     CTL_CODE(FILE_DEVICE_SERIAL_PORT,18,METHOD_BUFFERED,FILE_ANY_ACCESS)
///////////////////////////////////////////////////////////////////////////////



#endif