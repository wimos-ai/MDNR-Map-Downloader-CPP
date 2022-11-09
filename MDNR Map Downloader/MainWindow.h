#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <objidl.h>



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void register_WClass(HINSTANCE hInstance);

HWND createWindow(HINSTANCE hInstance, int nCmdShow);

void ShutdownMDNRMap();
