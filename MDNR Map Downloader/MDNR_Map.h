#pragma once

//Required Windows Headers
#include <Windows.h>
#include <winhttp.h>

//Add Windows libs
#pragma comment(lib, "winhttp.lib")

//Required std headers
#include <map>
#include <stdint.h>
#include <vector>
#include <mutex>

//Custom defined data types
#include "Location_t.h"
#include "IMG_t.h"

class MDNR_Map
{
private:
	//A Cache because the map is likely accessed multiple times. Now we don't need to take the time to request each item
	std::map<Location_t, IMG_t> internal_cache;

	//HTTP Connection Handles
	const HINTERNET  session_h = NULL, connect_h = NULL;

	//Mutex for threaded gets and caching
	std::mutex lock;


	/// <summary>
	/// Downloads the PNG from the MDNR database. 
	/// </summary>
	/// <param name="location">The location specifing which map pannel to download</param>
	/// <returns>An vector containing the bytes encoded as a PNG represented the selected pannel </returns>
	std::vector<uint8_t> download_img_bytes(Location_t location);

	/// <summary>
	/// Closes the internal HTTP handles. Used in the destructor
	/// </summary>
	void close_http_handles();

	/// <summary>
	/// Copies over session_h, connect_h and cache
	/// </summary>
	/// <param name="other">The MDNR_Map to create from</param>
	MDNR_Map(MDNR_Map& other);


public:
	/// <summary>
	/// Returns an image from the map in a thread safe manner
	/// </summary>
	/// <param name="location">The location specifing which map pannel to retreive</param>
	/// <returns></returns>
	IMG_t get(Location_t location);

	/// <summary>
	/// Clears the internal cache. Can be used to free up memory
	/// </summary>
	void clear_cache();

	/// <summary>
	/// Uses all availible threads to cache the list of locations provided
	/// </summary>
	void cache_list(std::vector<Location_t>& locations);


	/// <summary>
	/// Creates a underlying HTTP Session and connects to the MDNR server.
	/// Throws std::runtime_exception on failure
	/// </summary>
	MDNR_Map();

	/// <summary>
	/// Connects to the MDNR server using the provided session
	/// Throws std::runtime_exception on failure
	/// </summary>
	/// <param name="_hSession">A handle to an existing HTTP Session made from WinHttpOpen</param>
	MDNR_Map(HINTERNET  _hSession);

	///<summary>Default Destructor</summary>
	~MDNR_Map();

	/// <summary>
	/// Uses all availible threads to cache area provided
	/// </summary>
	void cacheArea(Location_t center, uint16_t radius);

	bool contains(Location_t location);

	static constexpr int pannel_width = 256;

	static constexpr int pannel_height = 256;

};
