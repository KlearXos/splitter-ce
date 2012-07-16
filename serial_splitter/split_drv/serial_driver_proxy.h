#ifndef __SERIAL_DRIVER_PROXY_H__
#define __SERIAL_DRIVER_PROXY_H__

#include <windows.h>

class SerialDriver;

class DriverProxy {
public:
	~DriverProxy();

	bool	Init(ULONG id);

	bool	PreDeinit(void);

	bool	PowerUp();
	bool	PowerDown();

	static DriverProxy* GetDriverProxy(wchar_t *driverPath);
private:
	DriverProxy(HMODULE driverLib, SerialDriver *driver);

	DriverProxy(const DriverProxy &src);
	DriverProxy& operator = (const DriverProxy &src);

	HMODULE			m_driverLib;
	SerialDriver	*m_driver;

	HANDLE		m_initHandle;

	static HMODULE	LoadDriverLibrary(wchar_t *driverPath);
	static void		UnloadDriverLibrary(HMODULE driverLib);

	static SerialDriver*	GetDriver(HMODULE driverLib);
	static void ReleaseDriver(SerialDriver *driver);

	void Deinit();
};

#endif