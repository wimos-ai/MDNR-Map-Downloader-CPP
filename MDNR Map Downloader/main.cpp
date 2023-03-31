// Windows Header Files
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <uxtheme.h>

//STL
#include <stdexcept>
#include <exception>

//Custom Header Files
#include "MainWindow.h"
#include "resource.h"

//Lib Files
#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"UxTheme.lib")


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	
	// Initialize GDI+.	
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Gdiplus::Status::Ok) {
		MessageBox(NULL, L"Application will terminate", L"GdiplusStartup has failed", MB_OK);
		return EXIT_FAILURE;
	}

	SetProcessDPIAware();

	//Allow Buffered Painting
	if (BufferedPaintInit() != Gdiplus::Status::Ok) {
		MessageBox(NULL, L"Application may behave poorly. Screen tearing may result", L"BufferedPaintInit has failed", MB_OK);
	}

	//Load hardware accelerators
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));


	//Create the threadMain window
	MainWindow mainWin(hInstance, nCmdShow);

	
	// Run the message loop.
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	//Shutdown the window
	mainWin.Shutdown();


	BufferedPaintUnInit();

	Gdiplus::GdiplusShutdown(gdiplusToken);
	
	return EXIT_SUCCESS;
}

