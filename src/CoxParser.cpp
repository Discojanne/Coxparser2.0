#include <iostream>
#include <iomanip>
#include <cmath>

#include "PrintFunctions.h"
#include "CoxParser.h"
#include "InputFunctions.h"
#include "ComputeFunctions.h"
#include "PointsLoader.h"



// ========================== CONFIG =============================
constexpr int ALL_RAIDS = -1;
constexpr int PAST_RAIDS = ALL_RAIDS;   // ALL_RAIDS or a number
constexpr int SESSION_RAIDS = 10;       // Number of raids to consider for "Last N" averages
const std::string PRIMARY_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\Disco Turtle_CoxTimes.txt";
const std::string SECONDARY_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\KGod_CoxTimes.txt";
//                                  ^ Cox analytics export files
const std::string POINTS_FILE = "C:\\Users\\DB96\\.runelite\\raid-data tracker\\cox\\raid_tracker_data.log";
//                                  ^ Raid data tracker points file, to match points to the primary raids
constexpr LayoutFilter LAYOUT_FILTER = LayoutFilter::All;




void runCoxAnalytics() {
    // ========================== INPUT ==========================
	// Read primary / secondary raid logs from Cox Analytics

	// A raid contains kc, times per room, total time, total points
    std::vector<Raid> primaryRaids, secondaryRaids;

    if (!readRaids(PRIMARY_FILE, primaryRaids)) {
        std::cerr << "Failed to read primary file\n";
        return;
    }
    bool secondaryOk = readRaids(SECONDARY_FILE, secondaryRaids);
    bool hasSecondary = secondaryOk && !secondaryRaids.empty();
    if (hasSecondary)
        keepMostRecentRaids(secondaryRaids, PAST_RAIDS);

    std::string primaryUser = getUsername(PRIMARY_FILE);
    std::string secondaryUser = getUsername(SECONDARY_FILE);

    // ======================= POINTS JOIN =======================
	// Load raid points from Raid Data Tracker and attach to primary raids
    // IMPORTANT: order matters (attach -> filter -> trim)

    auto pointsMap = loadPoints(PRIMARY_FILE, POINTS_FILE);
    attachPointsToRaids(primaryRaids, pointsMap);
    filterRaidsWithPoints(primaryRaids);
    keepMostRecentRaids(primaryRaids, PAST_RAIDS);

    if (primaryRaids.empty())
    {
        std::cout << "No raids to analyze.\n";
        return;
    }

	finalizeDerivedRaidTimes(primaryRaids);
    if (hasSecondary)
        finalizeDerivedRaidTimes(secondaryRaids);

	//Mainly separating full layout vs normal layout raids
    filterByLayout(primaryRaids, LAYOUT_FILTER);
    filterByLayout(secondaryRaids, LAYOUT_FILTER);



    // ====================== AGGREGATION ========================
    // Compute per-room, per-raid, and points-based statistics

    auto agg = computePointsStats(primaryRaids);

    PointsToPrint pointStats = makePointsToPrint(agg.bestPoints, agg.avgPoints,
        primaryRaids.back().totalPoints);
    PointsToPrint pphStats = makePointsToPrint(agg.bestPPH, agg.avgPPH, agg.recentPPH);

    std::map<std::string, Stats> primaryStats, secondaryStats;
	primaryStats = initializeStats();
    if (hasSecondary)
	    secondaryStats = initializeStats();
    aggregateStats(primaryStats, primaryRaids);
    if (hasSecondary)
        aggregateStats(secondaryStats, secondaryRaids);

    auto recentTimes = computeRecentRaidTimes(primaryRaids);


    std::vector<std::pair<std::string, const Stats*>> common = computeMostCommonRooms(primaryStats);
    RoomDistribution rd = computeRoomDistribution(primaryRaids);

    std::vector<std::tuple<int, std::string, int, std::string>> primaryDiscarded, secondaryDiscarded;
	primaryDiscarded = collectAndSortDiscarded(primaryStats);
    if (hasSecondary)
		secondaryDiscarded = collectAndSortDiscarded(secondaryStats);

    auto roomPPH = computeRoomPPH(primaryRaids); // time-weighted PPH per room

    auto lastNAvg = computeLastNStats(primaryRaids, SESSION_RAIDS);

    int totalWidth = computeTotalWidth(hasSecondary); // For table frame



	
    // ======================== OUTPUT ===========================
    // Print tables and summaries

    printAnalysisSummary(primaryUser, static_cast<int>(primaryRaids.size()), hasSecondary, secondaryUser,
        PAST_RAIDS, static_cast<int>(secondaryRaids.size()));

	printRaidStatisticsHeader(primaryUser, secondaryUser, hasSecondary, totalWidth, SESSION_RAIDS);

	printStatsTable(primaryStats, secondaryStats, recentTimes, secondaryUser,
        totalWidth, hasSecondary, pphStats, pointStats, lastNAvg);

    if (LAYOUT_FILTER != LayoutFilter::FullOnly)
    {
    printRoomPPHTable(roomPPH);
	printMostCommonPrepRooms(common, rd.five, rd.six, rd.other, static_cast<int>(primaryRaids.size()));
    }

	printDiscardedOutliers(primaryDiscarded, primaryUser, "Primary");
	if (hasSecondary)
	    printDiscardedOutliers(secondaryDiscarded, secondaryUser, "Secondary");
}