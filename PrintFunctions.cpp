#include <algorithm>
#include <sstream>

#include "PrintFunctions.h"
#include "ComputeFunctions.h"



void printRaidStatisticsHeader(const std::string& primaryUser, const std::string& secondaryUser, bool hasSecondary, int totalWidth)
{
    std::cout << "Raid Statistics - Primary: " << primaryUser << "\n";
    std::cout << std::string(totalWidth, '=') << "\n";

    std::cout << std::left << std::setw(NW) << "Room" << std::string(SEP, ' ')
        << std::right << std::setw((TW)) << "Best" << std::string(SEP, ' ')
        << std::right << std::setw(AW) << "Average" << std::string(SEP, ' ')
        << std::right << std::setw(RW) << "Recent";
    if (hasSecondary)
        std::cout << std::string(SEP, ' ') << std::right << std::setw(CW) << ("vs " + secondaryUser);
    std::cout << "\n";

    std::cout << std::string(totalWidth, '-') << "\n";
}

void printStatsTable(const std::map<std::string, Stats>& primaryStats, const std::map<std::string, Stats>& secondaryStats, const std::map<std::string,
    int>& recentVal, const std::string& secondaryUser, int countPad, int totalWidth, bool hasSecondary, PointsToPrint PPH, PointsToPrint Points)
{
    for (const auto& key : DISPLAY_ORDER) {
        auto it = primaryStats.find(key);
        if (it == primaryStats.end()) continue;
        const auto& ps = it->second;

        bool isTotalPoints = (key == "Total Points");
        bool isPPH = (key == "PPH");
        bool isPointsRow = isTotalPoints || isPPH;


        bool isPrepRoom = std::find(PREP_ROOMS.begin(), PREP_ROOMS.end(), key) != PREP_ROOMS.end();
        if (ps.validCount == 0 && isPrepRoom) continue;

        std::string bestStr = "--:--";

        if (isTotalPoints && Points.best > 0) {
            bestStr = std::to_string(Points.best);
        }
        else if (isPPH && PPH.best > 0) {
            bestStr = std::to_string(PPH.best);
        }
        else if (!isPointsRow) {
            bestStr = (ps.fastest <= 0) ? "--:--" : secondsToTime(ps.fastest);
        }



        std::string avgStr = "--:--";

        if (isTotalPoints && Points.average > 0) {
            avgStr = std::to_string(Points.average);
        }
        else if (isPPH && PPH.average > 0) {
            avgStr = std::to_string(PPH.average);
        }
        else if (!isPointsRow) {
            avgStr = secondsToTime(static_cast<int>(std::round(ps.avg)));
        }




        std::string recentStr = "-          -";

        if (isTotalPoints && Points.recent > 0) {
            recentStr = formatRecentValue(
                Points.recent,
                Points.average,
                false, true);
        }
        else if (isPPH && PPH.recent > 0) {
            recentStr = formatRecentValue(
                PPH.recent,
                PPH.average,
                false, true);
        }
        else if (!isPointsRow && recentVal.count(key)) {
            recentStr = formatRecentValue(
                recentVal.at(key),
                ps.avg,
                true, false);
        }




        // vs - same
        //std::string compStr = isPointsRow ? "" : "           0:00";
        std::string compStr = "";


        if (hasSecondary && !isPointsRow) {
            auto ssIt = secondaryStats.find(key);
            if (ssIt != secondaryStats.end()) {
                const auto& ss = ssIt->second;
                if (ss.avg > 0.5) {
                    std::string tstr = secondsToTime(static_cast<int>(std::round(ss.avg)));
                    double diff = ps.avg - ss.avg;
                    int diffAbs = static_cast<int>(std::round(std::abs(diff)));
                    std::string dstr = secondsToTime(diffAbs);
                    std::string sign = (std::abs(diff) < 0.5) ? " " : (diff < 0 ? "-" : "+");
                    std::string col = (std::abs(diff) < 0.5) ? COLOR_RESET : (diff < 0 ? COLOR_GREEN : COLOR_RED);
                    compStr = tstr + " " + col + sign + dstr + COLOR_RESET;
                }
            }
        }

        std::cout << std::left << std::setw(NW) << key << std::string(SEP, ' ')
            << std::right << std::setw(TW) << bestStr << std::string(SEP, ' ')
            << std::right << std::setw(AW) << avgStr << std::string(SEP, ' ')
            << padRightAligned(recentStr, RW);


        if (hasSecondary)
            std::cout << std::string(SEP, ' ') << std::right << std::setw(CW) << compStr;
        std::cout << "\n";

        if (key == "Pre-Olm" || key == "Raid Completed" || key == "Between room time")
            std::cout << std::string(totalWidth, '-') << "\n";
    }
    std::cout << std::string(totalWidth, '=') << "\n\n";
}

void printMostCommonPrepRooms(const std::vector<std::pair<std::string, const Stats*>>& common, int raids5, int raids6, int raidsOther, int totalRaids)
{
    if (!common.empty()) {
        std::cout << "Most Common Prep Rooms:\n";
        std::cout << std::string(62, '-') << "\n";
        std::cout << std::left << std::setw(28) << "Room"
            << std::right << std::setw(15) << "Average Time"
            << std::setw(15) << "Times Completed" << "\n";
        std::cout << std::string(62, '-') << "\n";
        for (const auto& [room, st] : common) {
            std::cout << std::left << std::setw(28) << room
                << std::right << std::setw(15) << secondsToTime(static_cast<int>(std::round(st->avg)))
                << std::setw(15) << st->validCount << "\n";
        }
        std::cout << "\n";
        double pct5 = totalRaids > 0 ? raids5 * 100.0 / totalRaids : 0.0;
        double pct6 = totalRaids > 0 ? raids6 * 100.0 / totalRaids : 0.0;
        std::cout << "Room count distribution: 5 rooms = " << raids5 << " (" << std::fixed << std::setprecision(1) << pct5 << "%), "
            << "6 rooms = " << raids6 << " (" << pct6 << "%)";
        if (raidsOther > 0) std::cout << ", other = " << raidsOther;
        std::cout << "\n\n";
    }
}

void printDiscardedOutliers(const std::vector<std::tuple<int, std::string, int, std::string>>& discarded, const std::string& user, const std::string& label)
{
    if (!discarded.empty()) {
        std::cout << "Discarded Outliers (Primary - " << user << ") - " << discarded.size() << " items:\n";
        std::cout << std::string(80, '-') << "\n";
        for (const auto& [kc, room, time, reason] : discarded) {
            std::cout << "KC " << std::setw(5) << kc << " | "
                << std::left << std::setw(26) << room
                << std::right << std::setw(8) << secondsToTime(time)
                << "  (" << reason << ")\n";
        }
        std::cout << "\n";
    }
}

void printAnalysisSummary(const std::string& primaryUser, int totalRaids, bool hasSecondary, const std::string& secondaryUser,
    int pastRaids, int secondaryRaidsCount) {
    std::cout << "Analyzing " << (pastRaids == -1 ? "all" : "last " + std::to_string(pastRaids))
        << " solo raids from " << primaryUser << " (" << totalRaids << " raids)\n";
    if (hasSecondary) {
        std::cout << "Comparison vs " << secondaryUser << " (" << secondaryRaidsCount << " raids)\n";
    }
    std::cout << "\n";
}

std::string padRightAligned(const std::string& s, int width)
{
    int pad = width - visibleLength(s);
    if (pad <= 0) return s;
    return std::string(pad, ' ') + s;
}

