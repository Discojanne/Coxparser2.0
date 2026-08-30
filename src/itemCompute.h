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
    double diff;                 // actual - expected (purples)
    int diffRaids;               // round(diff * purpleRate) — raids worth of that gap
};

struct PurpleItemWeight {
    std::string name;
    int weight;
};

struct PurpleWeightTable {
    int totalWeight;
    std::vector<PurpleItemWeight> items;
};

// Unique table before the weight update (regular + CM shared).
const PurpleWeightTable WEIGHTS_PRE = {
    69,
    {
        {"Dexterous prayer scroll", 20},
        {"Arcane prayer scroll", 20},
        {"Twisted buckler", 4},
        {"Dragon hunter crossbow", 4},
        {"Dinh's bulwark", 3},
        {"Ancestral hat", 3},
        {"Ancestral robe top", 3},
        {"Ancestral robe bottom", 3},
        {"Dragon claws", 3},
        {"Elder maul", 2},
        {"Kodai insignia", 2},
        {"Twisted bow", 2},
    }
};

// Unique table after the weight update — regular CoX.
const PurpleWeightTable WEIGHTS_POST_REGULAR = {
    60,
    {
        {"Dexterous prayer scroll", 14},
        {"Arcane prayer scroll", 14},
        {"Twisted buckler", 4},
        {"Dragon hunter crossbow", 4},
        {"Dinh's bulwark", 3},
        {"Ancestral hat", 4},
        {"Ancestral robe top", 4},
        {"Ancestral robe bottom", 4},
        {"Dragon claws", 3},
        {"Elder maul", 2},
        {"Kodai insignia", 2},
        {"Twisted bow", 2},
    }
};

// Unique table after the weight update — Challenge Mode.
const PurpleWeightTable WEIGHTS_POST_CM = {
    56,
    {
        {"Dexterous prayer scroll", 12},
        {"Arcane prayer scroll", 12},
        {"Twisted buckler", 4},
        {"Dragon hunter crossbow", 4},
        {"Dinh's bulwark", 3},
        {"Ancestral hat", 4},
        {"Ancestral robe top", 4},
        {"Ancestral robe bottom", 4},
        {"Dragon claws", 3},
        {"Elder maul", 2},
        {"Kodai insignia", 2},
        {"Twisted bow", 2},
    }
};

// Display order for the purple items table (same as WEIGHTS_PRE order).
const std::vector<std::string> PURPLE_ITEM_NAMES = {
    "Dexterous prayer scroll",
    "Arcane prayer scroll",
    "Twisted buckler",
    "Dragon hunter crossbow",
    "Dinh's bulwark",
    "Ancestral hat",
    "Ancestral robe top",
    "Ancestral robe bottom",
    "Dragon claws",
    "Elder maul",
    "Kodai insignia",
    "Twisted bow",
};

struct PurpleItemStat {
    std::string name;

    int got;

    double expectedActual;   // blended from actual purples per era/mode
    double diffActual;       // got - expectedActual

    double expectedOnRate;   // blended from estimated purples per era/mode
    double diffOnRate;       // got - expectedOnRate
};

// Inputs for era-aware item expectations.
struct PurpleItemExpectationInput {
    int actualPurplesPre = 0;
    int actualPurplesPostRegular = 0;
    int actualPurplesPostCm = 0;
    long long pointsPre = 0;
    long long pointsPostRegular = 0;
    long long pointsPostCm = 0;
    std::map<std::string, int> got;
};

inline bool isPrayerScroll(const std::string& item)
{
    return item == "Dexterous prayer scroll"
        || item == "Arcane prayer scroll";
}

inline double itemRate(const PurpleWeightTable& table, int weight)
{
    return static_cast<double>(table.totalWeight) / static_cast<double>(weight);
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
    const PurpleItemExpectationInput& in);

// Sum of ACTUAL_ITEM_COUNTS — single source of truth for total purples.
int sumActualPurples(const std::map<std::string, int>& itemCounts);

constexpr double POINTS_PER_PURPLE = 867600.0;
