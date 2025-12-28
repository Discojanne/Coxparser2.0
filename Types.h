#pragma once

#include <string>
#include <map>
#include <vector>

#define COLOR_GREEN "\033[32m"
#define COLOR_RED   "\033[31m"
#define COLOR_RESET "\033[0m"

struct Raid {
    int kc;
    std::map<std::string, int> times;
};

struct Stats {
    struct Entry { int kc; int value; };
    std::vector<Entry> entries;
    std::vector<std::tuple<int, std::string, int, std::string>> discarded;

    double avg = 0.0;
    int    fastest = 0;
    int    validCount = 0;
};

const std::vector<std::string> PREP_ROOMS = {
    "Tekton", "Crabs", "Ice demon", "Shamans", "Vanguards", "Thieving",
    "Vespula", "Tightrope", "Guardians", "Vasa", "Mystics", "Muttadiles"
};

const std::map<std::string, unsigned int> ROOM_REFERENCE_FOR_OUTLIERS = {
    {"Tekton", 30},
    {"Crabs", 45},
    {"Ice Demon", 90},
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

const std::vector<std::string> DISPLAY_ORDER = {
    "Tekton", "Crabs", "Ice demon", "Shamans", "Vanguards", "Thieving",
    "Vespula", "Tightrope", "Guardians", "Vasa", "Mystics", "Muttadiles",
    "Pre-Olm",
    "Olm mage hand phase 1", "Olm phase 1", "Olm mage hand phase 2",
    "Olm phase 2", "Olm phase 3", "Olm head",
    "Olm",
    "Raid Completed",
    "Between room time"
};