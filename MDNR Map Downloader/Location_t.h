#ifndef LOCATION_T_H097623708098732
#define LOCATION_T_H097623708098732

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <stdint.h>
#include <stdexcept>

/// <summary>
/// A 3-D coordinate specifying a location on the MDNR Map
/// </summary>
typedef struct Location_t {
	/// <summary>
	/// x coordinate
	/// </summary>
	uint16_t x;
	/// <summary>
	/// y coordinate
	/// </summary>
	uint16_t y;
	/// <summary>
	/// Layer, on range (0,16]
	/// </summary>
	uint8_t layer;

	static Location_t fromGPSCoords(double longitude, double latitude, uint8_t layer) {
		
		if (layer != 16)
		{
			throw std::invalid_argument("Layers other than 16 are not currently supported");
		}

		Location_t l(0, 0, layer);

		//These numbers were determined by drawing a linear correlation between map_x, and longitude
		l.x = static_cast<uint16_t>((182.038 * longitude) + 32766.9);

		//# These numbers were determined by drawing a linear correlation between map_y, and latitude
		l.y = static_cast<uint16_t>((-259.216 * latitude) + 35235.3);

		return l;
	}
	Location_t(uint16_t x, uint16_t y, uint8_t layer) :x(x), y(y), layer(layer) {}
	Location_t() :x(0), y(0), layer(0) {}

	void translateLayer(uint8_t newLayer) {
		if (newLayer < layer)
		{
			this->x /= (2 * layer - newLayer);
			this->y /= (2 * layer - newLayer);
			layer = newLayer;
		}
		else if(newLayer > layer) {
			this->x *= (2 * newLayer - layer );
			this->y *= (2 * newLayer - layer);
			layer = newLayer;
		}
		else {
			return;
		}
	}

	// The specialized hash function for `unordered_map` keys
	struct hash_fn
	{
		std::size_t operator() (const Location_t& location) const;
	};

}Location_t;

bool operator< (const Location_t& locationA, const Location_t& locationB);
bool operator== (const Location_t& locationA, const Location_t& locationB);
bool operator!= (const Location_t& locationA, const Location_t& locationB);
bool operator> (const Location_t& locationA, const Location_t& locationB);
bool operator>= (const Location_t& locationA, const Location_t& locationB);
bool operator<= (const Location_t& locationA, const Location_t& locationB);

#endif LOCATION_T_H097623708098732;