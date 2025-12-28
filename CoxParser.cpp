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

// ========================== CONFIG =========================
const int PAST_RAIDS = -1;
const std::string PRIMARY_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\Disco Turtle_CoxTimes.txt";
const std::string SECONDARY_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\Kaudal_CoxTimes.txt";
// ===========================================================

void coxParser() {
    std::vector<Raid> primaryRaids, secondaryRaids;

    if (!readRaids(PRIMARY_FILE, primaryRaids)) {
        std::cerr << "Failed to read primary file\n";
        return;
    }
    readRaids(SECONDARY_FILE, secondaryRaids);
	bool hasSecondary = !secondaryRaids.empty();

    size_t primaryStart = (PAST_RAIDS == -1) ? 0 : primaryRaids.size() - std::min<size_t>(PAST_RAIDS, primaryRaids.size());
    size_t secondaryStart = (PAST_RAIDS == -1) ? 0 : secondaryRaids.size() - std::min<size_t>(PAST_RAIDS, secondaryRaids.size());

    std::string primaryUser = getUsername(PRIMARY_FILE);
    std::string secondaryUser = getUsername(SECONDARY_FILE);

    int totalRaids = primaryRaids.size() - primaryStart;
    if (totalRaids == 0)
    {
        std::cout << "No raids to analyze.\n";
        return;
    }



	// Compute all statistics
    std::map<std::string, Stats> primaryStats, secondaryStats;
	primaryStats = initializeStats();
    if (hasSecondary)
	    secondaryStats = initializeStats();
	computeAllStats(primaryStats, primaryRaids, primaryStart);
    if (hasSecondary)
		computeAllStats(secondaryStats, secondaryRaids, secondaryStart);

    std::map<std::string, int> recentVal;
    if (primaryRaids.size() > primaryStart)
	    recentVal = computeRecentRaidTimes(primaryRaids);

    std::vector<std::pair<std::string, const Stats*>> common = computeMostCommonRooms(primaryStats);
    RoomDistribution rd = computeRoomDistribution(primaryRaids, primaryStart);
    int countPadding = computeCountPad(primaryStats);

    std::vector<std::tuple<int, std::string, int, std::string>> primaryDiscarded, secondaryDiscarded;
	primaryDiscarded = collectAndSortDiscarded(primaryStats);
    if (hasSecondary)
		secondaryDiscarded = collectAndSortDiscarded(secondaryStats);

    int totalWidth = computeTotalWidth(hasSecondary);


	
	// Print results
    printAnalysisSummary(primaryUser, totalRaids, hasSecondary, secondaryUser, PAST_RAIDS,
        secondaryRaids.size() - secondaryStart);

	printRaidStatisticsHeader(primaryUser, secondaryUser, hasSecondary, totalWidth);

	printStatsTable(primaryStats, secondaryStats, recentVal, secondaryUser, countPadding, totalWidth, hasSecondary);

	printMostCommonPrepRooms(common, rd.five, rd.six, rd.other, totalRaids);

	printDiscardedOutliers(primaryDiscarded, primaryUser, "Primary");
	if (hasSecondary)
	    printDiscardedOutliers(secondaryDiscarded, secondaryUser, "Secondary");
}