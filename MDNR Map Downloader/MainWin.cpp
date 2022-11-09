#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "MainWin.h"

#include "resource.h"

#include <winuser.h>
#include <iostream>

#include <shellapi.h>
#include <commdlg.h>

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#include <cmath>
#include <stdexcept>

#include "debug.h"

#include "Location_t.h"

#include "MDNR_Map.h"

MDNR_Map mdnr_map;

const wchar_t CLASS_NAME[] = L"Sample Window Class";
const wchar_t APP_NAME[] = L"MDNR Map Downloader";

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LongLatHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void paint(HWND hwnd, HDC hdc, PAINTSTRUCT& ps);


void register_WClass(HINSTANCE hInstance) {
	mdnr_map.cacheArea(Location_t(15788, 23127, 16), 9);

	BOOL procAware = SetProcessDPIAware();
	// Register the window class.
	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1);

	RegisterClass(&wc);
}

HWND createWindow(HINSTANCE hInstance, int nCmdShow) {
	// Create the window.

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		APP_NAME,    // Window longitudeString
		(WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),            // Window style

		// Size and position
		CW_USEDEFAULT, //X
		CW_USEDEFAULT, //Y
		CW_USEDEFAULT, //Width
		CW_USEDEFAULT, //Height

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		throw std::runtime_error("Window Creation Failed With Error: " + GetLastError());
	}

	ShowWindow(hwnd, nCmdShow);

	return hwnd; //Graphics item automaticaly created
}

Location_t map_location(15788, 23127, 16);
HWND WindowProcessHWND{ nullptr };

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WindowProcessHWND = hwnd;
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		const int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
			case ID_HELP_ABOUT:
			{
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
				break;
			}
			case ID_HELP_CONTROLS:
			{
				//TODO: Implement Help Popup
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CTRLSBOX), hwnd, About);
				break;
			}
			case ID_GOTO_GPSCOORDS:
			{
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LONLATBOX), hwnd, LongLatHandler);
				break;

			}
			case ID_SAVE_SAVEAS:
			{
				wchar_t szFile[MAX_PATH] = {0};

				OPENFILENAME ofn{0};

				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = NULL;
				ofn.lpstrFile = szFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(szFile);
				ofn.lpstrFilter = L"Bitmap\0*.BMP\0\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;


				GetSaveFileName(&ofn);

				screenshot(hwnd, ofn.lpstrFile);

				break;
			}
			case ID_ACCELERATOR40007: //Right
			{
				map_location.x += 1;
				InvalidateRect(hwnd, NULL, true);
				break;
			}
			case ID_ACCELERATOR40009: //Left
			{
				map_location.x -= 1;
				InvalidateRect(hwnd, NULL, true);
				break;
			}
			case ID_ACCELERATOR40010: //Up
			{
				map_location.y -= 1;
				InvalidateRect(hwnd, NULL, true);
				break;
			}
			case ID_ACCELERATOR40011: //Down
			{
				map_location.y += 1;
				InvalidateRect(hwnd, NULL, true);
				break;
			}
			case ID_ACCELERATORZOOMOUT: //+
			{
				if (map_location.layer > 1)
				{
					map_location.x /= 2;
					map_location.y /= 2;
					map_location.layer--;
					InvalidateRect(hwnd, NULL, true);
				}
				break;

			}
			case ID_ACCELERATORZOOMIN: //-
			{
				if (map_location.layer < 16)
				{
					map_location.x *= 2;
					map_location.y *= 2;
					map_location.layer++;
					InvalidateRect(hwnd, NULL, true);
				}
				break;
			}
		}

		break;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	case WM_SIZE:
	{
		int width = LOWORD(lParam);  // Macro to get the low-order word.
		int height = HIWORD(lParam); // Macro to get the high-order word.

		// Respond to the message:
		DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	}

	case WM_PAINT:
	{

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		paint(hwnd, hdc, ps);

		break;
	}

	case WM_CLOSE:
	{
		if (MessageBox(hwnd, L"Do you wish to quit?", APP_NAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			DestroyWindow(hwnd);
		}// Else: User canceled. Do nothing.
		break;
	}

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void paint(HWND hwnd, HDC hdc, PAINTSTRUCT& ps) {
	FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

	RECT bbox;
	if (!GetWindowRect(hwnd, &bbox)) {
		throw std::runtime_error("Failed to create Get Window Size. Win32 Error Code: " + GetLastError());
	}


	IMG_t panel = mdnr_map.get(map_location);

	constexpr unsigned int img_width = MDNR_Map::pannel_width;
	constexpr unsigned int img_height = MDNR_Map::pannel_height;

	const unsigned int x_width_pannels = (std::abs(bbox.left - bbox.right) / img_width) + 1;
	const unsigned int y_height_pannels = (std::abs(bbox.top - bbox.bottom) / img_height) + 1;

	for (unsigned int y = 0; y < y_height_pannels; y++)
	{
		for (unsigned int x = 0; x < x_width_pannels; x++)
		{
			//Location_t map_location(x + 15788, y + 23127, 16);
			Location_t get_loaction(x + map_location.x, y + map_location.y, map_location.layer);
			auto v = mdnr_map.get(get_loaction);

			draw_IMG(v, hdc, (INT)(img_width * x), (INT)(img_height * y));
		}
	}


	EndPaint(hwnd, &ps);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK LongLatHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			wchar_t longitudeString[512] = {0};
			SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_GETTEXT, sizeof(longitudeString) / sizeof(longitudeString[0]), (LPARAM)longitudeString);

			wchar_t* nullTerminator = (wchar_t*)memchr(longitudeString, 0, sizeof(longitudeString));
			double longitude = -wcstod(longitudeString, &nullTerminator);

			wchar_t latitudeString[512] = { 0 };
			SendMessage(GetDlgItem(hDlg, IDC_EDIT2), WM_GETTEXT, sizeof(latitudeString) / sizeof(latitudeString[0]), (LPARAM)latitudeString);

			nullTerminator = (wchar_t*)memchr(latitudeString, 0, sizeof(latitudeString));
			double latitude = wcstod(latitudeString, &nullTerminator);

			EndDialog(hDlg, LOWORD(wParam));

			if (latitude != 0.0 && longitude != 0)
			{
				map_location = Location_t::fromGPSCoords(longitude, latitude, 16);
				mdnr_map.clear_cache();
				mdnr_map.cacheArea(map_location, 8);
				InvalidateRect(WindowProcessHWND, NULL, TRUE);
			}
			else {
				MessageBox(WindowProcessHWND, L"Improper Input", L"Longitude and Latitude Could Not Be Read", MB_OK | MB_ICONEXCLAMATION);
			}

			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}