#include "Location_t.h"

#include <cstring>



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