#pragma once

#include <iostream>
#include <iomanip>
#include <tuple>

#include "Types.h"

void printRaidStatisticsHeader(const std::string& primaryUser, int totalWidth);

void printStatsTable(const std::map<std::string, Stats>& primaryStats,
    const std::map<std::string, Stats>& secondaryStats,
    const std::map<std::string, int>& recentVal,
    const std::string& secondaryUser,
    const std::vector<std::string>& DISPLAY_ORDER,
    const std::vector<std::string>& PREP_ROOMS,
    int countPad,
    int totalWidth,
    bool hasSecondary);

void printMostCommonPrepRooms(const std::vector<std::pair<std::string, const Stats*>>& common,
    int raids5, int raids6, int raidsOther,
    int totalRaids);

void printDiscardedOutliers(const std::vector<std::tuple<int, std::string, int, std::string>>& discarded,
    const std::string& user,
    const std::string& label);