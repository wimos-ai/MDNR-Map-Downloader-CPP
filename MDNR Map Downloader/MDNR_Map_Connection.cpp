#include "MDNR_Map_Connection.h"

#include <stdexcept>
#include <string>

#include "httpException.h"

Bitmap* MDNR_Map_Connection::download(Location_t location)
{

	std::wstring image_dst = L"/mapcache/gmaps/compass@mn_google/" + std::to_wstring(location.layer) + L"/" + std::to_wstring(location.x) + L"/" + std::to_wstring(location.y) + L".png";

	HINTERNET hRequest = WinHttpOpenRequest(connect_h, L"GET", image_dst.c_str(),
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_SECURE);

	if (!hRequest)
	{
		throw httpException("WinHttpOpenRequest failed", GetLastError());
	}

	// Send request
	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		throw httpException("WinHttpSendRequest failed", GetLastError());
	}

	// Wait for response
	if (!WinHttpReceiveResponse(hRequest, NULL)) {
		throw httpException("WinHttpReceiveResponse failed", GetLastError());
	}


	// Get content length of the image
	DWORD content_length;
	DWORD buffer_size = sizeof(content_length);
	if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &content_length, &buffer_size, WINHTTP_NO_HEADER_INDEX)) {
		throw httpException("WinHttpQueryHeaders failed", GetLastError());
	}

	// Allocate buffer for the image data
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, content_length);
	if (hGlobal == NULL)
	{
		throw std::bad_alloc();
	}

	LPVOID buffer = GlobalLock(hGlobal);
	if (buffer == NULL)
	{
		throw std::bad_alloc();
	}

	// Read image data into buffer
	DWORD bytes_read;
	if (!WinHttpReadData(hRequest, buffer, content_length, &bytes_read)){
		throw httpException("WinHttpReadData failed", GetLastError());
	}
	

	// Create GDIPlus Bitmap from image data
	GlobalUnlock(hGlobal);

	IStream* pStream = NULL;
	if (CreateStreamOnHGlobal(hGlobal, FALSE, &pStream) != S_OK)
	{
		throw std::bad_alloc();
	}

	Bitmap* pBitmap = Bitmap::FromStream(pStream);

	// Clean up
	WinHttpCloseHandle(hRequest);
	pStream->Release();
	GlobalFree(hGlobal);
	return pBitmap;
}

MDNR_Map_Connection::MDNR_Map_Connection() : session_h(WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)),
connect_h(WinHttpConnect(session_h, L"maps1.dnr.state.mn.us", INTERNET_DEFAULT_HTTPS_PORT, 0))
{
	if (!this->session_h)
		throw std::runtime_error("Failed to establish Win32 HTTP Session. Win32 Error Code: " + GetLastError());

	if (!this->connect_h)
		throw std::runtime_error("Failed to connect to maps1.dnr.state.mn.us. Win32 Error Code: " + GetLastError());
}

MDNR_Map_Connection::~MDNR_Map_Connection()
{
	//Close Http Handles
	if (this->connect_h) WinHttpCloseHandle(this->connect_h);
	if (this->session_h) WinHttpCloseHandle(this->session_h);
}
