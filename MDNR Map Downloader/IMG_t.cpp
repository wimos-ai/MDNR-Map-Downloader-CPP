#include "IMG_t.h"

using namespace Gdiplus;

//Additional Windows Headers
//#include <objidl.h>
#include <shlwapi.h>


//Including Required Window libs
#pragma comment(lib, "winhttp.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")

//Additional std headers
//Required std headers
#include <vector>
#include <stdint.h>
#include <string>
#include <stdexcept>


/// <summary>
/// In internal linkage for these helper functions
/// </summary>
namespace {

	/// <summary>
	/// Creates a IStream from std::vector<uint8_t>
	/// </summary>
	/// <param name="vec">Bytes to create a IStream from</param>
	/// 
	/// <returns>BAn IStream containing a copy of the bytes in the vector</returns>
	IStream* win_stream_from_vec(std::vector<uint8_t>& vec)
	{
		const uint8_t* begin{ &(vec.cbegin()[0]) };
		return SHCreateMemStream(begin, static_cast<UINT>(vec.size()));
	}

	/// <summary>
	/// Creates a IMG_t from a IStream*
	/// </summary>
	/// <param name="stream">A stream containing valid Image Bytes</param>
	/// 
	/// <returns>An Image made from those bytes</returns>
	IMG_t fromStream(IStream* stream) {
		Bitmap* bmp = new Bitmap(stream, FALSE);
		if (bmp == nullptr)
		{
			std::string error_string = "BMP Creation Failed. Win32 Error: " + std::to_string(GetLastError()) + '.';
			throw std::runtime_error(error_string);
		}
		stream->Release();
		return bmp;
	}

	/// <summary>
	/// Creates a IMG_t from a std::vector<uint8_t>
	/// </summary>
	/// <param name="vec">A array of valid image bytes</param>
	/// <returns>An Image made from those bytes</returns>
	IMG_t fromVec(std::vector<uint8_t>& vec) {
		return fromStream(win_stream_from_vec(vec));
	}


	/// <summary>
	/// downloads image bytes from a MDNR location 
	/// </summary>
	/// <param name="connect_h">A valid HTTP connection to the MDNR database</param>
	/// 
	/// <param name="location">The location of the Image to acquire</param>
	/// 
	/// <param name="buffer">A buffer to place the bytes in. Suggested starting size: 2KB</param>
	void download_img_bytes(HINTERNET connect_h, Location_t location, std::vector<uint8_t>& buffer) {
		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		LPSTR pszOutBuffer = nullptr;
		BOOL  bResults = FALSE;
		HINTERNET	hRequest = NULL;

		std::wstring image_dst = L"/mapcache/gmaps/compass@mn_google/" + std::to_wstring(location.layer) + L"/" + std::to_wstring(location.x) + L"/" + std::to_wstring(location.y) + L".png";

		hRequest = WinHttpOpenRequest(connect_h, L"GET", image_dst.c_str(),
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

		buffer.reserve(2000);

		if (!hRequest)
		{
			throw std::runtime_error("Failed to create Win32 Request. Win32 Error Code: " + GetLastError());
		}

		// Send a request.
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);

		if (!bResults)
		{
			if (hRequest) WinHttpCloseHandle(hRequest);
			//TODO, Make a popup if the wifi is not connected
			throw std::runtime_error("Failed to make send Win32 Request. Win32 Error Code: " + GetLastError());
		}


		// End the request.
		bResults = WinHttpReceiveResponse(hRequest, NULL);

		if (!bResults)
		{
			if (hRequest) WinHttpCloseHandle(hRequest);
			throw std::runtime_error("Failed to make recieve response from Win32 Request. Win32 Error Code: " + GetLastError());
		}

		// Keep checking for data until there is nothing left.
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
				if (hRequest) WinHttpCloseHandle(hRequest);
				throw std::runtime_error("Error in WinHttpQueryDataAvailable. Win32 Error Code: " + GetLastError());
			}
			if (dwSize == 0)
			{
				break;
			}

			buffer.reserve(buffer.size() + dwSize);

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				if (hRequest) WinHttpCloseHandle(hRequest);
				throw std::bad_alloc();
			}
			else
			{
				// Read the data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
					if (hRequest) WinHttpCloseHandle(hRequest);
					throw std::runtime_error("Error in WinHttpReadData. Win32 Error Code: " + GetLastError());
				}
				else {
					for (size_t i = 0; i < dwSize; i++)
					{
						buffer.push_back(pszOutBuffer[i]);
					}
				}


			}
		} while (dwSize > 0);


		// Free the memory allocated to the buffer.
		delete[] pszOutBuffer;

		// Close any open handles.
		if (hRequest) WinHttpCloseHandle(hRequest);
	}

	/// <summary>
	/// Forces the loading of a provided bitmap
	/// </summary>
	/// <param name="gdip_bitmap">The bitmap to force load</param>
	void force_load(IMG_t gdip_bitmap) {
		Color c;
		gdip_bitmap->GetPixel(0, 0, &c);
		gdip_bitmap->SetPixel(0, 0, c);
	}

}

/// <summary>
/// Downloads an image from the MDNR website
/// </summary>
/// <param name="connect_h">The connection handle to use</param>
/// <param name="location">The location of the picture to grab</param>
/// <returns>The picture from that location</returns>
IMG_t download_img(HINTERNET connect_h, Location_t location) {
	std::vector<uint8_t> bytes;
	bytes.reserve(2000);
	download_img_bytes(connect_h, location, bytes);
	IMG_t gdip_bitmap = fromVec(bytes);
	force_load(gdip_bitmap);
	return gdip_bitmap;
}

/// <summary>
/// Draws an image to the provided HDC
/// </summary>
/// <param name="im">An image</param>
/// <param name="dst">Destination HDC</param>
/// <param name="x_in">The x-coordinate of the top left corner</param>
/// <param name="y_in">The y-coordinate of the top left corner</param>
void draw_IMG(IMG_t im, HDC dst, int x_in, int y_in)
{

	Graphics g(dst);

	g.SetCompositingMode(CompositingMode::CompositingModeSourceCopy);

	g.SetInterpolationMode(InterpolationMode::InterpolationModeNearestNeighbor);

	g.DrawImage(im, x_in, y_in);

}

/// <summary>
/// Takes a snapshot of the provided window, and saves it to a file
/// </summary>
/// <param name="hWnd">A window handle</param>
/// <param name="fileName">The name of the file in which to save the resulting image</param>
void screenshot(HWND hWnd, wchar_t* fileName) {
	HBITMAP hbmScreen{ NULL };
	BITMAP bmpScreen{ NULL };
	DWORD dwBytesWritten = 0;
	DWORD dwSizeofDIB = 0;
	HANDLE hFile = NULL;
	char* lpbitmap = NULL;
	HANDLE hDIB = NULL;
	DWORD dwBmpSize = 0;
	BITMAPFILEHEADER bmfHeader{};
	BITMAPINFOHEADER bi{};

	// Retrieve the handle to a display device context for the client 
	// area of the window. 
	const HDC hdcScreen = GetDC(hWnd);

	// Create a compatible DC, which is used in a BitBlt from the window DC.
	const HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

	if (!hdcMemDC)
	{
		MessageBox(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
		goto done;
	}

	// Get the client area for size calculation.
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	// This is the best stretch mode.
	SetStretchBltMode(hdcScreen, HALFTONE);


	// Create a compatible bitmap from the Window DC.
	hbmScreen = CreateCompatibleBitmap(hdcScreen, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

	if (!hbmScreen)
	{
		MessageBox(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
		goto done;
	}

	// Select the compatible bitmap into the compatible memory DC.
	SelectObject(hdcMemDC, hbmScreen);

	// Bit block transfer into our compatible memory DC.
	if (!BitBlt(hdcMemDC,
		0, 0,
		rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
		hdcScreen,
		0, 0,
		SRCCOPY))
	{
		MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
		goto done;
	}

	// Get the BITMAP from the HBITMAP.
	GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);


	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bmpScreen.bmWidth;
	bi.biHeight = bmpScreen.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
	// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
	// have greater overhead than HeapAlloc.
	hDIB = GlobalAlloc(GHND, dwBmpSize);
	if (!hDIB)
	{
		throw std::bad_alloc();
	}

	lpbitmap = (char*)GlobalLock(hDIB); // Fixed
	if (!lpbitmap)
	{
		throw std::bad_alloc();
	}

	// Gets the "bits" from the bitmap, and copies them into a buffer 
	// that's pointed to by lpbitmap.
	GetDIBits(hdcScreen, hbmScreen, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap,
		(BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// A file is created, this is where we will save the screen capture.
	hFile = CreateFile(fileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	// Add the size of the headers to the size of the bitmap to get the total file size.
	dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	// Size of the file.
	bmfHeader.bfSize = dwSizeofDIB;

	// bfType must always be BM for Bitmaps.
	bmfHeader.bfType = 0x4D42; // BM.

	WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

	// Unlock and Free the DIB from the heap.
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);

	// Close the handle for the file that was created.
	CloseHandle(hFile);

	// Clean up.
done:
	if (hbmScreen)
	{
		DeleteObject(hbmScreen);
	}
	if (hdcMemDC)
	{
		DeleteObject(hdcMemDC);
	}
	ReleaseDC(hWnd, hdcScreen);
}
