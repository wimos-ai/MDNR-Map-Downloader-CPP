#include "MDNR_Map.h"


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include "Location_t.h"
//Additional Required std headers
#include <string>
#include <stdexcept>
#include <thread>
#include <cmath>
#include <utility>


MDNR_Map_Worker::MDNR_Map_Worker() :run(true), tasks(), lock(), thd([this]() {
	while (this->run) {
		sem.acquire();
		Task tsk{ 0 };
		{
			std::unique_lock<std::mutex> lck(this->lock);
			if (!tasks.empty())
			{
				tsk = tasks.front();
				tasks.pop();
			}
		}

		if (tsk != 0)
		{
			try {
				tsk();
			}
			catch (...) {

			}
		}

	}
	}) {
}

void MDNR_Map_Worker::addTask(Task task) {
	std::unique_lock<std::mutex> lck(this->lock);
	tasks.push(task);
	sem.release();
}

MDNR_Map_Worker::~MDNR_Map_Worker() {
	kill();
	thd.join();
}

void MDNR_Map_Worker::kill() {
	this->run = false;
	sem.release();
}

void MDNR_Map_Worker::clear() {
	std::unique_lock<std::mutex> lck(this->lock);
	while (!tasks.empty())
	{
		tasks.pop();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////



std::vector<Location_t> locationsInArea(Location_t top_left, Location_t bottom_right, int boarder_offset) {
	std::vector<Location_t> list;

	int width_panels(bottom_right.x - top_left.x);
	int height_panels(bottom_right.y - top_left.y);


	list.reserve(width_panels * height_panels);
	for (int x = -boarder_offset; x < width_panels + boarder_offset; x++) {
		for (int y = -boarder_offset; y < height_panels + boarder_offset; y++) {
			list.emplace_back(top_left.x + x, top_left.y + y, top_left.layer);
		}
	}
	return list;
}



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

	for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		workers.emplace_back(new MDNR_Map_Worker());
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

	for (size_t i = 0; i < workers.size(); i++)
	{
		delete workers[i].release();
	}

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

	auto list(locationsInArea(top_left, bottom_right, boarder_offset));
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


std::shared_ptr<Gdiplus::Bitmap> MDNR_Map::get(Location_t location) {
	if (this->contains(location)) {
		std::lock_guard<std::mutex> lck(lock);
		return internal_cache.at(location);
	}
	else {
		std::unique_ptr<Bitmap> tmp(download_img(connect_h, location));
		std::lock_guard<std::mutex> lck(lock);
		internal_cache[location] = std::move(tmp);
		return internal_cache[location];
	}
}

void MDNR_Map::clear_cache() {
	std::lock_guard<std::mutex> lck(lock);
	this->internal_cache.clear();
	for (auto& it : workers)
	{
		it->clear();
	}
}

void _cache_list(MDNR_Map* self, std::vector<Location_t>* list) {
	std::unique_ptr<std::vector<Location_t>> raii(list);

	for (Location_t& i : *list)
	{
		self->get(i);
	}
	return;
}

void MDNR_Map::cache_list(std::vector<Location_t>& locations)
{
	const unsigned int num_workers = workers.size();

	//If we have more hardware threads than requests, dole out one request to each thread
	size_t worker_idx = 0;
	for (auto& it : locations) {
		Task t = [it, this]() {
			this->get(it);
		};
		workers[worker_idx]->addTask(t);
		worker_idx++;
		if (worker_idx >= num_workers)
		{
			worker_idx = 0;
		}
	}



}

void MDNR_Map::close_http_handles() {
	if (this->connect_h) WinHttpCloseHandle(this->connect_h);
	if (this->session_h) WinHttpCloseHandle(this->session_h);
}

void MDNR_Map::trimToArea(Location_t top_left, Location_t bottom_right, int boarder_offset) {
	using std::vector;
	using std::pair;
	using std::unique_ptr;

	vector<Location_t> list(locationsInArea(top_left, bottom_right, boarder_offset));
	vector<pair<Location_t, std::shared_ptr<Gdiplus::Bitmap>>> kept_images;

	std::lock_guard<std::mutex> lck(lock);

	for (int i = 0; i < list.size(); i++)
	{
		if (internal_cache.find(list[i]) != internal_cache.end())
		{
			kept_images.emplace_back(list[i], this->internal_cache.at(list[i]));
		}
	}

	internal_cache.clear();

	for (auto& it : kept_images)
	{
		internal_cache[it.first] = std::move(it.second);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////