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

struct RecentCell
{
    std::string value;
    std::string diff;
    const char* color;
};

std::string secondsToTime(int seconds);

void processStats(std::map<std::string, Stats>& stats, const std::string& key, const std::vector<Raid>& raids, size_t start);

std::map<std::string, Stats> initializeStats();

void aggregateStats(std::map<std::string, Stats>& stats, const std::vector<Raid>& raids, size_t start = 0);

std::map<std::string, int> computeRecentRaidTimes(const std::vector<Raid>& raids);


RoomDistribution computeRoomDistribution(const std::vector<Raid>& raids);

int computeCountPad(const std::map<std::string, Stats>& stats);

std::vector<std::tuple<int, std::string, int, std::string>> collectAndSortDiscarded(
    const std::map<std::string, Stats>& stats);

std::vector<std::pair<std::string, const Stats*>> computeMostCommonRooms(const std::map<std::string, Stats>& stats);

int computeTotalWidth(bool hasSecondary);

void attachPointsToRaids(std::vector<Raid>& raids, const std::map<int, int>& pointsMap);

void filterRaidsWithPoints(std::vector<Raid>& raids);

void keepMostRecentRaids(std::vector<Raid>& raids, int maxCount);






PointsAggregate computePointsStats(const std::vector<Raid>& raids);

PointsToPrint makePointsToPrint(int best, int average, int recent);

std::vector<RoomPPHResult>computeRoomPPH(const std::vector<Raid>& raids);

double computeLastNTimeAvg(const std::vector<Raid>& raids, const std::string& key, int N);

double computeLastNPPH(const std::vector<Raid>& raids, int N);

double computeLastNPoints(const std::vector<Raid>& raids, int N);

void finalizeDerivedRaidTimes(std::vector<Raid>& raids);

std::map<std::string, double> computeLastNStats(const std::vector<Raid>& raids, int lastN);