#define _CRTDBG_MAP_ALLOC

//Windows Includes
#include <windows.h> //Windows.h Include must be first
#include <stdlib.h>
#include <crtdbg.h>
#include <uxtheme.h>
#include <shellapi.h>
#include <commdlg.h>
#include <objidl.h>
#include <gdiplus.h>
#include <winuser.h>

//STL Includes
#include <cmath>
#include <stdexcept>
#include <memory>
#include <array>
#include <chrono>
#include <thread>
#include <iostream>


//My Includes
#include "MainWindow.h"
#include "resource.h"
#include "httpException.h"
#include "debug.h"

//Needed Libs
#pragma comment (lib,"UxTheme.lib")
#pragma comment (lib,"Gdiplus.lib")

typedef struct LongLat {
	double longitude;
	double latitude;
}LongLat;

constexpr long long LONGLATMSG = 0x0401;		//passes a LongLat pointer in the wparam
constexpr long long CACHED_AREA_OUTSIDE_BOARDER = 3;

MainWindow::MainWindow(HINSTANCE hInstance, int nCmdShow) :WindowProcessHWND((this->register_WClass(hInstance), this->createWindow(hInstance, nCmdShow))), bufferedInitResult(BufferedPaintInit()) {
	cacheWindowArea(CACHED_AREA_OUTSIDE_BOARDER);
}

MainWindow::~MainWindow() {
	BufferedPaintUnInit();
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	MainWindow* me = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (me) {
		return me->memberWndProc(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void MainWindow::cacheWindowArea(int distanceOutsizeBoarder) {

	RECT rect;
	GetClientRect(WindowProcessHWND, &rect);
	const INT win_width{ rect.right - rect.left };
	const INT win_height{ rect.bottom - rect.top };


	const INT num_width_pannels{ (win_width / MDNR_Map::pannel_width) + 1 };
	const INT num_height_pannels{ (win_height / MDNR_Map::pannel_height) + 1 };

	mdnr_map.cacheArea(this->map_location, Location_t(this->map_location.x + num_width_pannels, this->map_location.y + num_height_pannels, this->map_location.layer), 0);

	mdnr_map.cacheArea(this->map_location, Location_t(this->map_location.x + num_width_pannels, this->map_location.y + num_height_pannels, this->map_location.layer), distanceOutsizeBoarder);

	mdnr_map.trimToArea(this->map_location, Location_t(this->map_location.x + num_width_pannels, this->map_location.y + num_height_pannels, this->map_location.layer), distanceOutsizeBoarder + 1);

}

void MainWindow::register_WClass(HINSTANCE hInstance) {
	BOOL procAware = SetProcessDPIAware();
	// Register the window class.
	WNDCLASS wc = { 0 };

	wc.lpfnWndProc = this->WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1);

	RegisterClass(&wc);
}

HWND MainWindow::createWindow(HINSTANCE hInstance, int nCmdShow) {
	// Create the window.
	HWND hwnd = CreateWindowExW(
		0,																									// Optional window styles.
		CLASS_NAME,																							// Window class
		APP_NAME,																							// Window longitudeString
		(WS_EX_LAYERED | WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),	// Window style
		CW_USEDEFAULT,																						//X
		CW_USEDEFAULT,																						//Y
		CW_USEDEFAULT,																						//Width
		CW_USEDEFAULT,																						//Height
		NULL,																								// Parent window    
		NULL,																								// Menu
		hInstance,																							// Instance handle
		NULL																								// Additional application data
	);

	if (hwnd == NULL)
	{
		throw std::runtime_error("Window Creation Failed With Error: " + GetLastError());
	}

	ShowWindow(hwnd, nCmdShow);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	return hwnd;
}

void MainWindow::Shutdown()
{
	mdnr_map.clear_cache();
}

std::unique_ptr<wchar_t> getFileSaveAsName() {
	std::unique_ptr<wchar_t> fileName{ new wchar_t[MAX_PATH] };
	ZeroMemory(fileName.get(), MAX_PATH);

	OPENFILENAME ofn{ 0 };

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = fileName.get();
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"Bitmap\0*.BMP\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT;


	GetSaveFileName(&ofn);

	return fileName;

}

LRESULT MainWindow::memberWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
			return 0;
		}
		case ID_HELP_CONTROLS:
		{
			//TODO: Implement Help Popup
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CTRLSBOX), hwnd, About);
			return 0;
		}
		case ID_GOTO_GPSCOORDS:
		{
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LONLATBOX), hwnd, LongLatHandler);
			return 0;

		}
		case ID_SAVE_SAVEAS:
		{
			std::unique_ptr<wchar_t> fileName{ getFileSaveAsName() };
			screenshot(hwnd, fileName.get());

			return 0;
		}
		case ID_SAVE_SAVEDETAILED:
		{
			std::unique_ptr<wchar_t> fileName{ getFileSaveAsName() };
			Location_t top_left(map_location);

			RECT rect;
			GetClientRect(hwnd, &rect);

			const INT win_width{ rect.right - rect.left };
			const INT win_height{ rect.bottom - rect.top };

			const INT num_width_pannels{ (win_width / MDNR_Map::pannel_width) + 1 };
			const INT num_height_pannels{ (win_height / MDNR_Map::pannel_height) + 1 };
			Location_t bottom_right(top_left.x + num_width_pannels, top_left.y + num_height_pannels, top_left.layer);

			top_left.translateLayer(16);
			bottom_right.translateLayer(16);

			saveArea(top_left, bottom_right, fileName.get());

			return 0;
		}
		case ID_SAVE_SAVETHRESHOLDED:
		{
			std::unique_ptr<wchar_t> fileName{ getFileSaveAsName() };
			notImplementedPopup(hwnd);
			return 0;
		}
		case ID_ACCELERATOR40007: //Right
		{
			map_location.x += 1;
			InvalidateRect(hwnd, NULL, FALSE);
			cacheWindowArea(CACHED_AREA_OUTSIDE_BOARDER);

			return 0;
		}
		case ID_ACCELERATOR40009: //Left
		{
			map_location.x -= 1;
			InvalidateRect(hwnd, NULL, FALSE);
			cacheWindowArea(CACHED_AREA_OUTSIDE_BOARDER);
			return 0;
		}
		case ID_ACCELERATOR40010: //Up
		{
			map_location.y -= 1;
			InvalidateRect(hwnd, NULL, FALSE);
			cacheWindowArea(CACHED_AREA_OUTSIDE_BOARDER);
			return 0;
		}
		case ID_ACCELERATOR40011: //Down
		{
			map_location.y += 1;
			InvalidateRect(hwnd, NULL, FALSE);
			cacheWindowArea(CACHED_AREA_OUTSIDE_BOARDER);
			return 0;
		}
		case ID_ACCELERATORZOOMOUT: //+
		{
			if (map_location.layer > 1)
			{
				map_location.x /= 2;
				map_location.y /= 2;
				map_location.layer--;
				InvalidateRect(hwnd, NULL, FALSE);
				mdnr_map.clear_cache();
				cacheWindowArea(CACHED_AREA_OUTSIDE_BOARDER);
			}
			return 0;

		}
		case ID_ACCELERATORZOOMIN: //-
		{
			if (map_location.layer < 16)
			{
				map_location.x *= 2;
				map_location.y *= 2;
				map_location.layer++;
				InvalidateRect(hwnd, NULL, FALSE);
				mdnr_map.clear_cache();
				cacheWindowArea(CACHED_AREA_OUTSIDE_BOARDER);
			}
			return 0;
		}
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_PAINT:
	{
		try {
			paintDoubleBuffered(hwnd);
		}
		catch (httpException) {
			ShowWindow(hwnd, SW_HIDE);
			mdnr_map.clear_cache();
			MessageBox(NULL, L"This program requires internet connectivity. Please connect and try again", L"Error!", MB_ICONERROR | MB_OK);
			DestroyWindow(hwnd);
		}
		return 0;
	}

	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
		return 0;
	}

	case LONGLATMSG:
	{
		std::unique_ptr<LongLat> coords{ reinterpret_cast<LongLat*>(wParam) };
		if (coords->latitude != 0.0 && coords->longitude != 0)
		{
			this->map_location = Location_t::fromGPSCoords(coords->longitude, coords->latitude, 16);
			mdnr_map.clear_cache();
			mdnr_map.cacheArea(map_location, 8);
			InvalidateRect(WindowProcessHWND, NULL, TRUE);
		}
		else {
			MessageBox(WindowProcessHWND, L"Improper Input", L"Longitude and Latitude Could Not Be Read", MB_OK | MB_ICONEXCLAMATION);
		}

		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		//Set up mouse tracking so we know when the mouse leaves
		TRACKMOUSEEVENT tm = { 0 };
		tm.cbSize = sizeof(TRACKMOUSEEVENT);
		tm.dwFlags = TME_LEAVE;
		tm.hwndTrack = hwnd;
		tm.dwHoverTime = NULL;

		//Make call with trace back for error handling
		if (!TrackMouseEvent(&tm)) {
			std::string errorMsg("Call to TrackMouseEvent failed");
			errorMsg += +__FILE__ + __LINE__;
			throw std::runtime_error(errorMsg);
		}

		//Change state to handle the button being pressed
		mButtonDown = true;
		GetCursorPos(&this->mouseDownLocation);
		this->currentMouseLocation = mouseDownLocation;
		InvalidateRect(hwnd, NULL, true);

		return 0;
	}

	case WM_MOUSELEAVE:
		if (!mButtonDown)
		{
			return 0;
		}
		//Fall through
	case WM_LBUTTONUP:
	{
		mButtonDown = false;
		GetCursorPos(&this->mouseDownLocation);

		//Calculate current drag offset
		int currMouseDx = (this->mouseDownLocation.x - this->currentMouseLocation.x);
		int currMouseDy = (this->mouseDownLocation.y - this->currentMouseLocation.y);

		//Increase stored drag offset
		dx += currMouseDx;
		dy += currMouseDy;

		//Change Location to reduce drag offset
		this->map_location.x -= (dx / MDNR_Map::pannel_width);
		this->map_location.y -= (dy / MDNR_Map::pannel_height);

		//Subtract pannels from offset because they aren't needed
		dx %= MDNR_Map::pannel_width;
		dy %= MDNR_Map::pannel_height;

		//Trim MDNR_Map to new bounds
		cacheWindowArea(CACHED_AREA_OUTSIDE_BOARDER);

		this->currentMouseLocation = mouseDownLocation;
		InvalidateRect(hwnd, NULL, true);
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (mButtonDown)
		{
			GetCursorPos(&this->mouseDownLocation);
			InvalidateRect(hwnd, NULL, true);
		}
		return 0;
	}

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void MainWindow::paintDoubleBuffered(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc{ BeginPaint(hwnd, &ps) };

	RECT sz;
	GetClientRect(hwnd, &sz);

	BP_PAINTPARAMS paintParams = { 0 };

	paintParams.cbSize = sizeof(paintParams);
	paintParams.dwFlags = BPPF_ERASE;
	paintParams.pBlendFunction = NULL;
	paintParams.prcExclude = NULL;

	HDC hdcBuffer;

	HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hdc, &sz, BPBF_COMPATIBLEBITMAP, &paintParams, &hdcBuffer);

	if (hBufferedPaint && this->bufferedInitResult == Gdiplus::Status::Ok) {
		// Application specific painting code
		paint(map_location, mdnr_map, hwnd, hdcBuffer);
		EndBufferedPaint(hBufferedPaint, TRUE);
	}
	else
	{
		paint(map_location, mdnr_map, hwnd, hdcBuffer);
	}

	ReleaseDC(hwnd, hdc);
}

void MainWindow::paint(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc{ BeginPaint(hwnd,&ps) };
	paint(map_location, mdnr_map, hwnd, hdc);
	EndPaint(hwnd, &ps);
	DeleteDC(hdc);
}

void MainWindow::paint(Location_t map_location, MDNR_Map& mdnr_map, HWND hwnd, HDC hdc) {
	constexpr INT img_width{ MDNR_Map::pannel_width };
	constexpr INT img_height{ MDNR_Map::pannel_height };

	RECT rect;
	GetClientRect(hwnd, &rect);

	const INT win_width{ rect.right - rect.left };
	const INT win_height{ rect.bottom - rect.top };

	const INT num_width_pannels{ (win_width / img_width) + 1 };
	const INT num_height_pannels{ (win_height / img_height) + 1 };

	Gdiplus::Graphics g(hdc);
	g.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceCopy);
	g.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor);


	int currMouseDx;
	int currMouseDy;
	if (mButtonDown) {
		currMouseDx = (this->mouseDownLocation.x - this->currentMouseLocation.x);
		currMouseDy = (this->mouseDownLocation.y - this->currentMouseLocation.y);
	}
	else {
		currMouseDx = 0;
		currMouseDy = 0;
	}
	currMouseDx += dx;
	currMouseDy += dy;



	for (INT y = -CACHED_AREA_OUTSIDE_BOARDER; y < num_height_pannels + CACHED_AREA_OUTSIDE_BOARDER; y++) {
		for (INT x = -CACHED_AREA_OUTSIDE_BOARDER; x < num_width_pannels + CACHED_AREA_OUTSIDE_BOARDER; x++) {

			Location_t get_loaction(x + map_location.x, y + map_location.y, map_location.layer);

			auto drawIm{ mdnr_map.get(get_loaction) };

			Gdiplus::Status stat{ g.DrawImage(drawIm.get(), (INT)(img_width * x) + currMouseDx, (INT)(img_height * y) + currMouseDy,img_width,img_height)};

			if (stat != Gdiplus::Status::Ok)
			{
				throw std::runtime_error(":(");
			}

		}
	}
}

// Message handler for about box.
INT_PTR CALLBACK MainWindow::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
INT_PTR CALLBACK MainWindow::LongLatHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			std::unique_ptr<LongLat> coords{ new LongLat() };
			wchar_t longitudeString[512] = { 0 };
			SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_GETTEXT, sizeof(longitudeString) / sizeof(longitudeString[0]), (LPARAM)longitudeString);

			wchar_t* nullTerminator = (wchar_t*)memchr(longitudeString, 0, sizeof(longitudeString));
			coords->longitude = -wcstod(longitudeString, &nullTerminator);

			wchar_t latitudeString[512] = { 0 };
			SendMessage(GetDlgItem(hDlg, IDC_EDIT2), WM_GETTEXT, sizeof(latitudeString) / sizeof(latitudeString[0]), (LPARAM)latitudeString);

			nullTerminator = (wchar_t*)memchr(latitudeString, 0, sizeof(latitudeString));
			coords->latitude = wcstod(latitudeString, &nullTerminator);

			EndDialog(hDlg, LOWORD(wParam));

			static_assert(sizeof(WPARAM) == sizeof(LongLat*), "OOps");

			PostMessage(GetParent(hDlg), LONGLATMSG, WPARAM(coords.release()), NULL);

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
