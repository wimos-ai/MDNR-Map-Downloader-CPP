#ifndef DEBUG_H982087436098
#define DEBUG_H982087436098

#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

void WinAPIErrorHandler(DWORD error_code);

void notImplementedPopup(HWND root_window);
#endif