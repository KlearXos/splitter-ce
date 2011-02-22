#ifndef __SPLITTER_SETTINGS_H_17022011__
#define __SPLITTER_SETTINGS_H_17022011__

#include <windows.h>

typedef struct __splitter_settings {
	DWORD	real_port_number;
} SplitterSettings, *PSplitterSettings;

BOOL	LoadSplitterSettings(PSplitterSettings settings);
BOOL	SaveSplitterSettings(PSplitterSettings settings);

#endif