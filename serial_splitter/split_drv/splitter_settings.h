#ifndef __SPLITTER_SETTINGS_H__
#define __SPLITTER_SETTINGS_H__

typedef struct __splitter_settings {
	wchar_t		*driverPath;
}SplitterSettings, *PSplitterSettings;

PSplitterSettings	GetSplitterSettings(wchar_t *reg_path);

void				FreeSplitterSettings(PSplitterSettings settings);

#endif