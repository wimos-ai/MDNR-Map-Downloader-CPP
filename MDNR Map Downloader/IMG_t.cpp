//Windows Headers
#include <Windows.h>
#include <gdiplus.h>
#include <winhttp.h>
#include <Objbase.h>
#include <shlwapi.h>

//Additional std headers
#include <vector>
#include <array>

#include <stdint.h>
#include <string>
#include <memory>
#include <stdexcept>

//My Includes
#include "IMG_t.h"
#include "Location_t.h"
#include "MDNR_Map.h"
#include "MainWindow.h"

//Including Required Window libs
#pragma comment(lib, "winhttp.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")


//Using Windows GDIplus Bitmap
using Gdiplus::Bitmap;

/// <summary>
/// In internal linkage for these helper functions
/// </summary>
namespace {

	using namespace Gdiplus;

	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		ImageCodecInfo* pImageCodecInfo = NULL;

		GetImageEncodersSize(&num, &size);
		if (size == 0)
			return -1;  // Failure

		pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL)
			return -1;  // Failure

		GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success
			}
		}

		free(pImageCodecInfo);
		return -1;  // Failure
	}

	typedef struct RGB_Struct {
		BYTE R;
		BYTE G;
		BYTE B;
	}RGB_Struct;

	bool operator==(const RGB_Struct& a, const RGB_Struct& b) {
		return a.R == b.R && a.G == b.G && a.B == b.B;
	}


	/// <summary>
	/// Iterates through the bitmap. If the pixel is in the list of allowed colors, it is changed to black, otherwise, white
	/// </summary>
	/// <typeparam name="S">Number of allowed colors</typeparam>
	/// <param name="bitmap">Bitmap</param>
	/// <param name="colors">Colors to be changed to black</param>
	template<size_t S>
	void threshold(Bitmap* bitmap, const std::array<RGB_Struct, S>& colors) {

		const UINT bmp_width = bitmap->GetWidth();
		const UINT bmp_height = bitmap->GetHeight();

		//Lock the bitmap buffer
		BitmapData bitmapData;
		Rect rect(0, 0, bmp_width, bmp_height);
		bitmap->LockBits(&rect, ImageLockModeRead | ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);

		// Get a pointer to the bitmap data
		unsigned char* pixelData = (unsigned char*)bitmapData.Scan0;

		// Get the stride (number of bytes per row)
		int bytesPerPixel = 4; // number of bytes per pixel for PixelFormat32bppARGB
		int stride = bytesPerPixel * bmp_width;

		// Iterate through each pixel in the bitmap
		for (size_t y = 0; y < bmp_height; y++) {
			for (size_t x = 0; x < bmp_width; x++) {
				// Calculate the index of the current pixel
				size_t index = y * stride + x * bytesPerPixel;
				bool changed = false;

				const RGB_Struct pixelColor{ pixelData[index + 2], pixelData[index + 1], pixelData[index] };

				//If the current pixel is in colors
				if (std::find(colors.begin(), colors.end(), pixelColor) != colors.end())
				{
					// Set it as black
					pixelData[index + 2] = 0; // Red channel
					pixelData[index + 1] = 0; // Green channel
					pixelData[index] = 0;     // Blue channel
				}
				else {
					//Set it white
					pixelData[index + 2] = 255; // Red channel
					pixelData[index + 1] = 255; // Green channel
					pixelData[index] = 255;     // Blue channel
				}
			}
		}

		//Unlock bitmap
		bitmap->UnlockBits(&bitmapData);
	}

	std::unique_ptr<Bitmap> bitmapFromArea(Location_t top_left, Location_t bottom_right) {
		//Step 1: Begin caching
		MDNR_Map mdnr_map;
		mdnr_map.cacheArea(top_left, bottom_right, 0); //Bitmaps are being pulled in the background\

		//Step 2: Establish Boundaries
		const int bitmapWidth = MDNR_Map::bitmap_width;
		const int bitmapHeight = MDNR_Map::bitmap_height;

		const int numColumns = bottom_right.x - top_left.x;
		const int numRows = bottom_right.y - top_left.y;

		const int stitchedWidth = bitmapWidth * numColumns;
		const int stitchedHeight = bitmapHeight * numRows;

		//Step 3: Construct Bitmap to hold the data
		// Create the final stitched bitmap
		std::unique_ptr<Bitmap> stitchedBitmap{ new Bitmap(stitchedWidth, stitchedHeight) };
		Gdiplus::Graphics graphics(stitchedBitmap.get());

		// Iterate through each bitmap in the 2D array and draw it onto the final stitched bitmap
		for (int row = 0; row < numRows; row++)
		{
			for (int col = 0; col < numColumns; col++)
			{
				const int x = col * bitmapWidth;
				const int y = row * bitmapHeight;
				Location_t l(top_left.x + col, top_left.y + row, top_left.layer);
				graphics.DrawImage(mdnr_map.get(l).get(), x, y, bitmapWidth, bitmapHeight);
			}
		}
		return stitchedBitmap;
	}

	std::wstring getFileExtension(const std::wstring& fileFormat) {
		size_t dotIndex = fileFormat.find_last_of(L'.');
		if (dotIndex != std::wstring::npos) {
			return fileFormat.substr(dotIndex + 1);
		}
		else {
			return L"";
		}
	}


	int saveBitmap(Bitmap* bmp, const wchar_t* fileName) {

		auto clsidExt = L"image/" + getFileExtension(fileName);

		CLSID pngClsid;
		if (GetEncoderClsid(clsidExt.c_str(), &pngClsid) == -1) {
			return EXIT_FAILURE;
		}
		bmp->Save(fileName, &pngClsid, NULL);
		return EXIT_SUCCESS;
	}
}


void screenshot(HWND hWnd, std::unique_ptr<wchar_t> fileName)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	const int width = rect.right - rect.left;
	const int height = rect.bottom - rect.top;

	HDC hdcScreen = GetDC(NULL);
	HDC hdcWindow = GetDC(hWnd);
	HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

	HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);

	// Select the bitmap object into the DC
	SelectObject(hdcMemDC, hbmScreen);

	// Bit block transfer into our compatible memory DC.
	if (!BitBlt(hdcMemDC,
		0, 0,
		width, height,
		hdcWindow,
		0, 0,
		SRCCOPY))
	{
		return;
	}

	// Create a new Bitmap object
	Bitmap* bmp = new Bitmap(hbmScreen, NULL);

	// Save the Bitmap object to a file
	saveBitmap(bmp, fileName.get());

	// Clean up resources
	DeleteObject(hbmScreen);
	DeleteDC(hdcMemDC);
	ReleaseDC(NULL, hdcScreen);
	ReleaseDC(hWnd, hdcWindow);
	delete bmp;
}

void saveArea(Location_t top_left, Location_t bottom_right, std::unique_ptr<wchar_t> fileName) {

	std::unique_ptr<Bitmap> bmp{ bitmapFromArea(top_left, bottom_right) };

	if (saveBitmap(bmp.get(), fileName.get()) == EXIT_FAILURE) {
		MessageBox(NULL, L"Saving Bitmap Error", L"Failed", MB_OK);
	}
}

void saveAreaThresholded(Location_t top_left, Location_t bottom_right, std::unique_ptr<wchar_t> fileName) {
	//White Listed Pixels
	//(85, 199, 251),
	//(68, 189, 242),
	//(52, 181, 232),
	//(20, 165, 213),
	//(40, 172, 218),
	//(4, 156, 207),
	//(50,157,185)

	const std::array<RGB_Struct, 7> colors{ {{85, 199, 251},
		{68, 189, 242},
		{52, 181, 232},
		{20, 165, 213},
		{40, 172, 218},
		{4, 156, 207},
		{50,157,185}} };

	std::unique_ptr<Bitmap> bmp{ bitmapFromArea(top_left, bottom_right) };

	threshold(bmp.get(), colors);

	if (saveBitmap(bmp.get(), fileName.get()) == EXIT_FAILURE) {
		MessageBox(NULL, L"Saving Bitmap Error", L"Failed", MB_OK);
	}
}
