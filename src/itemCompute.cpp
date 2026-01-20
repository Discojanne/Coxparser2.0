#include "itemCompute.h"
#include <iostream>

PurpleSummary computePurpleSummary(int totalRaids, double purpleRate, int actualPurples)
{
    PurpleSummary s{};
    s.totalRaids = totalRaids;
    s.purpleRate = purpleRate;
    s.actualPurples = actualPurples;

    s.expectedPurples = totalRaids / purpleRate;
    s.diff = actualPurples - s.expectedPurples;

    return s;
}

std::vector<PurpleItemStat> computePurpleItemStats(
    int actualPurples,
    double estimatedPurples,
    const std::map<std::string, int>& actualItemCounts)
{
    std::vector<PurpleItemStat> out;

    for (const auto& def : PURPLE_ITEMS) {
        int got = actualItemCounts.count(def.name)
            ? actualItemCounts.at(def.name)
            : 0;

        double expectedActual = actualPurples / def.rate;
        double expectedOnRate = estimatedPurples / def.rate;

        out.push_back({
            def.name,
            got,
            expectedActual,
            got - expectedActual,
            expectedOnRate,
            got - expectedOnRate
            });
    }

    return out;
}

void validatePurpleCounts(
    int expectedTotalPurples,
    const std::map<std::string, int>& itemCounts)
{
    int sum = 0;
    for (const auto& [item, count] : itemCounts)
        sum += count;

    if (sum != expectedTotalPurples)
    {
        std::cerr << "\n[ERROR] Purple count mismatch!\n";
        std::cerr << "  ACTUAL_PURPLES = " << expectedTotalPurples << "\n";
        std::cerr << "  Sum of ACTUAL_ITEM_COUNTS = " << sum << "\n";
        std::cerr << "  Difference = " << (sum - expectedTotalPurples) << "\n\n";

        std::cerr << "Fix ACTUAL_PURPLES or ACTUAL_ITEM_COUNTS before continuing.\n";
        std::exit(EXIT_FAILURE);
    }
}


