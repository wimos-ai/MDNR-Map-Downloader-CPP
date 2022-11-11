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
}

void MDNR_Map::cacheArea(Location_t center, uint16_t radius)
{
	Location_t top_left(center.x - radius, center.y - radius, center.layer);
	Location_t bottom_right(center.x + radius, center.y + radius, center.layer);
	cacheArea(top_left, bottom_right, 0);
}

void  MDNR_Map::cacheArea(Location_t top_left, Location_t bottom_right, int boarder_offset) {
	if (bottom_right.layer != top_left.layer)
	{
		throw std::invalid_argument("top_left and bottom_right must be on the same layer");
	}

	std::vector<Location_t> list;

	list.reserve((bottom_right.x - top_left.x) * (bottom_right.y - top_left.y));
	for (int x = top_left.x - boarder_offset; x <= bottom_right.x + boarder_offset; x++){
		for (int y = bottom_right.y + boarder_offset; y <= top_left.y - boarder_offset; y++) {
			list.emplace_back(x, y, top_left.layer);
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
		return internal_cache.at(location).get();
	}
	else {
		std::unique_ptr<Bitmap> tmp(download_img(connect_h, location));
		Bitmap* ret = tmp.get();
		std::lock_guard<std::mutex> lck(lock);
		internal_cache[location] = std::move(tmp);
		return ret;
	}
}

void MDNR_Map::clear_cache() {
	std::lock_guard<std::mutex> lck(lock);
	this->internal_cache.clear();
}

void _cache_list(MDNR_Map* self, std::vector<Location_t>* list) {
	std::unique_ptr<std::vector<Location_t>> raii(list);

	for (Location_t &i : *list)
	{
		self->get(i);
	}
	return;
}

void MDNR_Map::cache_list(std::vector<Location_t>& locations)
{
	const unsigned int num_threads = std::thread::hardware_concurrency();

	//If we have more hardware threads than requests, dole out one request to each thread
	if (locations.size() <= num_threads){
		for (auto& it : locations) {
			std::thread t(
				[it, this]() {
					this->get(it);
				});

			if (t.joinable())
			{
				t.detach();
			}
		}
	}
	else { //Dole out locations.size()/num_threads requests to each thread
		using std::vector;
		using std::unique_ptr;
		size_t max_requests = ceil(locations.size() / (double)num_threads);

		vector<unique_ptr<vector<Location_t>>> requests;
		for (size_t i = 0; i < requests.size(); i++)
		{
			requests.emplace_back(new vector<Location_t>);
			requests[i]->reserve(max_requests); 
		}

		size_t count = locations.size();
		size_t requests_index = 0;
		size_t location_index = 0;
		while (count > 0) {
			requests[requests_index]->push_back(locations[location_index]);
			location_index++;
			requests_index++;
			count--;
			if (requests_index >= requests.size())
			{
				requests_index = 0;
			}
		}

		for (auto &req : requests)
		{
			std::thread t(_cache_list, this, req.release());
			if (t.joinable())
			{
				t.detach();
			}
		}

	}

}

void MDNR_Map::close_http_handles() {
	if (this->connect_h) WinHttpCloseHandle(this->connect_h);
	if (this->session_h) WinHttpCloseHandle(this->session_h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////