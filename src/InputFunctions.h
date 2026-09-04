#pragma once

#include "Types.h"

struct PurpleHistory
{
    std::vector<bool> hasPurple; // true = YOU got a purple (solo+team+CM, log order)
    int cmRaids = 0;
    int cmPurples = 0;           // subset: CM rows in the map that were yours
};

std::string getUsername(const std::string& path);

bool readRaids(const std::string& filename, std::vector<Raid>& raids);