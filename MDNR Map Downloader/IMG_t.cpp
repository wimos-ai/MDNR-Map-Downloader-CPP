//Windows Headers
#include <Windows.h>
#include <shlwapi.h>

//Additional std headers
#include <vector>
#include <stdint.h>
#include <string>
#include <memory>
#include <stdint.h>
#include <stdexcept>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <tchar.h>
#include <signal.h>

//My Includes
#include "IMG_t.h"
#include "httpException.h"
#include "Location_t.h"
#include "MDNR_Map.h"
#include "MainWindow.h"
#include "EasyBitmap.h"

//Including Required Window libs
#pragma comment(lib, "winhttp.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")

/// <summary>
/// In internal linkage for these helper functions
/// </summary>
namespace {

	/// <summary>
	/// returns a vector of vectors (with the contained vectors representing columns of pixels in the image) when passed a file path:
	/// </summary>
	/// <param name="filename">File name</param>
	/// <returns>returns a vector of vectors (with the contained vectors representing columns of pixels in the image) when passed a file path</returns>
	std::vector<std::vector<unsigned>> getPixels(const wchar_t* filename) {
		Gdiplus::Bitmap bitmap(filename);

		//Pass up the width and height, as these are useful for accessing pixels in the vector o' vectors.
		int width = bitmap.GetWidth();
		int height = bitmap.GetHeight();

		Gdiplus::BitmapData* bitmapData = new Gdiplus::BitmapData;

		//Lock the whole bitmap so we can read pixel data easily.
		Gdiplus::Rect rect(0, 0, width, height);
		bitmap.LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead, PixelFormat32bppARGB, bitmapData);

		//Get the individual pixels from the locked area.
		unsigned int* pixels = static_cast<unsigned*>(bitmapData->Scan0);

		//Vector of vectors; each vector is a column.
		std::vector<std::vector<unsigned>> resultPixels(width, std::vector<unsigned>(height));

		const int stride{ abs(bitmapData->Stride) };
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				//Get the pixel colour from the pixels array which we got earlier.
				const unsigned pxColor = pixels[y * stride / 4 + x];

				//Get each individual colour component. Bitmap colours are in reverse order.
				const unsigned red = (pxColor & 0xFF0000) >> 16;
				const unsigned green = (pxColor & 0xFF00) >> 8;
				const unsigned blue = pxColor & 0xFF;

				//Combine the values in a more typical RGB format (as opposed to the bitmap way).
				const int rgbValue = RGB(red, green, blue);

				//Assign this RGB value to the pixel location in the vector o' vectors.
				resultPixels[x][y] = rgbValue;
			}
		}

		//Unlock the bits that we locked before.
		bitmap.UnlockBits(bitmapData);
		return resultPixels;
	}

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
		std::unique_ptr<Bitmap> bmp{ new Bitmap(stream, FALSE) };
		if (bmp.get() == nullptr)
		{
			std::string error_string = "BMP Creation Failed. Win32 Error: " + std::to_string(GetLastError()) + '.';
			throw std::runtime_error(error_string);
		}
		stream->Release();
		return bmp.release();
	}

	/// <summary>
	/// Creates a IMG_t from a std::vector<uint8_t>
	/// </summary>
	/// <param name="vec">A array of valid image bytes</param>
	/// <returns>An Image made from those bytes</returns>
	IMG_t fromVec(std::vector<uint8_t>& vec) {
		IStream* stream = win_stream_from_vec(vec);
		IMG_t im = fromStream(stream);
		return im;
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
			throw httpException("Failed to create Win32 Request. Win32 Error Code: ", GetLastError());
		}

		// Send a request.
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);

		if (bResults == FALSE)
		{
			if (hRequest) WinHttpCloseHandle(hRequest);
			//TODO, Make a popup if the wifi is not connected
			throw httpException("Failed to make send Win32 Request. Win32 Error Code: ", GetLastError());
		}


		// End the request.
		bResults = WinHttpReceiveResponse(hRequest, NULL);

		if (bResults == FALSE)
		{
			if (hRequest) WinHttpCloseHandle(hRequest);
			throw httpException("Failed to make send Win32 Request. Win32 Error Code: ", GetLastError());
		}

		// Keep checking for data until there is nothing left.
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
				if (hRequest) WinHttpCloseHandle(hRequest);
				throw httpException("Failed to make send Win32 Request. Win32 Error Code: ", GetLastError());
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
		Gdiplus::Color c;
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
/// <param name="x_in">The y-coordinate of the top left corner</param>
/// <param name="y_in">The y-coordinate of the top left corner</param>
void draw_IMG(IMG_t im, HDC dst, int x_in, int y_in)
{

	Gdiplus::Graphics g(dst);

	g.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceCopy);

	g.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeNearestNeighbor);

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
	GetDIBits(hdcScreen, hbmScreen, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

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


#pragma pack()
struct bitmapHeader {
	char front[2];
	uint32_t fileSize;
	uint32_t reserved;
	uint32_t offsetToBMPData;
}bitmapHeader;

#pragma pack()
struct bitmapInfoHeader {
	uint32_t sizeInfoHeader;
	uint32_t widthPixels;
	uint32_t heightPixels;
	uint16_t numPlanes;
	uint16_t bitsPerPixel;
	uint32_t compression;
	uint32_t imSizeCompression;
	uint32_t xPixPerM;
	uint32_t yPixPerM;
	uint32_t colorsUsed;
	uint32_t importantColors;
}bitmapInfoHeader;

void saveArea(Location_t top_left, Location_t bottom_right, wchar_t* fileName) {
	/*
		Because of the extremely high amount of memory this function may require, I must do this in a extremely low level manner
		Here be dragons, good luck!


		My basic idea is this, because the bitmap file specification writes the bitmap line by line from the bottom up,
		I can download each row of bitmaps and stream the composite line from each bitmap to the file without having to combine the bitmaps in memory.
		This sidesteps the contiguious memory requirement and allows me to write obcenely large bitmaps to a file.

		Because I know the area I am downloading, I can precompute my headers and write them out to the file ahead of time.

		The specification can be found here: http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm

	*/

	std::unique_ptr<wchar_t> fnameRAII{ fileName };

	//Step 1: Calculate Number of pannels
	const unsigned int num_width_pannels{ static_cast<unsigned int>(std::abs(top_left.x - bottom_right.x)) };
	const unsigned int num_height_pannels{ static_cast<unsigned int>(std::abs(top_left.y - bottom_right.y)) };

	const unsigned int num_width_pixels{ static_cast<unsigned int>(num_width_pannels * MDNR_Map::bitmap_width) };
	const unsigned int num_height_pixels{ static_cast<unsigned int>(num_height_pannels * MDNR_Map::bitmap_height) };

	constexpr int bitsPerPixel = 32;
	constexpr int bytesPexPixel = 4;


	//Step 2: Compute My header
	struct bitmapHeader bmh { 0 };
	bmh.front[0] = 'B';
	bmh.front[1] = 'M';
	bmh.fileSize = sizeof(struct bitmapHeader) + sizeof(struct bitmapInfoHeader) + (num_width_pixels * num_height_pixels * bytesPexPixel);
	bmh.offsetToBMPData = sizeof(struct bitmapHeader) + sizeof(struct bitmapInfoHeader);

	//Step 3: Compute Info Header
	struct bitmapInfoHeader bmi { 0 };
	bmi.sizeInfoHeader = sizeof(struct bitmapInfoHeader);
	bmi.widthPixels = num_width_pixels;
	bmi.heightPixels = num_height_pixels;
	bmi.numPlanes = 0;
	bmi.bitsPerPixel = bitsPerPixel;
	bmi.xPixPerM = 1;
	bmi.yPixPerM = 1;

	//Step 4: Begin caching
	MDNR_Map mdnr_map;
	mdnr_map.cacheArea(top_left, bottom_right, 0); //Bitmaps are being pulled in the background

	//Step 5: Open the output file
	std::ofstream fout;
	fout.open(fileName, std::ios::binary | std::ios::out);

	//Step 6: Write the headers
	fout.write((char*)&bmh, sizeof(bmh));
	fout.write((char*)&bmi, sizeof(bmi));

	const auto hdlr = signal(SIGSEGV, [](int i) {
		throw std::exception("Bad Memory access");
	});


	//Step 7: Iterate through each row of bitmaps from the bottom up and write each line of bitmap to the file
	for (int y = 0; y < num_height_pannels; y++)
	{

		std::vector<EasyBitmap> row; //Set up container
		row.reserve(num_width_pannels); //Allocate space for bitmaps

		//Download the row and lock each bitmap for use
		for (size_t x = 0; x < num_width_pannels; x++)
		{
			auto im = mdnr_map.get(Location_t(top_left.x + x, bottom_right.y + y, top_left.layer)).get();

			auto vec = getPixels(*im);

			EasyBitmap curr(im);

			//Add it to vector
			row.push_back(std::move(curr));
		}

		for (size_t y = MDNR_Map::bitmap_height; y != 0; y--) {
			for (auto& bitmap : row)
			{
				for (size_t i = 0; i < bitmap.width(); i++)
				{
					ARGB_Pixel pix = *bitmap.pixelAt(i, y);
					fout.write((char*)&pix, sizeof(ARGB_Pixel));
				}
			}
		}
	}

	signal(SIGSEGV, hdlr);
}


std::vector<std::vector<unsigned>> getPixels(Gdiplus::Bitmap& bitmap) {
	//Pass up the width and height, as these are useful for accessing pixels in the vector o' vectors.
	auto width = bitmap.GetWidth();
	auto height = bitmap.GetHeight();

	auto* bitmapData = new Gdiplus::BitmapData;

	//Lock the whole bitmap so we can read pixel data easily.
	Gdiplus::Rect rect(0, 0, width, height);
	bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData);

	//Get the individual pixels from the locked area.
	auto* pixels = static_cast<unsigned*>(bitmapData->Scan0);

	//Vector of vectors; each vector is a column.
	std::vector<std::vector<unsigned>> resultPixels(width, std::vector<unsigned>(height));

	const int stride = abs(bitmapData->Stride);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			//Get the pixel colour from the pixels array which we got earlier.
			const unsigned pxColor = pixels[y * stride / 4 + x];

			//Get each individual colour component. Bitmap colours are in reverse order.
			const unsigned red = (pxColor & 0xFF0000) >> 16;
			const unsigned green = (pxColor & 0xFF00) >> 8;
			const unsigned blue = pxColor & 0xFF;

			//Combine the values in a more typical RGB format (as opposed to the bitmap way).
			const unsigned rgbValue = RGB(red, green, blue);

			//Assign this RGB value to the pixel location in the vector o' vectors.
			resultPixels[x][y] = rgbValue;
		}
	}

	//Unlock the bits that we locked before.
	bitmap.UnlockBits(bitmapData);
	return resultPixels;
}