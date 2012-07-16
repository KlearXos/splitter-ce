#include "serial_driver_proxy.h"

#include "serial_driver.h"

extern "C" HINSTANCE LoadDriver(LPCWSTR lpszFileName);

DriverProxy::DriverProxy(HMODULE driverLib, SerialDriver *driver) 
  : m_driverLib(driverLib), m_driver(driver), m_initHandle(NULL) {
}

DriverProxy::~DriverProxy() {
	Deinit();

	UnloadDriverLibrary(m_driverLib);
	ReleaseDriver(m_driver);
}

DriverProxy* DriverProxy::GetDriverProxy(wchar_t *driverPath) {
	HMODULE	driverLib;
	driverLib = LoadDriverLibrary(driverPath);
	if (NULL != driverLib) {
		SerialDriver	*driver = GetDriver(driverLib);
		if (NULL != driver) {
			return new DriverProxy(driverLib, driver);
		}

		UnloadDriverLibrary(driverLib);
	}

	return NULL;
}

HMODULE	DriverProxy::LoadDriverLibrary(wchar_t *driverPath) {
	return LoadDriver(driverPath);
}

void	DriverProxy::UnloadDriverLibrary(HMODULE driverLib) {
	FreeLibrary(driverLib);
}

SerialDriver*	DriverProxy::GetDriver(HMODULE driverLib) {
	return SerialDriver::GetDriver(driverLib);
}

void	DriverProxy::ReleaseDriver(SerialDriver *driver) {
	delete driver;
}

bool	DriverProxy::Init(ULONG id) {
	m_initHandle = m_driver->Init(id);

	return (NULL != m_initHandle);
}

bool	DriverProxy::PreDeinit(void) {
	return m_driver->PreDeinit(m_initHandle) ? true : false;
}

void	DriverProxy::Deinit() {
	if (NULL != m_initHandle) {
		m_driver->Deinit(m_initHandle);
	}
}

bool	DriverProxy::PowerUp() {
	return m_driver->PowerUp(m_initHandle) ? true : false;
}

bool	DriverProxy::PowerDown() {
	return m_driver->PowerDown(m_initHandle) ? true : false;
}