#pragma once

#include <string>
#include <vector>
#include <map>

#include "Types.h"

std::string secondsToTime(int seconds);

void processStats(std::map<std::string, Stats>& stats, const std::string& key, const std::vector<Raid>& raids, size_t start);
