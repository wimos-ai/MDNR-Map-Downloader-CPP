#pragma once

#define UXTHME_BUFFER

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <objidl.h>

#include <cwchar>

#include "Location_t.h"

#include "MDNR_Map.h"

class MainWindow {

	MDNR_Map mdnr_map;

	Location_t map_location{ 15788, 23127, 16 };

	const HWND WindowProcessHWND;
#ifdef UXTHME_BUFFER
	const HRESULT bufferedInitResult;
#endif

	static constexpr wchar_t CLASS_NAME[] = L"Sample Window Class";
	static constexpr wchar_t APP_NAME[] = L"MDNR Map Downloader";

	LRESULT memberWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void register_WClass(HINSTANCE hInstance);

	HWND createWindow(HINSTANCE hInstance, int nCmdShow);

	void Shutdown();

	void paint(HWND hwnd);

	static void paint(Location_t map_location, MDNR_Map& mdnr_map, HWND hwnd,HDC hdc);

	void paintDoubleBuffered(HWND hwnd);

	void cacheWindowArea(int distanceOutsizeBoarder);


public:
	~MainWindow();

	MainWindow(HINSTANCE hInstance, int nCmdShow);

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		MainWindow* me = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (me) {
			return me->memberWndProc(hwnd, msg, wParam, lParam);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

};