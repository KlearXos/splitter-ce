#include "splitter_settings.h"

static const wchar_t	settings_key_root[] = L"Software\\SerialSplitter\\Settings";

static const wchar_t	value_name_installed[] = L"Installed";
static const wchar_t	value_name_index[] = L"Index";
static const wchar_t	value_name_drv_path[] = L"RealDrv";
static const wchar_t	value_name_our_drv[] = L"OurDrv";

BOOL
get_splitter_settings(PSplitterSettings settings) {
	BOOL		result;
	HKEY		settings_key;
	LONG		error;
	DWORD		temp;
	DWORD		size;

	if (settings == NULL) return FALSE;

	memset(settings, 0, sizeof(SplitterSettings));

	error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, settings_key_root, 0, 0, &settings_key);
	if (error != ERROR_SUCCESS) {
		return FALSE;
	}

	result = FALSE;

	do {
		size = sizeof(DWORD);
		error = RegQueryValueEx(settings_key, value_name_installed, NULL, NULL, (LPBYTE)&temp, &size);
		if (error != ERROR_SUCCESS) break;

		settings->installed = (temp ? TRUE : FALSE);

		size = sizeof(DWORD);
		error = RegQueryValueEx(settings_key, value_name_index, NULL, NULL, (LPBYTE)&settings->device_index, &size);
		if (error != ERROR_SUCCESS) break;

		size = sizeof(settings->our_driver_path);
		error = RegQueryValueEx(settings_key, value_name_our_drv, NULL, NULL, (LPBYTE)settings->our_driver_path, &size);
		if (error != ERROR_SUCCESS) break;

		size = sizeof(settings->real_driver_path);
		error = RegQueryValueEx(settings_key, value_name_drv_path, NULL, NULL, (LPBYTE)settings->real_driver_path, &size);
		if (error != ERROR_SUCCESS) break;

		result = TRUE;
	} while (0);

	RegCloseKey(settings_key);

	return result;
}

BOOL
save_splitter_settings(PSplitterSettings settings) {
	BOOL		result;
	ULONG		error;

	DWORD		temp;
	DWORD		size;

	HKEY		settings_key;

	if (settings == NULL) {
		RegDeleteKey(HKEY_LOCAL_MACHINE, settings_key_root);
		return TRUE;
	}

	result = FALSE;

	error = RegCreateKeyEx(HKEY_LOCAL_MACHINE, settings_key_root, 0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &settings_key, NULL);
	if (error != ERROR_SUCCESS) {
		return FALSE;
	}

	do {
		temp = settings->installed;
		size = sizeof(DWORD);
		error = RegSetValueEx(settings_key, value_name_installed, 0, REG_DWORD, (LPBYTE)&temp, size);
		if (error != ERROR_SUCCESS) break;

		size = sizeof(DWORD);
		error = RegSetValueEx(settings_key, value_name_index, 0, REG_DWORD, (LPBYTE)&settings->device_index, size);
		if (error != ERROR_SUCCESS) break;

		size = (wcslen(settings->our_driver_path) + 1)*sizeof(wchar_t);
		error = RegSetValueEx(settings_key, value_name_our_drv, 0, REG_SZ, (LPBYTE)settings->our_driver_path, size);
		if (error != ERROR_SUCCESS) break;

		size = (wcslen(settings->real_driver_path) + 1)*sizeof(wchar_t);
		error = RegSetValueEx(settings_key, value_name_drv_path, 0, REG_SZ, (LPBYTE)settings->real_driver_path, size);
		if (error != ERROR_SUCCESS) break;

		result = TRUE;
	} while (0);

	RegCloseKey(settings_key);

	return result;
}