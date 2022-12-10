#include "Location_t.h"

#include <functional> //std::size_t,  std::hash
#include <stdexcept> //std::invalid_argument


std::size_t Location_t::hash_fn::operator() (const Location_t& location) const
{
	std::size_t h1 = std::hash<decltype(location.x)>()(location.x);
	std::size_t h2 = std::hash<decltype(location.y)>()(location.y);
	std::size_t h3 = std::hash<decltype(location.layer)>()(location.layer);

	return (h1 ^ h2) ^ h3;
}

void Location_t::translateLayer(uint8_t newLayer) {
	if (newLayer < layer)
	{
		this->x /= (2 * layer - newLayer);
		this->y /= (2 * layer - newLayer);
		layer = newLayer;
	}
	else if (newLayer > layer) {
		this->x *= (2 * newLayer - layer);
		this->y *= (2 * newLayer - layer);
		layer = newLayer;
	}
	else {
		return;
	}
}

Location_t Location_t::fromGPSCoords(double longitude, double latitude, uint8_t layer){

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


//// Location_t functions /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool operator< (const Location_t& locationA, const Location_t& locationB) {
	if (locationA.x < locationB.x)
	{
		return true;
	}
	else if (locationA.x > locationB.x) {
		return false;
	}
	else { // locationA.x == locationB.x
		if (locationA.y < locationB.y)
		{
			return true;
		}
		else if (locationA.y > locationB.y) {
			return false;
		}
		else { // locationA.y == locationB.y
			if (locationA.layer < locationB.layer)
			{
				return true;
			}
			else if (locationA.layer > locationB.layer) {
				return false;
			}
			return false;
		}
	}
}

bool operator== (const Location_t& locationA, const Location_t& locationB) {
	return (locationA.y == locationB.y) && (locationA.x == locationB.y) && (locationA.layer == locationB.layer);
}

bool operator!=(const Location_t& locationA, const Location_t& locationB)
{
	return !(locationA == locationB);
}

bool operator>(const Location_t& locationA, const Location_t& locationB)
{
	return locationB < locationA;
}

bool operator>=(const Location_t& locationA, const Location_t& locationB)
{
	return !(locationA < locationB);
}

bool operator<=(const Location_t& locationA, const Location_t& locationB)
{
	return (locationA < locationB) || (locationA == locationB);
}
