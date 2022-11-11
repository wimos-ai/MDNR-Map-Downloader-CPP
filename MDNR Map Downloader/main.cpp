// MDNR Map Downloader.cpp : Defines the entry point for the application.
//
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

// Windows Header Files
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

//Custom Header Files
#include "MainWindow.h"
#include "resource.h"

//STL
#include <stdexcept>
#include <exception>



int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,	_In_ LPWSTR lpCmdLine,	_In_ int nCmdShow){
	// Initialize GDI+.	
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	
	{
		MainWindow mainWin(hInstance, nCmdShow);

		HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

		// Run the message loop.
		MSG msg{};
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	GdiplusShutdown(gdiplusToken);

#ifdef _DEBUG
	if (_CrtDumpMemoryLeaks()) {
		//throw std::bad_alloc();
	}
#endif
	return 0;
}

