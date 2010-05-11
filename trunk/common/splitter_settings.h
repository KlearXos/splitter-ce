#ifndef __SERIAL_SPLITTER_SETTINGS_H__
#define __SERIAL_SPLITTER_SETTINGS_H__

#include <windows.h>

// get and set splitter settings functions set
// used from driver and from control application

typedef struct __splitter_settings {
	// is splitter installed to only one port
	// [out]
	BOOL		installed;

	// our driver path
	// [in/out]
	wchar_t		our_driver_path[MAX_PATH];

	// real port driver path
	// [in/out]
	wchar_t		real_driver_path[MAX_PATH];

	// port driver index
	// [in/out]
	ULONG		device_index;
} SplitterSettings, *PSplitterSettings;

#ifdef __cplusplus
extern "C" {
#endif

// get settings from registry
// RETURN:
// 	FALSE (0) - if fails
// 	not 0 if success
// [in] - port index
// NOTE:
// 	if settings not setted, FALSE is returned
BOOL	get_splitter_settings(PSplitterSettings settings);

// store settings
// RETURN:
// 	FALSE (0) - if fails
// 	not 0 if success
BOOL	save_splitter_settings(PSplitterSettings settings);

#ifdef __cplusplus
}
#endif

#endif