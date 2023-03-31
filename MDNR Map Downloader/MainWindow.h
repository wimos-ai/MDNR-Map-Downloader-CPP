#ifndef MAINWINDOW_H7832601796320
#define MAINWINDOW_H7832601796320
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files
#include <windows.h>
#include <objidl.h>

//STL Includes
#include <cwchar>

//My Includes
#include "Location_t.h"
#include "MDNR_Map.h"

class MainWindow {
public:
	~MainWindow() = default;

	MainWindow(HINSTANCE hInstance, int nCmdShow);

	void Shutdown();

private:
	//Members for mapping
	MDNR_Map mdnr_map;
	Location_t map_location{ 15788, 23127, 16 };

	//Members for drag and move
	POINT mouseDownLocation = {0};
	POINT currentMouseLocation = { 0 };
	bool mButtonDown = false;
	int dx = 0;
	int dy = 0;

	//HWND of class
	const HWND WindowProcessHWND;

	LRESULT memberWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void register_WClass(HINSTANCE hInstance);

	HWND createWindow(HINSTANCE hInstance, int nCmdShow);

	void paint(Location_t map_location, MDNR_Map& mdnr_map, HWND hwnd,HDC hdc);

	void paintDoubleBuffered(HWND hwnd);

	void cacheWindowArea(int distanceOutsizeBoarder);

	static constexpr wchar_t CLASS_NAME[] = L"Sample Window Class";
	static constexpr wchar_t APP_NAME[] = L"MDNR Map Downloader";

	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static INT_PTR CALLBACK LongLatHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif //MAINWINDOW_H7832601796320