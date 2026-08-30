#include "itemCompute.h"
#include <cmath>

namespace {
int weightFor(const PurpleWeightTable& table, const std::string& name)
{
    for (const auto& item : table.items)
    {
        if (item.name == name)
            return item.weight;
    }
    return 0;
}

double expectedFromBucket(
    double purples,
    const PurpleWeightTable& table,
    const std::string& name)
{
    const int w = weightFor(table, name);
    if (w <= 0 || purples <= 0.0)
        return 0.0;
    return purples / itemRate(table, w);
}
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
    s.diffRaids = (s.expectedPurples > 0.0)
        ? static_cast<int>(std::round(s.diff * s.purpleRate))
        : 0;
    return s;
}

std::vector<PurpleItemStat> computePurpleItemStats(
    const PurpleItemExpectationInput& in)
{
    std::vector<PurpleItemStat> out;

    const double expPurplesPre =
        static_cast<double>(in.pointsPre) / POINTS_PER_PURPLE;
    const double expPurplesPostReg =
        static_cast<double>(in.pointsPostRegular) / POINTS_PER_PURPLE;
    const double expPurplesPostCm =
        static_cast<double>(in.pointsPostCm) / POINTS_PER_PURPLE;

    for (const auto& name : PURPLE_ITEM_NAMES)
    {
        const int got = in.got.count(name) ? in.got.at(name) : 0;

        const double expectedActual =
            expectedFromBucket(in.actualPurplesPre, WEIGHTS_PRE, name)
            + expectedFromBucket(in.actualPurplesPostRegular, WEIGHTS_POST_REGULAR, name)
            + expectedFromBucket(in.actualPurplesPostCm, WEIGHTS_POST_CM, name);

        const double expectedOnRate =
            expectedFromBucket(expPurplesPre, WEIGHTS_PRE, name)
            + expectedFromBucket(expPurplesPostReg, WEIGHTS_POST_REGULAR, name)
            + expectedFromBucket(expPurplesPostCm, WEIGHTS_POST_CM, name);

        out.push_back({
            name,
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
