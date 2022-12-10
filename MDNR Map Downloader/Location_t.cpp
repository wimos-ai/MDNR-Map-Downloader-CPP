#include "Location_t.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif

#include <cstring>

std::size_t Location_t::hash_fn::operator() (const Location_t& location) const
{
	std::size_t h1 = std::hash<decltype(location.x)>()(location.x);
	std::size_t h2 = std::hash<decltype(location.y)>()(location.y);
	std::size_t h3 = std::hash<decltype(location.layer)>()(location.layer);

	return (h1 ^ h2) ^ h3;
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
