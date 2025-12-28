#pragma once

#include "Types.h"

std::string getUsername(const std::string& path);

bool readRaids(const std::string& filename, std::vector<Raid>& raids);