#pragma once

#include <string>
#include <vector>
#include <map>

#include "Types.h"

struct RoomDistribution {
    int five = 0;
    int six = 0;
    int other = 0;
};

std::string secondsToTime(int seconds);

void processStats(std::map<std::string, Stats>& stats, const std::string& key, const std::vector<Raid>& raids, size_t start);

std::map<std::string, Stats> initializeStats();

void computeAllStats(std::map<std::string, Stats>& stats, const std::vector<Raid>& raids, size_t start);

std::map<std::string, int> computeRecentRaidTimes(const std::vector<Raid>& raids);


RoomDistribution computeRoomDistribution(const std::vector<Raid>& raids, size_t start);

int computeCountPad(const std::map<std::string, Stats>& stats);

std::vector<std::tuple<int, std::string, int, std::string>> collectAndSortDiscarded(
    const std::map<std::string, Stats>& stats);

std::vector<std::pair<std::string, const Stats*>> computeMostCommonRooms(const std::map<std::string, Stats>& stats);

int computeTotalWidth(bool hasSecondary);
