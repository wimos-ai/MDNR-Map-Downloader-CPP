#pragma once


//Windows Headders
#include <Windows.h>
#include <gdiplus.h>

#include <winhttp.h>

//Using Windows GDI Namespace
using Gdiplus::Bitmap;

//My Defines
#include "Location_t.h"

using IMG_t = Bitmap*;

/// <summary>
/// Downloads a Image from the MDNR Database
/// </summary>
/// <param name="connect_h">A connection to the MDNR database</param>
/// <param name="location">The location of the required Image</param>
/// <returns>The image in the given location</returns>
IMG_t download_img(HINTERNET connect_h, Location_t location);

/// <summary>
/// draws an Image to the provided HDC
/// </summary>
/// <param name="im">An image to draw</param>
/// <param name="dst">The destination HDC</param>
/// <param name="x">The X-Location of where to draw the image</param>
/// <param name="y">The Y-Location of where to draw the image</param>
void draw_IMG(IMG_t im, HDC dst, int x, int y);

void screenshot(HWND hwnd, wchar_t* fileName);