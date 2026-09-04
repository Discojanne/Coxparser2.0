#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

#include "PrintFunctions.h"
#include "CoxParser.h"
#include "Config.h"
#include "InputFunctions.h"
#include "ComputeFunctions.h"
#include "PointsLoader.h"
#include "itemCompute.h"
#include "itemPrint.h"


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
    const DeathStats deathStats = summarizeDeathStats(
        POINTS_FILE,
        DEATH_THRESHOLD_FULL_REGULAR,
        DEATH_THRESHOLD_REGULAR,
        DEATH_THRESHOLD_CM_SOLO,
        DEATH_THRESHOLD_CM_TEAM);

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
    const auto era = loadPurpleEraAnalysis(
        POINTS_FILE,
        primaryUser,
        RATE_CHANGE_KC,
        RATE_CHANGE_CM_KC,
        regularKC,
        cmKC);

    const int lifetimePurples = actualPurples;
    const int postPurples =
        era.purplesPostRegular + era.purplesPostCm;
    const int prePurples = std::max(0, lifetimePurples - postPurples);

    const long long untrackedPoints =
        static_cast<long long>(breakdown.nUntracked) * UNTRACKED_AVG_POINTS;
    const long long totalPointsEst =
        breakdown.sumPersonalKnown + untrackedPoints;
    const long long postPoints =
        era.pointsPostRegular + era.pointsPostCm;
    const long long prePoints = std::max(0LL, totalPointsEst - postPoints);

    PurpleSummary purpleSummary{};
    PurpleItemExpectationInput itemIn{};
    const PurpleHistory* purpleHistory = &era.historyAll;
    int historyActualPurples = lifetimePurples;

    if (PURPLE_VIEW_POST_ONLY)
    {
        const int postRegularKc = std::max(0, regularKC - RATE_CHANGE_KC);
        double postCmEquiv = 0.0;
        if (avgTrackedPoints > 0.0 && era.pointsPostCm > 0)
            postCmEquiv = static_cast<double>(era.pointsPostCm) / avgTrackedPoints;
        const double postEffectiveKc = postRegularKc + postCmEquiv;
        const int postRaidsEst = era.nPostRegular + era.nPostCm;

        purpleSummary = computePurpleSummary(
            postEffectiveKc,
            postPoints,
            postRaidsEst,
            /*nUntracked*/ 0,
            /*untrackedAvgPoints*/ 0,
            postPurples,
            era.itemsPost);

        itemIn.actualPurplesPre = 0;
        itemIn.actualPurplesPostRegular = era.purplesPostRegular;
        itemIn.actualPurplesPostCm = era.purplesPostCm;
        itemIn.pointsPre = 0;
        itemIn.pointsPostRegular = era.pointsPostRegular;
        itemIn.pointsPostCm = era.pointsPostCm;
        itemIn.got = era.itemsPost;

        purpleHistory = &era.historyPost;
        historyActualPurples = postPurples;
    }
    else
    {
        const double effectiveKC = breakdown.regularKC + breakdown.cmEquiv;
        const int raidsWithPointsEst =
            breakdown.nSoloLogged + breakdown.nTeam + breakdown.nCM;

        purpleSummary = computePurpleSummary(
            effectiveKC,
            breakdown.sumPersonalKnown,
            raidsWithPointsEst,
            breakdown.nUntracked,
            UNTRACKED_AVG_POINTS,
            lifetimePurples,
            ACTUAL_ITEM_COUNTS);

        itemIn.actualPurplesPre = prePurples;
        itemIn.actualPurplesPostRegular = era.purplesPostRegular;
        itemIn.actualPurplesPostCm = era.purplesPostCm;
        itemIn.pointsPre = prePoints;
        itemIn.pointsPostRegular = era.pointsPostRegular;
        itemIn.pointsPostCm = era.pointsPostCm;
        itemIn.got = ACTUAL_ITEM_COUNTS;
    }

    auto itemStats = computePurpleItemStats(itemIn);

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
        const char* lootTitle = PURPLE_VIEW_POST_ONLY
            ? "LOOT & PURPLE ANALYSIS (post-split)"
            : "LOOT & PURPLE ANALYSIS";
        printSectionDivider(lootTitle, totalWidth);
        printDeathStats(deathStats);
        if (!PURPLE_VIEW_POST_ONLY)
            printAccountBreakdown(breakdown);
        printPurpleSummary(purpleSummary);
        printPurpleItemTable(itemStats);
        printPurpleHistory(
            *purpleHistory,
            purpleSummary.purpleRate,
            100,
            breakdown.regularKC,
            historyActualPurples);
    }
}
