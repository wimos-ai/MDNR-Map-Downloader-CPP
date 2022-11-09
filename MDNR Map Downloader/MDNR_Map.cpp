#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "Location_t.h"
#include "MDNR_Map.h"

//Additional Required std headers
#include <string>
#include <stdexcept>
#include <thread>
#include <cmath>





//// MDNR_Map methods and functions ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

MDNR_Map::MDNR_Map() :
	session_h(WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0)),
	connect_h(WinHttpConnect(session_h, L"maps1.dnr.state.mn.us", INTERNET_DEFAULT_HTTPS_PORT, 0))
{

	if (!this->session_h)
	{
		throw std::runtime_error("Failed to establish Win32 HTTP Session. Win32 Error Code: " + GetLastError());
	}

	if (!this->connect_h)
	{
		throw std::runtime_error("Failed to connect to maps1.dnr.state.mn.us. Win32 Error Code: " + GetLastError());
	}
}

MDNR_Map::MDNR_Map(HINTERNET _hSession) :
	session_h(_hSession),
	connect_h(WinHttpConnect(session_h, L"maps1.dnr.state.mn.us", INTERNET_DEFAULT_HTTPS_PORT, 0))
{
	if (!session_h)
	{
		throw std::runtime_error("Passed Invalid Win32 HTTP Session");
	}

	if (!this->connect_h)
	{
		throw std::runtime_error("Failed to connect to maps1.dnr.state.mn.us. Win32 Error Code: " + GetLastError());
	}

}

MDNR_Map::~MDNR_Map() {
	close_http_handles();
	this->clear_cache();
}

void MDNR_Map::cacheArea(Location_t center, uint16_t radius)
{
	std::vector<Location_t> list;

	list.reserve(static_cast<size_t>((static_cast<int>(radius) * 2) * (static_cast<int>(radius) * 2)));
	for (int x = -radius; x < radius; x++)
	{
		for (int y = -radius; y < radius; y++) {
			Location_t get_location = center;
			get_location.x = get_location.x + x;
			get_location.y = get_location.y + y;

			list.push_back(get_location);
		}
	}
	this->cache_list(list);
}

bool MDNR_Map::contains(Location_t location)
{
	std::lock_guard<std::mutex> lck(lock);
	return (internal_cache.find(location) != internal_cache.end());
}

MDNR_Map::MDNR_Map(MDNR_Map& other) :session_h(other.session_h), connect_h(other.connect_h) {
	//Don't need to validate that session_h and connect_h are non-null because a MDNR_Map cannot be created without those values being non-null
}

IMG_t MDNR_Map::get(Location_t location) {
	if (this->contains(location)) {
		std::lock_guard<std::mutex> lck(lock);
		return internal_cache.at(location);
	}else {
		auto tmp = download_img(connect_h,location);
		std::lock_guard<std::mutex> lck(lock);
		internal_cache[location] = tmp;
		return internal_cache[location];
	}
}



void MDNR_Map::clear_cache() {
	std::lock_guard<std::mutex> lck(lock);
	this->internal_cache.clear();
}

void _cache_list(MDNR_Map* self, std::vector<Location_t>* list) {

	for (auto& i : *list)
	{
		self->get(i);
	}

	delete list;
	return;
}

void MDNR_Map::cache_list(std::vector<Location_t>& locations)
{
	using std::vector;
	const unsigned int num_threads = std::thread::hardware_concurrency();

	if ((num_threads > 1) && (locations.size() > num_threads))
	{
		//Distribute each thread a list of locations


		// Allocate a vector for each future thread
		vector <vector<Location_t>*> lists;// ((size_t)num_threads);

		lists.reserve(num_threads);

		const size_t storage_required = std::ceil(locations.size() / (double)num_threads);
		for (size_t i = 0; i < num_threads; i++)
		{
			// Each thread will service ~locations.size() / num_threads locations
			// We round up to cover the extra
			lists.push_back(new vector<Location_t>());
			lists[i]->reserve(storage_required);
		}

		size_t idx = 0;
		for (size_t i = 0; i < locations.size(); i++) {
			idx = idx % num_threads;
			lists[idx]->push_back(locations[i]);
			idx++;
		}

		//Errors happen around here
		for (size_t i = 0; i < num_threads; i++)
		{
			std::thread t(_cache_list, this, lists[i]);
			if (t.joinable())
			{
				t.detach();
			}
		}

	}
	else {
		//Fall Back and give one thread all the locations.


		std::thread t(_cache_list, this, new vector<Location_t>(locations));
		if (t.joinable())
		{
			t.detach();
		}
	}

}

void MDNR_Map::close_http_handles() {
	if (this->connect_h) WinHttpCloseHandle(this->connect_h);
	if (this->session_h) WinHttpCloseHandle(this->session_h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////