#include "debug.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif

#include <string>
#include <stdlib.h>
#include <crtdbg.h>
#include <iomanip>
#include <sstream>

void WinAPIErrorHandler(DWORD error_code) {
	std::wostringstream oss;  // note the 'w'
	oss << L"API Failed with error code: 0x" << std::hex << error_code;

	if (MessageBox(NULL, oss.str().c_str(), L"Windows API Error", MB_OK | MB_ICONERROR) == IDOK) {
		ExitProcess(-1);
	}
}

void notImplementedPopup(HWND root_window) {
	if (MessageBox(root_window, L"Do you wish to quit?", L"This Feature Is Not Implemented", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
	{
		ExitProcess(-1);
	}
}