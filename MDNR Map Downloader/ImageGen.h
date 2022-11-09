#pragma once


#include <stdint.h>

#include "IMG_t.h"
#include "Location_t.h"

IMG_t create_screen_image(uint32_t screen_width, uint32_t screen_height, Location_t top_left_point);