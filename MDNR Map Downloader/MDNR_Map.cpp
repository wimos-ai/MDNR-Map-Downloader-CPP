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


std::vector<Location_t> locationsInArea(Location_t top_left, Location_t bottom_right, int boarder_offset);

//// Cache Methods //////////////////////////////////////////////////////////////////////////////////

void MDNR_Map::cacheArea(Location_t center, uint16_t radius)
{
	Location_t top_left(center.x - radius, center.y - radius, center.layer);
	Location_t bottom_right(center.x + radius, center.y + radius, center.layer);
	cacheArea(top_left, bottom_right, 0);
}

void  MDNR_Map::cacheArea(Location_t top_left, Location_t bottom_right, int boarder_offset) {
	if (bottom_right.layer != top_left.layer)
		throw std::invalid_argument("top_left and bottom_right must be on the same layer");

	std::vector<Location_t> list = std::move(locationsInArea(top_left, bottom_right, boarder_offset));
	this->cache_list_asyc(list);
}

void MDNR_Map::cache_list_asyc(std::vector<Location_t>& locations)
{
	//Dole out one request to each thread until we run out of requests
	for (auto l:locations)
	{
		Task t = [this, l]() {
			Bitmap* bmp = this->map_con.download(l);
			std::lock_guard<std::mutex>(this->lock);
			this->internal_cache[l] = std::shared_ptr<Bitmap>(bmp);
		};
		pool.submit_task(t);
	}

}

bool MDNR_Map::contains(Location_t location)
{
	std::lock_guard<std::mutex> lck(lock);
	return (internal_cache.find(location) != internal_cache.end());
}

std::shared_ptr<Gdiplus::Bitmap> MDNR_Map::get(Location_t location) {
	if (this->contains(location)) {
		std::lock_guard<std::mutex> lck(lock);
		return internal_cache[location];
	}
	else {
		std::unique_ptr<Bitmap> tmp(map_con.download(location));
		std::lock_guard<std::mutex> lck(lock);
		internal_cache[location] = std::move(tmp);
		return internal_cache[location];
	}
}

void MDNR_Map::clear_cache() {
	std::lock_guard<std::mutex> lck(lock);
	this->internal_cache.clear();
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

/// <summary>
/// Helper function that returns all the locations within an area
/// </summary>
/// <param name="top_left">Top Left coordinate point</param>
/// <param name="bottom_right">Bottom right coordinate point</param>
/// <param name="boarder_offset">The offset outside the coordinate point</param>
/// <returns>All the locations within an area</returns>
std::vector<Location_t> locationsInArea(Location_t top_left, Location_t bottom_right, int boarder_offset) {
	std::vector<Location_t> list;

	int width_panels{ bottom_right.x - top_left.x };
	int height_panels{ bottom_right.y - top_left.y };


	list.reserve(width_panels * height_panels);
	for (int x = -boarder_offset; x < width_panels + boarder_offset; x++) {
		for (int y = -boarder_offset; y < height_panels + boarder_offset; y++) {
			list.emplace_back(top_left.x + x, top_left.y + y, top_left.layer);
		}
	}
	return list;
}