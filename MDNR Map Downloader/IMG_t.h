#ifndef IMG_T_H986132709870932
#define IMG_T_H986132709870932


//Windows Headders
#include <Windows.h>

//STL
#include <memory>

//My Defines
#include "Location_t.h"


/// <summary>/// 
/// Takes a screenshot of the contents of the provided HWND and saves it to the given file
/// </summary>/// 
/// <param name="hwnd">
/// Window handle to save
/// </param>/// 
/// <param name="fileName">
/// The path to save the resultant image at
/// </param>
void screenshot(HWND hwnd, std::unique_ptr<wchar_t> fileName);

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
void saveArea(Location_t top_left, Location_t bottom_right, std::unique_ptr<wchar_t> fileName);

void saveAreaThresholded(Location_t top_left, Location_t bottom_right, std::unique_ptr<wchar_t> fileName);

#endif // IMG_T_H986132709870932