#pragma once

#include <string>
#include <map>
#include <vector>

#define COLOR_GREEN "\033[32m"
#define COLOR_RED   "\033[31m"
#define COLOR_RESET "\033[0m"

// Represents a single Chambers of Xeric raid run
// Times are stored per room in seconds
// Derived values (totalSeconds, Pre-Olm) are filled later
// Points are attached later from a separate source
struct Raid {
    int kc;                              // Kill count at time of raid
    std::map<std::string, int> times;    // Room name, seconds
    int totalSeconds = 0;                // Total raid duration (derived)
    int totalPoints = -1;                // Points earned in this raid
};


// Aggregated statistics for a single room or phase across many raids
struct Stats {
    struct Entry {
        int kc;
        int time;
    };

    std::vector<Entry> entries;   // All valid samples
    std::vector<std::tuple<int, std::string, int, std::string>> discarded;
    // ^ Outliers removed from analysis

    double avg = 0.0;             // Average value across valid samples
    int fastest = 0;              // Best (minimum) observed value
    int validCount = 0;           // Number of valid samples
};


const std::vector<std::string> PREP_ROOMS = {
    "Tekton", "Crabs", "Ice demon", "Shamans", "Vanguards", "Thieving",
    "Vespula", "Tightrope", "Guardians", "Vasa", "Mystics", "Muttadiles"
};

inline bool isPrepRoom(const std::string& room)
{
    return std::find(PREP_ROOMS.begin(), PREP_ROOMS.end(), room) != PREP_ROOMS.end();
}

const std::map<std::string, unsigned int> ROOM_REFERENCE_FOR_OUTLIERS = {
    {"Tekton", 30},
    {"Crabs", 45},
    {"Ice demon", 90},
    {"Shamans", 27},
    {"Vanguards", 60},
    {"Thieving", 45},
    {"Vespula", 15},
    {"Tightrope", 25},
    {"Guardians", 35},
    {"Vasa", 30},
    {"Mystics", 30},
    {"Muttadiles", 45},
    {"Between room time", 20}
};

const std::map<std::string, unsigned int> ROOM_MAX_REFERENCE = {
    {"Tekton",              240},
    {"Crabs",               240},
    {"Ice demon",           240},
    {"Shamans",             240},
    {"Vanguards",           240},
    {"Thieving",            240},
    {"Vespula",             240},
    {"Tightrope",           240},
    {"Guardians",           240},
    {"Vasa",                240},
    {"Mystics",             240},
    {"Muttadiles",          240},
    {"Between room time",   600}
};

const std::vector<std::string> DISPLAY_ORDER = {
    "Tekton", "Crabs", "Ice demon", "Shamans", "Vanguards", "Thieving",
    "Vespula", "Tightrope", "Guardians", "Vasa", "Mystics", "Muttadiles",
    "Pre-Olm",
    "Olm mage hand phase 1", "Olm phase 1", "Olm mage hand phase 2",
    "Olm phase 2", "Olm phase 3", "Olm head",
    "Olm",
    "Raid Completed",
    "Between room time",
	"Total Points",
    "PPH"
};

// Values prepared for table output (points or PPH)
struct PointsToPrint {
    int best;        // Best observed value
    int average;     // Average across all raids
    int recent;      // Most recent raid value
    int recentDiff;  // recent - average (signed)
};


// 2 structures for estimating pph per room
struct RoomPPHStats
{
    int raids = 0;
    double totalPoints = 0.0;
    double totalSeconds = 0.0;
};


struct RoomPPHResult
{
    std::string room;
    int avgPPH;
    int raids;
};