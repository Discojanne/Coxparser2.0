#pragma once
#include <string>
#include <vector>
#include <map>

struct PurpleSummary
{
    double effectiveKC;          // regular KC + CM equiv
    double avgPersonalPoints;    // all raids: known/est pts + untracked @ assumed pts
    double purpleRate;           // combined 1-in-X: effectiveKC / expected
    int actualPurples;
    int prayerScrolls;           // dex + arcane
    double prayerScrollPct;      // prayerScrolls / actualPurples * 100

    double expectedPurples;
    double diff;                 // actual - expected
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

PurpleSummary computePurpleSummary(
    double effectiveKC,
    long long personalPointsKnownEst,
    int raidsWithPointsEst,
    int nUntracked,
    int untrackedAvgPoints,
    int actualPurples,
    const std::map<std::string, int>& actualItemCounts);

std::vector<PurpleItemStat> computePurpleItemStats(
    int actualPurples,
    double estimatedPurples,
    const std::map<std::string, int>& actualItemCounts);

// Sum of ACTUAL_ITEM_COUNTS — single source of truth for total purples.
int sumActualPurples(const std::map<std::string, int>& itemCounts);