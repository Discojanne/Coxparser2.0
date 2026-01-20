#include <iostream>
#include <iomanip>
#include <string>

#include "itemPrint.h"
#include "Types.h"

void printPurpleSummary(const PurpleSummary& s)
{
    std::cout << "Purple Summary\n";
    std::cout << std::string(40, '-') << "\n";

    std::cout << std::left << std::setw(20) << "Total raids"
        << std::right << std::setw(10) << s.totalRaids << "\n";

    std::cout << std::left << std::setw(20) << "Purple rate"
        << std::right << std::setw(10)
        << ("1/" + std::to_string(s.purpleRate)) << "\n";

    std::cout << std::left << std::setw(20) << "Expected purples"
        << std::right << std::setw(10)
        << std::fixed << std::setprecision(1)
        << s.expectedPurples << "\n";

    std::cout << std::left << std::setw(20) << "Actual purples"
        << std::right << std::setw(10)
        << s.actualPurples << "\n";

    const char* col = (s.diff >= 0) ? COLOR_GREEN : COLOR_RED;

    std::cout << std::left << std::setw(20) << "Difference"
        << col
        << std::right << std::setw(10)
        << std::showpos << std::fixed << std::setprecision(1)
        << s.diff
        << COLOR_RESET << "\n";

    std::cout << "\n";
}

void printPurpleItemTable(const std::vector<PurpleItemStat>& stats)
{
    std::cout << "Purple Items\n";
    std::cout << std::string(92, '-') << "\n";

    std::cout << std::left << std::setw(32) << "Item"
        << std::right
        << std::setw(6) << "Got"
        << std::setw(10) << "Expected"
        << std::setw(8) << "Diff"
        << std::setw(10) << "On Rate"
        << std::setw(8) << "Diff"
        << "\n";

    std::cout << std::string(92, '-') << "\n";

    for (const auto& s : stats) {
        const char* colActual = (s.diffActual >= 0) ? COLOR_GREEN : COLOR_RED;
        const char* colRate = (s.diffOnRate >= 0) ? COLOR_GREEN : COLOR_RED;

        std::cout << std::left << std::setw(32) << s.name
            << std::right
            << std::setw(6) << s.got
            << std::setw(10) << std::fixed << std::setprecision(1) << s.expectedActual
            << colActual
            << std::setw(8) << std::fixed << std::setprecision(1) << s.diffActual
            << COLOR_RESET
            << std::setw(10) << std::fixed << std::setprecision(1) << s.expectedOnRate
            << colRate
            << std::setw(8) << std::fixed << std::setprecision(1) << s.diffOnRate
            << COLOR_RESET
            << "\n";
    }

    std::cout << "\n";
}

void printPurpleHistory(const PurpleHistory& hist,
    double purpleRate,
    int rowWidth,
    int totalRaidsGlobal,
    int actualPurples)
{
    if (hist.hasPurple.empty())
        return;

    const int trackedRaids =
        static_cast<int>(hist.hasPurple.size());

    const int trackedPurples =
        static_cast<int>(std::count(hist.hasPurple.begin(),
            hist.hasPurple.end(),
            true));

    std::cout << std::noshowpos;
    std::cout << "Purple History (oldest to newest, mostly since KC: 457)\n";
    std::cout << "Raids tracked: " << trackedRaids
        << " | Purples: " << trackedPurples << "\n";

    std::cout << std::string(rowWidth, '-') << "\n";

    int sinceLast = 0;
    int longestDry = 0;

    int dryStreakCount = 0;
    int dryStreakSum = 0;

    std::string row;
    row.reserve(rowWidth);

    int raidIndex = 0;
    const int expectedEvery =
        static_cast<int>(std::round(purpleRate));

    for (bool gotPurple : hist.hasPurple)
    {
        raidIndex++;
        sinceLast++;

        char c = '.';

        if (gotPurple)
        {
            c = '+';

            int finishedStreak = sinceLast - 1;
            if (finishedStreak >= 0)
            {
                longestDry = std::max(longestDry, finishedStreak);
                dryStreakSum += finishedStreak;
                dryStreakCount++;
            }

            sinceLast = 0;
        }
        else if (expectedEvery > 0 && raidIndex % expectedEvery == 0)
        {
            c = '\''; // expected marker
        }

        row.push_back(c);

        if (row.size() == static_cast<size_t>(rowWidth))
        {
            std::cout << row << "\n";
            row.clear();
        }
    }

    if (!row.empty())
        std::cout << row << "\n";

    // trailing dry streak
    if (sinceLast > 0)
    {
        longestDry = std::max(longestDry, sinceLast);
        dryStreakSum += sinceLast;
        dryStreakCount++;
    }

    double avgDry = dryStreakCount > 0
        ? static_cast<double>(dryStreakSum) / dryStreakCount
        : 0.0;

    std::cout << std::string(rowWidth, '-') << "\n";
    std::cout << "Longest dry streak: " << longestDry << " raids\n";
    std::cout << "Average dry streak: "
        << std::fixed << std::setprecision(1)
        << avgDry << " raids\n";

    // ================= UNTRACKED DATA =================

    const int untrackedRaids =
        totalRaidsGlobal - trackedRaids;

    const int untrackedPurples =
        actualPurples - trackedPurples;

    if (untrackedRaids > 0)
    {
        std::cout << "Untracked raids:  "
            << untrackedRaids
            << " | Purples: "
            << untrackedPurples;

        if (untrackedPurples > 0)
        {
            double untrackedRate =
                static_cast<double>(untrackedRaids) / untrackedPurples;

            std::cout << " | Avg rate: 1/"
                << std::fixed << std::setprecision(2)
                << untrackedRate;
        }

        std::cout << "\n";
    }

    std::cout << "\n";
}

void printSectionDivider(const std::string& title, int width)
{
    std::string line(width, '=');
    std::cout << "\n" << line << "\n";
    std::cout << title << "\n";
    std::cout << line << "\n\n";
}
