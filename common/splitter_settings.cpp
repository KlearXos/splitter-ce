#include "splitter_settings.h"

BOOL
LoadSplitterSettings(PSplitterSettings settings) {
	if (settings == NULL) {
		return FALSE;
	}

	settings->real_port_number = 11;
	
	return TRUE;
}

BOOL
SaveSplitterSettings(PSplitterSettings settings) {
	if (settings == NULL) {
		return FALSE;
	}
}