#pragma once

#include <iostream>
#include <iomanip>
#include <tuple>

#include "Types.h"

const int NW = 26;
const int TW = 10;
const int AW = 24;
const int RW = 12;  // "XX:XX  +XX:XX" = 12 chars
const int SEP = 3;

void printRaidStatisticsHeader(const std::string& primaryUser, const std::string& secondaryUser, bool hasSecondary, int totalWidth);

void printStatsTable(const std::map<std::string, Stats>& primaryStats, const std::map<std::string, Stats>& secondaryStats, const std::map<std::string,
    int>& recentVal, const std::string& secondaryUser, int countPad, int totalWidth, bool hasSecondary);

void printMostCommonPrepRooms(const std::vector<std::pair<std::string, const Stats*>>& common,
    int raids5, int raids6, int raidsOther,
    int totalRaids);

void printDiscardedOutliers(const std::vector<std::tuple<int, std::string, int, std::string>>& discarded,
    const std::string& user,
    const std::string& label);

void printAnalysisSummary(const std::string& primaryUser, int totalRaids, bool hasSecondary, const std::string& secondaryUser, 
    int pastRaids, int secondaryRaidsCount);
