#ifndef LOCATION_T_H097623708098732
#define LOCATION_T_H097623708098732

#include <stdint.h> //uint16_t, uint8_t
#include <functional> //std::size_t


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

	static Location_t fromGPSCoords(double longitude, double latitude, uint8_t layer);
	Location_t(uint16_t x, uint16_t y, uint8_t layer) :x(x), y(y), layer(layer) {}
	Location_t() :x(0), y(0), layer(0) {}

	void translateLayer(uint8_t newLayer);

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