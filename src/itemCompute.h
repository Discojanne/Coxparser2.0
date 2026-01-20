#pragma once
#include <string>
#include <vector>
#include <map>

struct PurpleSummary
{
    int totalRaids;
    double purpleRate;     // e.g. 26.7548
    int actualPurples;

    double expectedPurples;
    double diff;           // actual - expected
};

struct PurpleItemDef {
    std::string name;
    double rate;   // e.g. 3.45 means 1 / 3.45
};

const std::vector<PurpleItemDef> PURPLE_ITEMS = {
    {"Dexterous prayer scroll", 3.45},
    {"Arcane prayer scroll", 3.45},
    {"Twisted buckler", 17.25},
    {"Dragon hunter crossbow", 17.25},
    {"Dinh's bulwark", 23.0},
    {"Ancestral hat", 23.0},
    {"Ancestral robe top", 23.0},
    {"Ancestral robe bottom", 23.0},
    {"Dragon claws", 23.0},
    {"Elder maul", 34.5},
    {"Kodai insignia", 34.5},
    {"Twisted bow", 34.5}
};

struct PurpleItemStat {
    std::string name;

    int got;

    double expectedActual;   // based on ACTUAL_PURPLES
    double diffActual;       // got - expectedActual

    double expectedOnRate;   // based on ESTIMATED_PURPLES
    double diffOnRate;       // got - expectedOnRate
};




inline bool isPrayerScroll(const std::string& item)
{
    return item == "Dexterous prayer scroll"
        || item == "Arcane prayer scroll";
}

PurpleSummary computePurpleSummary(int totalRaids, double purpleRate, int actualPurples);

std::vector<PurpleItemStat> computePurpleItemStats(
    int actualPurples,
    double estimatedPurples,
    const std::map<std::string, int>& actualItemCounts);

void validatePurpleCounts(
    int expectedTotalPurples,
    const std::map<std::string, int>& itemCounts);