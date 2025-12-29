#pragma once

#include <iostream>
#include <iomanip>
#include <tuple>

#include "Types.h"
#include "PointsLoader.h"

const int NW = 21;  // 
const int TW = 5;   // Best
const int AW = 7;   // Avg
const int RW = 12;   // Recent
const int CW = 10;  // Compare
const int SEP = 4;  // spaces between columns

void printRaidStatisticsHeader(const std::string& primaryUser, const std::string& secondaryUser, bool hasSecondary, int totalWidth);

void printStatsTable(const std::map<std::string, Stats>& primaryStats, const std::map<std::string, Stats>& secondaryStats, const std::map<std::string,
    int>& recentVal, const std::string& secondaryUser, int countPad, int totalWidth, bool hasSecondary, PointsToPrint PPH, PointsToPrint Points);

void printMostCommonPrepRooms(const std::vector<std::pair<std::string, const Stats*>>& common,
    int raids5, int raids6, int raidsOther,
    int totalRaids);

void printDiscardedOutliers(const std::vector<std::tuple<int, std::string, int, std::string>>& discarded,
    const std::string& user,
    const std::string& label);

void printAnalysisSummary(const std::string& primaryUser, int totalRaids, bool hasSecondary, const std::string& secondaryUser, 
    int pastRaids, int secondaryRaidsCount);

inline bool isPointsRow(const std::string& key)
{
    return key == "Total points" || key == "PPH";
}

inline std::string emptyVS()
{
    return "";
}

std::string padRightAligned(const std::string& s, int width);
