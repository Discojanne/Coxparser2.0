#include <iostream>
#include <iomanip>
#include <cmath>

#include "PrintFunctions.h"
#include "CoxParser.h"
#include "InputFunctions.h"
#include "ComputeFunctions.h"
#include "PointsLoader.h"
#include "itemCompute.h"
#include "itemPrint.h"

// ========================== CONFIG =============================
constexpr int ALL_RAIDS = -1;
constexpr int PAST_RAIDS = ALL_RAIDS;   // ALL_RAIDS or a number
constexpr int SESSION_RAIDS = 10;       // "Last N" averages

const std::string PRIMARY_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\Disco Turtle_CoxTimes.txt";
const std::string SECONDARY_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\KGod_CoxTimes.txt";
const std::string CM_FILE = "C:\\Users\\DB96\\.runelite\\cox-analytics\\Disco Turtle_CmTimes.txt";
const std::string POINTS_FILE = "C:\\Users\\DB96\\.runelite\\raid-data tracker\\cox\\raid_tracker_data.log";

constexpr LayoutFilter LAYOUT_FILTER = LayoutFilter::FullOnly;
bool PRINT_PURPLE_SUMMARY = true;

// Manual constants (log gaps — update item counts when you get drops / revise untracked estimate)
constexpr int UNTRACKED_AVG_POINTS = 28000; // assumed personal pts per untracked regular raid
const std::map<std::string, int> ACTUAL_ITEM_COUNTS = {
    {"Dexterous prayer scroll", 20},
    {"Arcane prayer scroll",    17},

    {"Twisted buckler",         2},
    {"Dragon hunter crossbow",  1},

    {"Dinh's bulwark",          1},
    {"Ancestral hat",           4},
    {"Ancestral robe top",      1},
    {"Ancestral robe bottom",   4},
    {"Dragon claws",            2},

    {"Elder maul",              5},
    {"Kodai insignia",          0},
    {"Twisted bow",             0}
};


void runCoxAnalytics()
{
    const int actualPurples = sumActualPurples(ACTUAL_ITEM_COUNTS);

    // ========================== INPUT ==========================
    std::vector<Raid> primaryRaids, secondaryRaids;

    if (!readRaids(PRIMARY_FILE, primaryRaids))
    {
        std::cerr << "Failed to read primary file\n";
        return;
    }

    bool secondaryOk = readRaids(SECONDARY_FILE, secondaryRaids);
    bool hasSecondary = secondaryOk && !secondaryRaids.empty();
    if (hasSecondary)
        keepMostRecentRaids(secondaryRaids, PAST_RAIDS);

    std::string primaryUser = getUsername(PRIMARY_FILE);
    std::string secondaryUser = getUsername(SECONDARY_FILE);

    const int regularKC = readMaxCoxKC(PRIMARY_FILE);
    const int cmKC = readMaxCmKC(CM_FILE);

    // ======================= POINTS JOIN =======================
    // Order: attach -> filter -> trim
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

    // ====================== ACCOUNT COUNTS =====================
    // Loot / purple math uses the full joined set (layout must not affect this).
    const int nTracked = static_cast<int>(primaryRaids.size());
    auto pointsStats = summarizePointsLog(POINTS_FILE);

    double avgTrackedPoints = 0.0;
    {
        long long sum = 0;
        int n = 0;
        for (const auto& r : primaryRaids)
        {
            if (r.totalPoints > 0)
            {
                sum += r.totalPoints;
                ++n;
            }
        }
        if (n > 0)
            avgTrackedPoints = static_cast<double>(sum) / n;
    }

    const AccountBreakdown breakdown = buildAccountBreakdown(
        regularKC, cmKC, nTracked, avgTrackedPoints, pointsStats);

    // Layout filter: time / PPH / outlier tables only
    filterByLayout(primaryRaids, LAYOUT_FILTER);
    filterByLayout(secondaryRaids, LAYOUT_FILTER);

    if (primaryRaids.empty())
    {
        std::cout << "No raids left after layout filter.\n";
        return;
    }

    // ====================== AGGREGATION ========================
    auto agg = computePointsStats(primaryRaids);

    PointsToPrint pointStats = makePointsToPrint(
        agg.bestPoints, agg.avgPoints, primaryRaids.back().totalPoints);
    PointsToPrint pphStats = makePointsToPrint(
        agg.bestPPH, agg.avgPPH, agg.recentPPH);

    std::map<std::string, Stats> primaryStats = initializeStats();
    std::map<std::string, Stats> secondaryStats;
    if (hasSecondary)
        secondaryStats = initializeStats();

    aggregateStats(primaryStats, primaryRaids);
    if (hasSecondary)
        aggregateStats(secondaryStats, secondaryRaids);

    auto recentTimes = computeRecentRaidTimes(primaryRaids);
    auto common = computeMostCommonRooms(primaryStats);
    RoomDistribution rd = computeRoomDistribution(primaryRaids);

    auto primaryDiscarded = collectAndSortDiscarded(primaryStats);
    std::vector<std::tuple<int, std::string, int, std::string>> secondaryDiscarded;
    if (hasSecondary)
        secondaryDiscarded = collectAndSortDiscarded(secondaryStats);

    auto roomPPH = computeRoomPPH(primaryRaids);
    auto lastNAvg = computeLastNStats(primaryRaids, SESSION_RAIDS);
    int totalWidth = computeTotalWidth(hasSecondary);

    // ===================== PURPLE SUMMARY ========================
    const double effectiveKC = breakdown.regularKC + breakdown.cmEquiv;
    const int raidsWithPointsEst =
        breakdown.nSoloLogged + breakdown.nTeam + breakdown.nCM;

    auto purpleSummary = computePurpleSummary(
        effectiveKC,
        breakdown.sumPersonalKnown,
        raidsWithPointsEst,
        breakdown.nUntracked,
        UNTRACKED_AVG_POINTS,
        actualPurples,
        ACTUAL_ITEM_COUNTS);
    auto itemStats = computePurpleItemStats(
        actualPurples,
        purpleSummary.expectedPurples,
        ACTUAL_ITEM_COUNTS);
    auto purpleHistory = loadPurpleHistory(POINTS_FILE, primaryUser);

    // ======================== OUTPUT ===========================
    printAnalysisSummary(
        primaryUser, static_cast<int>(primaryRaids.size()),
        hasSecondary, secondaryUser,
        PAST_RAIDS, static_cast<int>(secondaryRaids.size()));

    printRaidStatisticsHeader(
        primaryUser, secondaryUser, hasSecondary, totalWidth, SESSION_RAIDS);

    printStatsTable(
        primaryStats, secondaryStats, recentTimes, secondaryUser,
        totalWidth, hasSecondary, pphStats, pointStats, lastNAvg);

    if (LAYOUT_FILTER != LayoutFilter::FullOnly)
    {
        printRoomPPHTable(roomPPH);
        printMostCommonPrepRooms(
            common, rd.five, rd.six, rd.other,
            static_cast<int>(primaryRaids.size()));
    }

    printDiscardedOutliers(primaryDiscarded, primaryUser, "Primary");
    if (hasSecondary)
        printDiscardedOutliers(secondaryDiscarded, secondaryUser, "Secondary");

    if (PRINT_PURPLE_SUMMARY)
    {
        printSectionDivider("LOOT & PURPLE ANALYSIS", totalWidth);
        printAccountBreakdown(breakdown);
        printPurpleSummary(purpleSummary);
        printPurpleItemTable(itemStats);
        printPurpleHistory(
            purpleHistory,
            purpleSummary.purpleRate,
            100,
            breakdown.regularKC,
            actualPurples);
    }
}
