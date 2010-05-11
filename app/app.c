#include "app.h"

HINSTANCE	hInst;

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static wchar_t	our_drv_path[] = L"\\windows\\split_driver.dll";

void	init_dlg(HWND hDlg);
void	install_splitter(HWND hDlg);
void	uninstall_splitter(HWND hDlg);


int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPWSTR cmd, int show) {
	int	result;

	InitCommonControls();
	hInst = hInstance;

	result = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, MainDlgProc);
	return 0;
}

BOOL CALLBACK
MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	BOOL			result;

	switch (uMsg) {
	case WM_INITDIALOG:
		result = TRUE;

		// get settings
		init_dlg(hDlg);

		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_INSTALL:
			result = TRUE;
			install_splitter(hDlg);
			break;
		case IDC_UNINSTALL:
			result = TRUE;
			uninstall_splitter(hDlg);
			break;
		default:
			result = FALSE;
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		result = TRUE;
		break;

	default:
		result = FALSE;
		break;
	}
	return result;
}

void
init_dlg(HWND hDlg) {
	HWND		button_install;
	HWND		button_uninstall;
	HWND		edit_port;

	BOOL		result;

	SplitterSettings	settings;

	button_install = GetDlgItem(hDlg, IDC_INSTALL);
	button_uninstall = GetDlgItem(hDlg, IDC_UNINSTALL);
	edit_port = GetDlgItem(hDlg, IDC_PORT_NUMBER);

	memset(&settings, 0, sizeof(settings));

	result = get_splitter_settings(&settings);

	if (!result || !settings.installed) {
		EnableWindow(button_uninstall, FALSE);
	} else {
		EnableWindow(button_install, FALSE);
	}

	Edit_LimitText(edit_port, 3);

	if (result) {
		wchar_t	port_number[10];

		wsprintf(port_number, L"%d", settings.device_index);
		SetWindowText(edit_port, port_number);
	}
}

void
install_splitter(HWND hDlg) {
	BOOL			result;
	SplitterSettings	settings;
	wchar_t			port_number[4];
	size_t			length;
	size_t			i;
	HKEY			device_active_key;
	HWND			edit_port_number;

	// get port number from edit
	memset(port_number, 0, sizeof(port_number));
	edit_port_number = GetDlgItem(hDlg, IDC_PORT_NUMBER);
	Edit_GetText(edit_port_number, port_number, 3);

	memset(&settings, 0, sizeof(settings));
	length = wcslen(port_number);
	for (i=0; i<length; i++) {
		settings.device_index *= 10;
		settings.device_index += port_number[i] - L'0';
	}

	// find this device
	result = get_device_key(settings.device_index, &device_active_key);
	if (result) {
		// replace driver
		result = replace_driver(device_active_key, &settings);
		if (result) {
			// update shell
			EnableWindow(edit_port_number, FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_INSTALL), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_UNINSTALL), TRUE);
		} else {
			MessageBox(hDlg, L"Failed to replace driver", L"error", MB_OK);
		}

		RegCloseKey(device_active_key);
	} else {
		MessageBox(hDlg, L"Device not found", L"error", MB_OK);
	}
}

void
uninstall_splitter(HWND hDlg) {
	SplitterSettings	settings;
	BOOL			result;
	HKEY			device_active_key;

	memset(&settings, 0, sizeof(settings));

	result = get_splitter_settings(&settings);

	if (!result) {
		MessageBox(hDlg, L"Failed to get settings", L"error", MB_OK);
		return ;
	}

	// find this device
	result = get_device_key(settings.device_index, &device_active_key);
	if (result) {
		result = restore_driver(device_active_key, &settings);
		if (result) {
			// update shell
			EnableWindow(GetDlgItem(hDlg, IDC_PORT_NUMBER), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_INSTALL), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_UNINSTALL), FALSE);
		} else {
			MessageBox(hDlg, L"Failed to restore driver", L"error", MB_OK);
		}
		RegCloseKey(device_active_key);
	} else {
		MessageBox(hDlg, L"Device not found", L"error", MB_OK);
	}
}