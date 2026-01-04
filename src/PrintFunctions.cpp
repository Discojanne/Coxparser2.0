#include "PrintFunctions.h"
#include "ComputeFunctions.h"

struct RowContext {
    bool isPointsRow;
    const PointsToPrint* pts; // nullptr for normal rows
};

void printRaidStatisticsHeader(const std::string& primaryUser, const std::string& secondaryUser, bool hasSecondary, int totalWidth, int nPastRaids)
{
    std::cout << "Raid Statistics - Primary: " << primaryUser << "\n";
    std::cout << std::string(totalWidth, '=') << "\n";

    std::cout << std::left << std::setw(NW) << "Room" << std::string(SEP, ' ')
        << std::right << std::setw(TW) << "Best" << std::string(SEP, ' ')
        << std::right << std::setw(AW) << "Average" << std::string(SEP, ' ')
        << centerText("Recent", RW) << std::string(SEP, ' ')
        << centerText("Last " + std::to_string(nPastRaids), LW);


    if (hasSecondary)
        std::cout << std::string(SEP, ' ')
        << centerText("vs " + secondaryUser, CW);


    std::cout << "\n";
    std::cout << std::string(totalWidth, '-') << "\n";
}

void printStatsTable(const std::map<std::string, Stats>& primaryStats, const std::map<std::string, Stats>& secondaryStats, const std::map<std::string,
    int>& recentVal, const std::string& secondaryUser, int totalWidth, bool hasSecondary, PointsToPrint PPH, PointsToPrint Points,
    std::map<std::string, double> lastNAvg)
{
    for (const auto& key : DISPLAY_ORDER) {
        auto it = primaryStats.find(key);
        if (it == primaryStats.end()) continue;
        // 
        const auto& ps = it->second;

        RowContext ctx{ false, nullptr };

        if (key == "Total Points") {
            ctx = { true, &Points };
        }
        else if (key == "PPH") {
            ctx = { true, &PPH };
        }

        if (ps.validCount == 0 && isPrepRoom(key)) continue;

        std::string bestStr = "--:--";
        std::string avgStr = "--:--";

        if (ctx.isPointsRow) {
            bestStr = std::to_string(ctx.pts->best);
            avgStr = std::to_string(ctx.pts->average);
        }
        else {
            bestStr = (ps.fastest > 0) ? secondsToTime(ps.fastest) : "--:--";
            avgStr = secondsToTime(static_cast<int>(std::round(ps.avg)));
        }

		// Comparison column (if applicable)
        std::string compStr = "";
        if (hasSecondary && !ctx.isPointsRow) {
            if (hasSecondary && !ctx.isPointsRow && secondaryStats.count(key))
            {
                auto ssIt = secondaryStats.find(key);
                if (ssIt != secondaryStats.end()) {
                    const auto& ss = ssIt->second;
                    if (ss.avg > 0.5) {
                        std::string tstr = secondsToTime(static_cast<int>(std::round(ss.avg)));
                        double diff = ps.avg - ss.avg;
                        int diffAbs = static_cast<int>(std::round(std::abs(diff)));
                        std::string dstr = secondsToTime(diffAbs);
                        std::string sign = (std::abs(diff) < 0.5) ? " " : (diff < 0 ? "-" : "+");
                        const char* col = diffColor(
                            static_cast<int>(std::round(diff)),
                            true,          // time comparison
                            false          // lower time is better
                        );

                        compStr = tstr + " " + col + sign + dstr + COLOR_RESET + " (" + std::to_string(ss.validCount) + ")";
                    }
                }
            }
        }

		// Printing starts here

        std::cout << std::left << std::setw(NW) << key << std::string(SEP, ' ')
            << std::right << std::setw(TW) << bestStr << std::string(SEP, ' ')
            << std::right << std::setw(AW) << avgStr << std::string(SEP, ' ');

        auto printValueCell = [&](bool hasValue, int value, double avg, bool isTime, bool invertColor)
            {
                if (hasValue) {
                    printCell(makeCell(value, avg, isTime, invertColor));
                }
                else {
                    std::cout << std::setw(VALUE_W + 1 + DIFF_W) << "-";
                }
            };

        if (ctx.isPointsRow) {
            printValueCell(
                ctx.pts->recent > 0,
                ctx.pts->recent,
                ctx.pts->average,
                false,
                true
            );
        }
        else {
            printValueCell(
                recentVal.count(key),
                recentVal.count(key) ? recentVal.at(key) : 0,
                ps.avg,
                true,
                false
            );
        }

        std::cout << std::string(SEP, ' ');

        if (ctx.isPointsRow) {
            printValueCell(
                lastNAvg.count(key),
                static_cast<int>(lastNAvg.at(key)),
                ctx.pts->average,
                false,
                true
            );
        }
        else {
            printValueCell(
                lastNAvg.count(key),
                static_cast<int>(lastNAvg.at(key)),
                ps.avg,
                true,
                false
            );
        }

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
    constexpr int MCP_ROOM_W = 10;
    constexpr int MCP_AVG_W = 15;
    constexpr int MCP_COUNT_W = 15;
    constexpr int MCP_SEP_W = 0;

    constexpr int MCP_TOTAL_W = MCP_ROOM_W + MCP_AVG_W + MCP_COUNT_W;

    if (common.empty())
        return;

    std::cout << "Most Common Prep Rooms:\n";
    std::cout << std::string(MCP_TOTAL_W, '-') << "\n";

    std::cout << std::left << std::setw(MCP_ROOM_W) << "Room"
        << std::right << std::setw(MCP_AVG_W) << "Avg time"
        << std::right << std::setw(MCP_COUNT_W) << "# Completed"
        << "\n";

    std::cout << std::string(MCP_TOTAL_W, '-') << "\n";

    for (const auto& [room, st] : common)
    {
        std::cout << std::left << std::setw(MCP_ROOM_W) << room
            << std::right << std::setw(MCP_AVG_W)
            << secondsToTime(static_cast<int>(std::round(st->avg)))
            << std::right << std::setw(MCP_COUNT_W)
            << st->validCount
            << "\n";
    }

    std::cout << "\n";

    double pct5 = totalRaids > 0 ? raids5 * 100.0 / totalRaids : 0.0;
    double pct6 = totalRaids > 0 ? raids6 * 100.0 / totalRaids : 0.0;

    std::cout << "Room count distribution: "
        << "5 rooms = " << raids5
        << " (" << std::fixed << std::setprecision(1) << pct5 << "%), "
        << "6 rooms = " << raids6
        << " (" << pct6 << "%)";

    if (raidsOther > 0)
        std::cout << ", other = " << raidsOther;

    std::cout << "\n\n";
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



void printRoomPPHTable(const std::vector<RoomPPHResult>& rows)
{
    constexpr int NW = 18;
    constexpr int PW = 10;
    constexpr int CW = 8;

    std::cout << "Room Efficiency (PPH)\n";
    std::cout << std::string(38, '=') << "\n";
    std::cout << std::left << std::setw(NW) << "Room"
        << std::right << std::setw(PW) << "Avg PPH"
        << std::right << std::setw(CW) << "Raids\n";
    std::cout << std::string(38, '-') << "\n";

    for (const auto& r : rows)
    {
        std::cout << std::left << std::setw(NW) << r.room
            << std::right << std::setw(PW) << r.avgPPH
            << std::right << std::setw(CW) << r.raids
            << "\n";
    }

    std::cout << std::string(38, '=') << "\n\n";
}

Cell makeCell(int value, double avg, bool isTime, bool positiveIsGood)
{
    Cell c{};

    c.value = isTime
        ? secondsToTime(value)
        : std::to_string(value);

    int d = value - static_cast<int>(std::round(avg));

    c.diff = (d >= 0 ? "+" : "-") +
        (isTime ? secondsToTime(std::abs(d))
            : std::to_string(std::abs(d)));

    c.color = diffColor(d, isTime, positiveIsGood);

    return c;
}

void printCell(const Cell& c)
{
    std::cout
        << std::right << std::setw(VALUE_W) << c.value
        << " "
        << c.color
        << std::right << std::setw(DIFF_W) << c.diff
        << COLOR_RESET;
}

std::string centerText(const std::string& text, int width)
{
    if (static_cast<int>(text.size()) >= width)
        return text.substr(0, width);

    int pad = width - static_cast<int>(text.size());
    int left = pad / 2;
    int right = pad - left;

    return std::string(left, ' ') + text + std::string(right, ' ');
}