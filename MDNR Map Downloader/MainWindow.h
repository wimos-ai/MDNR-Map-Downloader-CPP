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

	POINT mouseDownLocation = {0};
	POINT currentMouseLocation = { 0 };

	bool mButtonDown = false;
	int dx = 0;
	int dy = 0;

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

	void paint(Location_t map_location, MDNR_Map& mdnr_map, HWND hwnd,HDC hdc);

	void paintDoubleBuffered(HWND hwnd);

	void cacheWindowArea(int distanceOutsizeBoarder);

	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static INT_PTR CALLBACK LongLatHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


public:
	~MainWindow();

	MainWindow(HINSTANCE hInstance, int nCmdShow);

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	friend LRESULT CALLBACK mouseClickEventMaker(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);

};