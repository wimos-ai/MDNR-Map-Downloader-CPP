#pragma once

//Windows Headders
#include <Windows.h>
#include <gdiplus.h>
#include <winhttp.h>

//Using Windows GDI Namespace
using Gdiplus::Bitmap;

#include "Location_t.h"


class MDNR_Map_Connection
{
public:
	Bitmap* download(Location_t location);
	MDNR_Map_Connection();
	~MDNR_Map_Connection();
private:
	//HTTP Connection Handles
	const HINTERNET session_h = NULL;
	const HINTERNET connect_h = NULL;
};

