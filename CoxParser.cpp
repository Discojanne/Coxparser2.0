#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <tuple>
#include <climits>
#include <filesystem>

//#include "PrintFunctions.cpp"

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

    std::cout << "Analyzing " << (PAST_RAIDS == -1 ? "all" : "last " + std::to_string(PAST_RAIDS))
        << " solo raids from " << primaryUser << " (" << totalRaids << " raids)\n";
    if (hasSecondary) {
        std::cout << "Comparison vs " << secondaryUser << " (" << (secondaryRaids.size() - secondaryStart) << " raids)\n";
    }
    std::cout << "\n";

    std::map<std::string, Stats> primaryStats, secondaryStats;
    for (const auto& k : DISPLAY_ORDER) {
        primaryStats[k];
        if (hasSecondary) secondaryStats[k];
    }

    for (const auto& k : DISPLAY_ORDER) {
        processStats(primaryStats, k, primaryRaids, primaryStart);
        if (hasSecondary) processStats(secondaryStats, k, secondaryRaids, secondaryStart);
    }

    // Recent raid values
    std::map<std::string, int> recentVal;
    if (primaryRaids.size() > primaryStart) {
        const auto& rr = primaryRaids.back();
        int prep = 0;
        int olm = rr.times.count("Olm") ? rr.times.at("Olm") : 0;
        int total = rr.times.at("Raid Completed");

        for (const auto& room : PREP_ROOMS) {
            if (rr.times.count(room)) prep += rr.times.at(room);
        }

        recentVal["Pre-Olm"] = prep;
        recentVal["Olm"] = olm;
        recentVal["Raid Completed"] = total;
        recentVal["Between room time"] = total - prep - olm;
        for (const auto& [k, v] : rr.times) recentVal[k] = v;
    }

    // Room count distribution (5 or 6 rooms, or other)
    int raids5 = 0, raids6 = 0, raidsOther = 0;
    for (size_t i = primaryStart; i < primaryRaids.size(); ++i) {
        const auto& r = primaryRaids[i];
        int count = 0;
        for (const auto& room : PREP_ROOMS) {
            if (r.times.count(room)) ++count;
        }
        if (count == 5) ++raids5;
        else if (count == 6) ++raids6;
        else ++raidsOther;
    }

    // Determine width for prep-room count column (for table alignment)
    int maxPrepCount = 0;
    for (const auto& room : PREP_ROOMS) {
        maxPrepCount = std::max(maxPrepCount, primaryStats[room].validCount);
    }
    int countPad = (maxPrepCount == 0) ? 1 : static_cast<int>(std::to_string(maxPrepCount).length());

    // Collect and sort all discarded outliers across rooms for reporting
    std::vector<std::tuple<int, std::string, int, std::string>> primaryDiscarded, secondaryDiscarded;
    for (const auto& pair : primaryStats) {
        const auto& st = pair.second;
        for (const auto& d : st.discarded) primaryDiscarded.push_back(d);
    }
    for (const auto& pair : secondaryStats) {
        const auto& st = pair.second;
        for (const auto& d : st.discarded) secondaryDiscarded.push_back(d);
    }
    std::sort(primaryDiscarded.rbegin(), primaryDiscarded.rend());
    std::sort(secondaryDiscarded.rbegin(), secondaryDiscarded.rend());

    /// ==================== PERFECT TABLE - 12 CHAR FIXED STRINGS ====================                     

    //printTable();

    const int NW = 26;
    const int TW = 10;
    const int AW = 24;
    const int RW = 12;  // "XX:XX  +XX:XX" = 12 chars

    const int SEP = 3;

    int totalWidth = NW + TW + AW + RW + (!hasSecondary ? 0 : RW) + SEP * 4;

    std::cout << "Raid Statistics - Primary: " << primaryUser << "\n";
    std::cout << std::string(totalWidth, '=') << "\n";

    std::cout << std::left << std::setw(NW) << "Room" << std::string(SEP, ' ')
        << std::right << std::setw(TW) << "Fastest" << std::string(SEP, ' ')
        << std::right << std::setw(AW) << "Average (count)" << std::string(SEP, ' ')
        << std::right << std::setw(RW) << "Recent";
    if (hasSecondary)
        std::cout << std::string(SEP, ' ') << std::right << std::setw(RW) << ("vs " + secondaryUser);
    std::cout << "\n";

    std::cout << std::string(totalWidth, '-') << "\n";

    for (const auto& key : DISPLAY_ORDER) {
        const auto& ps = primaryStats[key];
        bool isPrepRoom = std::find(PREP_ROOMS.begin(), PREP_ROOMS.end(), key) != PREP_ROOMS.end();
        if (ps.validCount == 0 && isPrepRoom) continue;

        std::string fastestStr = (ps.fastest <= 0) ? "--:--" : secondsToTime(ps.fastest);

        std::ostringstream avgOss;
        if (isPrepRoom) {
            avgOss << secondsToTime(static_cast<int>(std::round(ps.avg))) << " ("
                << std::setw(countPad) << std::right << ps.validCount << ")";
        }
        else {
            avgOss << secondsToTime(static_cast<int>(std::round(ps.avg)));
        }

        // Recent - always exactly 12 visible characters
        std::string recentStr = "--:--  0:00";  // default = no time
        if (recentVal.count(key)) {
            std::string tstr = secondsToTime(recentVal.at(key));
            double diff = recentVal.at(key) - ps.avg;
            int diffAbs = static_cast<int>(std::round(std::abs(diff)));
            std::string dstr = secondsToTime(diffAbs);

            std::string sign = " ";  // always start with space
            std::string col = COLOR_RESET;
            if (std::abs(diff) >= 0.5) {
                sign = (diff < 0 ? "-" : "+");
                col = (diff < 0 ? COLOR_GREEN : COLOR_RED);
            }

            recentStr = tstr + " " + col + sign + dstr + COLOR_RESET;
        }

        // vs - same
        std::string compStr = "           0:00";
        if (hasSecondary) {
            const auto& ss = secondaryStats[key];
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

        std::cout << std::left << std::setw(NW) << key << std::string(SEP, ' ')
            << std::right << std::setw(TW) << fastestStr << std::string(SEP, ' ')
            << std::right << std::setw(AW) << avgOss.str() << std::string(SEP, ' ')
            << std::right << std::setw(RW) << recentStr;
        if (hasSecondary)
            std::cout << std::string(SEP, ' ') << std::right << std::setw(RW) << compStr;
        std::cout << "\n";

        if (key == "Pre-Olm" || key == "Raid Completed")
            std::cout << std::string(totalWidth, '-') << "\n";
    }
    std::cout << std::string(totalWidth, '=') << "\n\n";

    // Most Common + room count distribution
    std::vector<std::pair<std::string, const Stats*>> common;
    for (const auto& room : PREP_ROOMS) {
        const auto& st = primaryStats[room];
        if (st.validCount > 0) common.emplace_back(room, &st);
    }
    std::sort(common.begin(), common.end(), [](const auto& a, const auto& b) {
        return a.second->validCount > b.second->validCount ||
            (a.second->validCount == b.second->validCount && a.first < b.first);
        });

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

    // Discarded outliers
    if (!primaryDiscarded.empty()) {
        std::cout << "Discarded Outliers (Primary - " << primaryUser << ") - " << primaryDiscarded.size() << " items:\n";
        std::cout << std::string(80, '-') << "\n";
        for (const auto& [kc, room, time, reason] : primaryDiscarded) {
            std::cout << "KC " << std::setw(5) << kc << " | "
                << std::left << std::setw(26) << room
                << std::right << std::setw(8) << secondsToTime(time)
                << "  (" << reason << ")\n";
        }
        std::cout << "\n";
    }

    if (hasSecondary && !secondaryDiscarded.empty()) {
        std::cout << "Discarded Outliers (Secondary - " << secondaryUser << ") - " << secondaryDiscarded.size() << " items:\n";
        std::cout << std::string(80, '-') << "\n";
        for (const auto& [kc, room, time, reason] : secondaryDiscarded) {
            std::cout << "KC " << std::setw(5) << kc << " | "
                << std::left << std::setw(26) << room
                << std::right << std::setw(8) << secondsToTime(time)
                << "  (" << reason << ")\n";
        }
        std::cout << "\n";
    }
}