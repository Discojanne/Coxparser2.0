#include "itemCompute.h"
#include <iostream>

namespace {
constexpr double POINTS_PER_PURPLE = 867600.0;
}

PurpleSummary computePurpleSummary(
    double effectiveKC,
    long long personalPointsKnownEst,
    int raidsWithPointsEst,
    int nUntracked,
    int untrackedAvgPoints,
    int actualPurples,
    const std::map<std::string, int>& actualItemCounts)
{
    PurpleSummary s{};
    s.effectiveKC = effectiveKC;
    s.actualPurples = actualPurples;

    const long long untrackedPoints =
        static_cast<long long>(nUntracked) * untrackedAvgPoints;
    const long long totalPointsEst = personalPointsKnownEst + untrackedPoints;
    const int totalRaidsForAvg = raidsWithPointsEst + nUntracked;

    s.avgPersonalPoints = (totalRaidsForAvg > 0)
        ? static_cast<double>(totalPointsEst) / totalRaidsForAvg
        : 0.0;

    s.expectedPurples =
        static_cast<double>(personalPointsKnownEst) / POINTS_PER_PURPLE
        + static_cast<double>(untrackedPoints) / POINTS_PER_PURPLE;

    // One combined rate over all effective KC (how often we should see a purple).
    s.purpleRate = (s.expectedPurples > 0.0)
        ? (effectiveKC / s.expectedPurples)
        : 0.0;

    s.prayerScrolls = 0;
    for (const auto& [name, count] : actualItemCounts)
    {
        if (isPrayerScroll(name))
            s.prayerScrolls += count;
    }
    s.prayerScrollPct = (actualPurples > 0)
        ? (100.0 * s.prayerScrolls / actualPurples)
        : 0.0;

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

int sumActualPurples(const std::map<std::string, int>& itemCounts)
{
    int sum = 0;
    for (const auto& [item, count] : itemCounts)
        sum += count;
    return sum;
}
