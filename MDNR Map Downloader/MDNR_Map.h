#ifndef MDNR_MAP_H8163277
#define MDNR_MAP_H8163277

//Required Windows Headers
#include <Windows.h>
#include <winhttp.h>

//Required std headers
#include <map>
#include <stdint.h>
#include <vector>
#include <mutex>
#include <memory>

//Custom defined data types
#include "Location_t.h"
#include "IMG_t.h"
#include "WorkerThread.h"

//Add Windows libs
#pragma comment(lib, "winhttp.lib")

class MDNR_Map
{
public:

	/// <summary>
	/// Returns an image from the map in a thread safe manner
	/// </summary>
	/// <param name="location">The location specifing which map pannel to retreive</param>
	/// <returns></returns>
	std::shared_ptr<Gdiplus::Bitmap> get(Location_t location);

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
	explicit MDNR_Map(HINTERNET  _hSession);

	///<summary>Default Destructor</summary>
	~MDNR_Map();

	/// <summary>
	/// Uses all availible threads to cache area provided
	/// </summary>
	void cacheArea(Location_t center, uint16_t radius);

	/// <summary>
	/// Caches an area
	/// </summary>
	/// <param name="top_left"></param>
	/// <param name="bottom_right"></param>
	/// <param name="boarder_offset"></param>
	void cacheArea(Location_t top_left, Location_t bottom_right, int boarder_offset);

	/// <summary>
	/// Returns true if the location is a cached location
	/// </summary>
	/// <param name="location">A location to check</param>
	/// <returns>true if the location is cached</returns>
	bool contains(Location_t location);

	/// <summary>
	/// Trims the internal cache to hold only the area within the location bounds plus and offset
	/// </summary>
	/// <param name="top_left">Top left coordinate</param>
	/// <param name="bottom_right">Bottom left coordinate</param>
	/// <param name="boarder_offset">The area outside the two coordinates that are kept</param>
	void trimToArea(Location_t top_left, Location_t bottom_right, int boarder_offset);

	static constexpr int bitmap_width = 256;

	static constexpr int bitmap_height = 256;

private:
	/// <summary>
	/// Closes the internal HTTP handles. Used in the destructor
	/// </summary>
	void close_http_handles();

	/// <summary>
	/// Copies over session_h, connect_h and cache
	/// </summary>
	/// <param name="other">The MDNR_Map to create from</param>
	MDNR_Map(MDNR_Map& other);

	//A cache
	std::map<Location_t, std::shared_ptr<Gdiplus::Bitmap>> internal_cache;

	//HTTP Connection Handles
	const HINTERNET  session_h = NULL, connect_h = NULL;

	//Mutex for threaded downloads and cache requests
	std::mutex lock;

	//Threads to do non-blocking downloads
	std::vector<std::unique_ptr<WorkerThread>> workers;

};

#endif // MDNR_MAP_H8163277