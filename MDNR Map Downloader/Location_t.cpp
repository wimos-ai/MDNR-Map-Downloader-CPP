#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "Location_t.h"

#include <cstring>



//// Location_t functions /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool operator< (const Location_t& locationA, const Location_t& locationB) {
	uint8_t a_bytes[8];

	uint8_t b_bytes[8];

	std::memset(a_bytes, 0, sizeof(a_bytes)); // Zero a_bytes

	std::memset(b_bytes, 0, sizeof(b_bytes)); // Zero b_bytes

	((uint16_t*)a_bytes)[0] = locationA.x;

	((uint16_t*)b_bytes)[0] = locationB.x;

	((uint16_t*)a_bytes)[1] = locationA.y;

	((uint16_t*)b_bytes)[1] = locationB.y;

	((uint8_t*)a_bytes)[5] = locationA.layer;

	((uint8_t*)b_bytes)[5] = locationB.layer;

	return (*((uint64_t*)a_bytes) < *((uint64_t*)b_bytes));
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