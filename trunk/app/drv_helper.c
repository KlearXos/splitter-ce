#include "drv_helper.h"

static const wchar_t device_prefix[] = L"COM";
static const wchar_t active_drivers_key_name[] = L"Drivers\\Active";

static const wchar_t handle_value_name[] = L"Hnd";
static const wchar_t driver_key_value_name[] = L"Key";
static const wchar_t driver_dll_value_name[] = L"Dll";

static const wchar_t our_driver_path[] = L"\\windows\\split_driver.dll";

BOOL	is_target_device_key(PHKEY active_device_key, wchar_t *device_name);

// NOTE: device index greater than 9 not supported yet
BOOL
get_device_key(DWORD device_index, PHKEY active_device_key) {
	HKEY	active_key;
	BOOL	result;
	LONG	error;
	DWORD	subkeys;
	DWORD	max_subkey_len;
	DWORD	subkey_len;
	DWORD	i;
	wchar_t	*subkey_name;
	wchar_t	full_name[] = L"XXXn:";

	if (device_index > 9 ) return FALSE;

	error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, active_drivers_key_name, 0, 0, &active_key);
	if (error != ERROR_SUCCESS) {
		return FALSE;
	}

	result = FALSE;

	// enum allsubkeys and find device
	error = RegQueryInfoKey(active_key, NULL, NULL, NULL, &subkeys, &max_subkey_len, NULL, NULL, NULL, NULL, NULL, NULL);
	if (error == ERROR_SUCCESS) {
		wsprintf(full_name, L"%s%d:", device_prefix, device_index);
		max_subkey_len ++;

		subkey_name = LocalAlloc(LMEM_ZEROINIT, sizeof(wchar_t) * max_subkey_len);
		if (subkey_name != NULL) {
			for (i=0; i<subkeys; i++) {
				subkey_len = max_subkey_len;
				error = RegEnumKeyEx(active_key, i, subkey_name, &subkey_len, NULL, NULL, NULL, NULL);
				if (error != ERROR_SUCCESS) break;

				error = RegOpenKeyEx(active_key, subkey_name, 0, 0, active_device_key);
				if (error != ERROR_SUCCESS) break;

				if (is_target_device_key(active_device_key, full_name)) {
					result = TRUE;
					break;
				}

				RegCloseKey(*active_device_key);
			}

			LocalFree(subkey_name);
		}
	}

	RegCloseKey(active_key);

	return result;
}

BOOL
replace_driver(HKEY active_key, PSplitterSettings settings) {
	DWORD		dev_handle;
	DWORD		size;
	wchar_t		*drv_key_name;
	BOOL		result;
	HKEY		dev_key;
	LONG		error;

	// get device handle
	size = sizeof(DWORD);
	error = RegQueryValueEx(active_key, handle_value_name, NULL, NULL, (LPBYTE)&dev_handle, &size);
	if (error != ERROR_SUCCESS) {
		return FALSE;
	}

	result = FALSE;

	// get deivce key name
	error = RegQueryValueEx(active_key, driver_key_value_name, NULL, NULL, NULL, &size);
	if (error == ERROR_SUCCESS) {
		drv_key_name = (wchar_t*)LocalAlloc(LMEM_ZEROINIT, size);
		if (drv_key_name != NULL) {
			error = RegQueryValueEx(active_key, driver_key_value_name, NULL, NULL, (LPBYTE)drv_key_name, &size);
			if (error == ERROR_SUCCESS) {
				error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, drv_key_name, 0, 0, &dev_key);
				if (error == ERROR_SUCCESS) {
					size = sizeof(settings->real_driver_path);
					error = RegQueryValueEx(dev_key, driver_dll_value_name, NULL, NULL, (LPBYTE)settings->real_driver_path, &size );
					if (error == ERROR_SUCCESS) {
						size = sizeof(our_driver_path);
						memcpy(settings->our_driver_path, our_driver_path, size);
						settings->installed = TRUE;

						result = save_splitter_settings(settings);
						if (result) {
							// deactivate
							result = DeactivateDevice((HANDLE)dev_handle);
							if (result) {
								result = FALSE;
								// replace dll
								error = RegSetValueEx(dev_key, driver_dll_value_name, 0, REG_SZ, (LPBYTE)our_driver_path, size);
								if (error == ERROR_SUCCESS) {
									// activate
									// XXX: client info
									if (ActivateDevice(drv_key_name, 0)) {
										result = TRUE;
									} else {
										error = GetLastError();
									}
								}
							}

							if (!result) {
								settings->installed = FALSE;
								save_splitter_settings(settings);
							}
						}
					}

					RegCloseKey(dev_key);
				}
			}

			LocalFree(drv_key_name);
		}
	}

	return result;
}


BOOL
restore_driver(HKEY active_key, PSplitterSettings settings) {
	DWORD		dev_handle;
	DWORD		size;
	wchar_t		*drv_key_name;
	BOOL		result;
	HKEY		dev_key;
	LONG		error;

	// get device handle
	size = sizeof(DWORD);
	error = RegQueryValueEx(active_key, handle_value_name, NULL, NULL, (LPBYTE)&dev_handle, &size);
	if (error != ERROR_SUCCESS) {
		return FALSE;
	}

	result = FALSE;

	// replace driver dll
	error = RegQueryValueEx(active_key, driver_key_value_name, NULL, NULL, NULL, &size);
	if (error == ERROR_SUCCESS) {
		drv_key_name = (wchar_t*)LocalAlloc(LMEM_ZEROINIT, size);
		if (drv_key_name != NULL) {
			error = RegQueryValueEx(active_key, driver_key_value_name, NULL, NULL, (LPBYTE)drv_key_name, &size);
			if (error == ERROR_SUCCESS) {
				result = DeactivateDevice((HANDLE)dev_handle);
				if (result) {
					result = FALSE;
					error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, drv_key_name, 0, 0, &dev_key);
					if (error == ERROR_SUCCESS) {
						size = sizeof(wchar_t) * (wcslen(settings->real_driver_path) + 1);

						error = RegSetValueEx(dev_key, driver_dll_value_name, 0, REG_SZ, (LPBYTE)settings->real_driver_path, size);
						if (error == ERROR_SUCCESS) {
							// XXX client info
							if ( ActivateDevice(drv_key_name, 0) != NULL) {
								result = TRUE;
								settings->installed = FALSE;
								save_splitter_settings(settings);
							} else {
								error = GetLastError();
							}
						}

						RegCloseKey(dev_key);
					}
				} else {
					result = GetLastError();
				}
			}

			LocalFree(drv_key_name);
		}
	}

	return result;
}

BOOL
is_target_device_key(PHKEY active_device_key, wchar_t *device_name) {
	wchar_t		active_device_name[] = L"XXXn:";
	LONG		error;
	BOOL		result;
	DWORD		size;

	result = FALSE;
	size = sizeof(active_device_name);

	error = RegQueryValueEx(*active_device_key, L"Name", NULL, NULL, (LPBYTE)active_device_name, &size);
	if (error == ERROR_SUCCESS) {
		if (size == sizeof(active_device_name)) {
			if (!memcmp(active_device_name, device_name, size)) {
				result = TRUE;
			}
		}
	}

	return result;
}