#ifndef __SPLITTER_DRV_HELPER_H__
#define __SPLITTER_DRV_HELPER_H__

#include "..\common\splitter_settings.h"

BOOL	get_device_key(DWORD device_index, PHKEY active_key);

BOOL	replace_driver(HKEY active_key, PSplitterSettings settings);
BOOL	restore_driver(HKEY active_key, PSplitterSettings settings);

#endif