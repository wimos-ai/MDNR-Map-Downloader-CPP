#ifndef IMG_T_H986132709870932
#define IMG_T_H986132709870932


//Windows Headders
#include <Windows.h>
#include <gdiplus.h>
#include <winhttp.h>

//Using Windows GDI Namespace
using Gdiplus::Bitmap;

//My Defines
#include "Location_t.h"

/// <summary>
/// Image type. Type ailias of Gdiplus::Bitmap*
/// </summary>
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

/// <summary>/// 
/// Takes a screenshot of the contents of the provided HWND and saves it to the given file
/// </summary>/// 
/// <param name="hwnd">
/// Window handle to save
/// </param>/// 
/// <param name="fileName">
/// The path to save the resultant image at
/// </param>
void screenshot(HWND hwnd, wchar_t* fileName);

/// <summary>/// 
///Saves all of the map between the two points to a file
/// </summary>/// 
/// <param name="top_left">
/// Top Left point in the map
/// </param>/// 
/// <param name="bottom_right">
/// Bottom Right point in the map
/// </param>
/// <param name="fileName">
/// The name of the file to save the result to
/// </param>
void saveArea(Location_t top_left, Location_t bottom_right, wchar_t* fileName);

#endif // IMG_T_H986132709870932