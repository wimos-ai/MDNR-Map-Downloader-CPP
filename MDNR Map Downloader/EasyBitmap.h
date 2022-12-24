#pragma once

// Windows Header Files
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

//STL Header Files
#include <stdint.h>

typedef struct ARGB_Pixel {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t alpha;
}ARGB_Pixel;

static_assert(sizeof(ARGB_Pixel) == 4, "Check structure packing");

/// <summary>
/// Look at: https://stackoverflow.com/questions/3340017/gdiplusbitmap-to-byte-array
/// </summary>
class EasyBitmap
{
public:
	EasyBitmap(Gdiplus::Bitmap* bmp);

	ARGB_Pixel* pixelAt(int xidx, int yidx);

	~EasyBitmap();

	int width();

	int height();

private:
	Gdiplus::Bitmap* bmp;
	Gdiplus::BitmapData data;
};

