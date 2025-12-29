#include <iostream>
#include <fstream>
#include <set>
#include <iomanip>
#include <cmath>
#include <tuple>
#include <climits>
#include <filesystem>

#include "PrintFunctions.h"
#include "CoxParser.h"
#include "InputFunctions.h"
#include "ComputeFunctions.h"
#include "PointsLoader.h"

// ========================== CONFIG =========================
const int PAST_RAIDS = -1;
const std::string PRIMARY_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\Disco Turtle_CoxTimes.txt";
const std::string SECONDARY_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\Kaudal_CoxTimes.txt";
const std::string POINTS_FILE = "C:\\Users\\DB96\\.runelite\\raid-data tracker\\cox\\raid_tracker_data.log";
// ===========================================================
// 
// =========================== TODO ===========================
// - Calc all points stuff at once.
// - Fix prep room name inconsistencies (Ice demon vs Ice Demon).
// - Design a new table ranking rooms based on pph.
// - Make sure it works with PAST_RAIDS set to any number.
// ===========================================================

void coxParser() {
    std::vector<Raid> primaryRaids, secondaryRaids;

    if (!readRaids(PRIMARY_FILE, primaryRaids)) {
        std::cerr << "Failed to read primary file\n";
        return;
    }
    readRaids(SECONDARY_FILE, secondaryRaids);
	bool hasSecondary = !secondaryRaids.empty();

    std::string primaryUser = getUsername(PRIMARY_FILE);
    std::string secondaryUser = getUsername(SECONDARY_FILE);
    size_t secondaryStart = (PAST_RAIDS == -1) ? 0 : secondaryRaids.size() - std::min<size_t>(PAST_RAIDS, secondaryRaids.size());



	// Load points and process points mapping
    auto pointsMap = loadPoints(PRIMARY_FILE, POINTS_FILE);
	mapPointsToRaids(primaryRaids, pointsMap, true, PAST_RAIDS);
    if (primaryRaids.empty())
    {
        std::cout << "No raids to analyze.\n";
        return;
    }

    PointsToPrint pphStats;
    PointsToPrint pointStats;
    pphStats.average = computeAveragePPH(primaryRaids, pointStats.best);
	pphStats.best = computeBestPPH(primaryRaids);
	computeRecentPPH(primaryRaids, pphStats.recent, pphStats.recentDiff, pphStats.average);

    pointStats.average = computeAveragePoints(primaryRaids);
	pointStats.recent = primaryRaids.back().totalPoints;
	pointStats.recentDiff = pointStats.recent - pointStats.average;


	// Compute all statistics
    std::map<std::string, Stats> primaryStats, secondaryStats;
	primaryStats = initializeStats();
    if (hasSecondary)
	    secondaryStats = initializeStats();
	computeAllStats(primaryStats, primaryRaids);
    if (hasSecondary)
		computeAllStats(secondaryStats, secondaryRaids, secondaryStart);

    std::map<std::string, int> recentVal;
    if (primaryRaids.size() > 1)
	    recentVal = computeRecentRaidTimes(primaryRaids);

    std::vector<std::pair<std::string, const Stats*>> common = computeMostCommonRooms(primaryStats);
    RoomDistribution rd = computeRoomDistribution(primaryRaids);
    int countPadding = computeCountPad(primaryStats);

    std::vector<std::tuple<int, std::string, int, std::string>> primaryDiscarded, secondaryDiscarded;
	primaryDiscarded = collectAndSortDiscarded(primaryStats);
    if (hasSecondary)
		secondaryDiscarded = collectAndSortDiscarded(secondaryStats);

    int totalWidth = computeTotalWidth(hasSecondary); // For table frame


	
	// Print results
    printAnalysisSummary(primaryUser, primaryRaids.size(), hasSecondary, secondaryUser, PAST_RAIDS,
        secondaryRaids.size() - secondaryStart);

	printRaidStatisticsHeader(primaryUser, secondaryUser, hasSecondary, totalWidth);

	printStatsTable(primaryStats, secondaryStats, recentVal, secondaryUser, countPadding,
        totalWidth, hasSecondary, pphStats, pointStats);

	printMostCommonPrepRooms(common, rd.five, rd.six, rd.other, primaryRaids.size());

	printDiscardedOutliers(primaryDiscarded, primaryUser, "Primary");
	if (hasSecondary)
	    printDiscardedOutliers(secondaryDiscarded, secondaryUser, "Secondary");
}