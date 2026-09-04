#include "Config.h"

// ========================== CONFIG =============================
// Keep user-edited settings here so CoxParser.cpp does not recompile.

const int PAST_RAIDS = ALL_RAIDS;   // ALL_RAIDS or a number
const int SESSION_RAIDS = 10;       // "Last N" averages

const std::string PRIMARY_FILE =
    "C:\\Users\\DB96\\.runelite\\cox-analytics\\Disco Turtle_CoxTimes.txt";
const std::string SECONDARY_FILE =
    "C:\\Users\\DB96\\.runelite\\cox-analytics\\KGod_CoxTimes.txt";
const std::string CM_FILE =
    "C:\\Users\\DB96\\.runelite\\cox-analytics\\Disco Turtle_CmTimes.txt";
const std::string SECONDARY_CM_FILE =
    "C:\\Users\\DB96\\.runelite\\cox-analytics\\KGod_CmTimes.txt";
const std::string POINTS_FILE =
    "C:\\Users\\DB96\\.runelite\\raid-data tracker\\cox\\raid_tracker_data.log";

const LayoutFilter LAYOUT_FILTER = LayoutFilter::FullOnly;
const bool TIMES_SOLO_CM = false; // true = solo CM (times/points/PPH); layout filter ignored
bool PRINT_PURPLE_SUMMARY = true;
bool PURPLE_VIEW_POST_ONLY = true; // true = loot section uses only post-split data

// Unique-table weight change cutoffs (set to your KC on update day).
const int RATE_CHANGE_KC = 1832;
const int RATE_CHANGE_CM_KC = 138;

// Manual constants (log gaps — update item counts when you get drops)
const int UNTRACKED_AVG_POINTS = 28000;

const std::map<std::string, int> ACTUAL_ITEM_COUNTS = {
    {"Dexterous prayer scroll", 20},
    {"Arcane prayer scroll",    18},

    {"Twisted buckler",         3},
    {"Dragon hunter crossbow",  3},

    {"Dinh's bulwark",          1},
    {"Ancestral hat",           5},
    {"Ancestral robe top",      1},
    {"Ancestral robe bottom",   4},
    {"Dragon claws",            3},

    {"Elder maul",              5},
    {"Kodai insignia",          0},
    {"Twisted bow",             0}
};

const int DEATH_THRESHOLD_FULL_REGULAR = 48000;
const int DEATH_THRESHOLD_REGULAR = 29000;
const int DEATH_THRESHOLD_CM_SOLO = 59000;
const int DEATH_THRESHOLD_CM_TEAM = 40000;
