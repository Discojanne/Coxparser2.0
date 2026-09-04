#pragma once

#include "Types.h"

struct RoomDistribution {
    int five = 0;
    int six = 0;
    int other = 0;
};

// Used when computing points statistics
struct PointsAggregate
{
    int bestPPH = 0;
    int avgPPH = 0;
    int recentPPH = 0;
    int bestPoints = 0;
    int avgPoints = 0;
};

std::string secondsToTime(int seconds);

std::map<std::string, Stats> initializeStats();

void aggregateStats(std::map<std::string, Stats>& stats, const std::vector<Raid>& raids, size_t start = 0, bool applyRoomOutlierRefs = true);

std::map<std::string, int> computeRecentRaidTimes(const std::vector<Raid>& raids);

RoomDistribution computeRoomDistribution(const std::vector<Raid>& raids);

std::vector<std::tuple<int, std::string, int, std::string>> collectAndSortDiscarded(
    const std::map<std::string, Stats>& stats);

std::vector<std::pair<std::string, const Stats*>> computeMostCommonRooms(const std::map<std::string, Stats>& stats);

int computeTotalWidth(bool hasSecondary);

// Attach points, drop unmatched, then optional last-N trim. Does not derive times.
void attachAndKeepJoinedRaids(
    std::vector<Raid>& raids,
    const std::map<int, int>& pointsMap,
    int maxCount);

void keepMostRecentRaids(std::vector<Raid>& raids, int maxCount);


PointsAggregate computePointsStats(const std::vector<Raid>& raids);

PointsToPrint makePointsToPrint(int best, int average, int recent);

std::vector<RoomPPHResult>computeRoomPPH(const std::vector<Raid>& raids);

void finalizeDerivedRaidTimes(std::vector<Raid>& raids);

std::map<std::string, double> computeLastNStats(const std::vector<Raid>& raids, int lastN);

void filterByLayout(std::vector<Raid>& raids, LayoutFilter mode);
