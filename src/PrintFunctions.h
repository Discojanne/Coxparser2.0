#pragma once

#include <iostream>
#include <iomanip>
#include <tuple>

#include "Types.h"
#include "PointsLoader.h"

// Width of numeric value printed in value/diff columns (e.g. "77455")
constexpr int VALUE_W = 6;

// Width of signed difference printed next to value (e.g. "+01234")
constexpr int DIFF_W = 7;

// Column widths for the main statistics table
const int NW = 21;  // Room name column
const int TW = 5;   // Best value (time or points)
const int AW = 6;   // Average value (time or points)

// Recent and Last-N columns consist of: <value><space><diff>
const int RW = VALUE_W + 1 + DIFF_W;  // Recent column width
const int LW = VALUE_W + 1 + DIFF_W;  // Last-N column width

const int CW = 18;  // Comparison column (vs secondary user)
const int SEP = 5;  // Spaces between columns


struct Cell
{
    std::string value;   // "17:23" or "77455"
    std::string diff;    // "+0:12" or "-36322"
    const char* color;   // COLOR_GREEN / COLOR_RED / COLOR_RESET
};

inline const char* diffColor(
    int diff,
    bool isTime,
    bool positiveIsGood)
{
    // Neutral for tiny differences
    if (std::abs(diff) < 1)
        return COLOR_RESET;

    // Time-specific grading
    if (isTime)
    {
        // Faster is better for time
        if (!positiveIsGood)
        {
            if (diff <= -20)
                return COLOR_CYAN;
            if (diff < 0)
                return COLOR_GREEN;   // faster
            if (diff < 10)
                return COLOR_ORANGE;  // small slowdown
            return COLOR_RED;         // big slowdown
        }
    }

    // Generic (points etc.)
    bool good = positiveIsGood ? (diff > 0) : (diff < 0);
    return good ? COLOR_GREEN : COLOR_RED;
}


void printRaidStatisticsHeader(const std::string& primaryUser, const std::string& secondaryUser, bool hasSecondary, int totalWidth, int nPastRaids);

void printStatsTable(const std::map<std::string, Stats>& primaryStats, const std::map<std::string, Stats>& secondaryStats, const std::map<std::string,
    int>& recentVal, const std::string& secondaryUser, int totalWidth, bool hasSecondary, PointsToPrint PPH, PointsToPrint Points,
    std::map<std::string, double> lastNAvg);

void printMostCommonPrepRooms(const std::vector<std::pair<std::string, const Stats*>>& common,
    int raids5, int raids6, int raidsOther,
    int totalRaids);

void printDiscardedOutliers(const std::vector<std::tuple<int, std::string, int, std::string>>& discarded,
    const std::string& user,
    const std::string& label);

void printAnalysisSummary(const std::string& primaryUser, int totalRaids, bool hasSecondary, const std::string& secondaryUser, 
    int pastRaids, int secondaryRaidsCount);

void printRoomPPHTable(const std::vector<RoomPPHResult>& rows);

Cell makeCell(int value, double avg, bool isTime, bool positiveIsGood);

bool makeSecondaryCell(const Stats& primary, const Stats& secondary, Cell& out);

void printCell(const Cell& c);

std::string centerText(const std::string& text, int width);
