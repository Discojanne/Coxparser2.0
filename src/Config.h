#pragma once

#include <map>
#include <string>

#include "Types.h"

// Rarely changed — safe to keep in the header.
constexpr int ALL_RAIDS = -1;

// Edit Config.cpp when updating paths, loot counts, or toggles.

extern const int PAST_RAIDS;
extern const int SESSION_RAIDS;

extern const std::string PRIMARY_FILE;
extern const std::string SECONDARY_FILE;
extern const std::string CM_FILE;
extern const std::string POINTS_FILE;

extern const LayoutFilter LAYOUT_FILTER;
extern bool PRINT_PURPLE_SUMMARY;
extern bool PURPLE_VIEW_POST_ONLY;

extern const int RATE_CHANGE_KC;
extern const int RATE_CHANGE_CM_KC;

extern const int UNTRACKED_AVG_POINTS;
extern const std::map<std::string, int> ACTUAL_ITEM_COUNTS;

// Personal-point thresholds below which a raid is treated as having died.
extern const int DEATH_THRESHOLD_FULL_REGULAR;
extern const int DEATH_THRESHOLD_REGULAR;
extern const int DEATH_THRESHOLD_CM_SOLO;
extern const int DEATH_THRESHOLD_CM_TEAM;
