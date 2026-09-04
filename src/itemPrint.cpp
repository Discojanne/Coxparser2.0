#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "itemPrint.h"
#include "Types.h"

void printPurpleSummary(const PurpleSummary& s)
{
    std::cout << "Purple Summary\n";
    std::cout << std::string(40, '-') << "\n";

    std::cout << std::left << std::setw(22) << "Effective KC"
        << std::right << std::setw(10)
        << std::fixed << std::setprecision(1) << s.effectiveKC << "\n";

    std::cout << std::left << std::setw(22) << "Avg personal pts"
        << std::right << std::setw(10)
        << std::fixed << std::setprecision(0) << s.avgPersonalPoints << "\n";

    std::ostringstream rateStr;
    rateStr << "1/" << std::fixed << std::setprecision(2) << s.purpleRate;
    std::cout << std::left << std::setw(22) << "Purple rate"
        << std::right << std::setw(10) << rateStr.str() << "\n";

    std::cout << std::left << std::setw(22) << "Expected purples"
        << std::right << std::setw(10)
        << std::fixed << std::setprecision(1) << s.expectedPurples << "\n";

    std::cout << std::left << std::setw(22) << "Actual purples"
        << std::right << std::setw(10) << s.actualPurples << "\n";

    std::ostringstream scrollStr;
    scrollStr << std::fixed << std::setprecision(1) << s.prayerScrollPct << "%"
        << " (" << s.prayerScrolls << ")";
    std::cout << std::left << std::setw(22) << "Prayer scrolls"
        << std::right << std::setw(10) << scrollStr.str() << "\n";

    const char* col = (s.diff >= 0) ? COLOR_GREEN : COLOR_RED;

    std::ostringstream diffStr;
    diffStr << std::showpos << std::fixed << std::setprecision(1) << s.diff
        << " (" << s.diffRaids << " raids)";

    std::cout << std::left << std::setw(22) << "Difference"
        << col
        << std::right << std::setw(14) << diffStr.str()
        << COLOR_RESET << "\n";

    std::cout << std::noshowpos << "\n";
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
    int preferredMaxWidth,
    int /*totalRaidsGlobal*/,
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

    const int expectedEvery =
        std::max(1, static_cast<int>(std::round(purpleRate)));

    // Row width = largest multiple of expectedEvery that fits preferredMaxWidth,
    // so red ' markers line up in the same columns on every row.
    int rowWidth = expectedEvery;
    if (preferredMaxWidth >= expectedEvery)
        rowWidth = (preferredMaxWidth / expectedEvery) * expectedEvery;
    if (rowWidth < 1)
        rowWidth = preferredMaxWidth > 0 ? preferredMaxWidth : 80;

    std::cout << std::noshowpos;
    std::cout << "Purple History (solo+team+CM, oldest to newest)\n";
    std::cout << "Raids: " << trackedRaids
        << " | Purples: " << trackedPurples;
    if (hist.cmRaids > 0)
    {
        std::cout << "  (CM: " << hist.cmRaids
            << " raids, " << hist.cmPurples << " purples)";
    }
    std::cout << "  |  ' = every " << expectedEvery << " raids\n";

    std::cout << std::string(rowWidth, '-') << "\n";

    int sinceLast = 0;
    int longestDry = 0;

    int dryStreakCount = 0;
    int dryStreakSum = 0;

    int raidIndex = 0;
    int charsInRow = 0;

    for (bool gotPurple : hist.hasPurple)
    {
        raidIndex++;
        sinceLast++;

        char c = '.';
        const char* col = nullptr;

        if (gotPurple)
        {
            c = '+';
            col = COLOR_GREEN;

            int finishedStreak = sinceLast - 1;
            if (finishedStreak >= 0)
            {
                longestDry = std::max(longestDry, finishedStreak);
                dryStreakSum += finishedStreak;
                dryStreakCount++;
            }

            sinceLast = 0;
        }
        else if (raidIndex % expectedEvery == 0)
        {
            c = '\'';
            col = COLOR_RED;
        }

        if (col)
            std::cout << col << c << COLOR_RESET;
        else
            std::cout << c;

        ++charsInRow;
        if (charsInRow == rowWidth)
        {
            std::cout << "\n";
            charsInRow = 0;
        }
    }

    if (charsInRow > 0)
        std::cout << "\n";

    const int currentDry = sinceLast;

    // trailing dry streak counts toward longest / average
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
    std::cout << "Current dry streak: " << currentDry << " raids\n";
    std::cout << "Longest dry streak: " << longestDry << " raids\n";
    std::cout << "Average dry streak: "
        << std::fixed << std::setprecision(1)
        << avgDry << " raids\n";

    // Purples outside this logged history (untracked era, etc.)
    const int otherPurples = actualPurples - trackedPurples;
    if (otherPurples > 0)
    {
        std::cout << "Purples outside this history: " << otherPurples
            << "  (of " << actualPurples << " total actual)\n";
    }

    std::cout << "\n";
}

void printAccountBreakdown(const AccountBreakdown& b)
{
    std::cout << "Account Breakdown\n";
    std::cout << std::string(40, '-') << "\n";

    auto row = [](const char* label, const std::string& value)
    {
        std::cout << std::left << std::setw(22) << label
            << std::right << std::setw(12) << value << "\n";
    };

    row("Regular KC", std::to_string(b.regularKC));
    row("  Solo", std::to_string(b.nSoloLogged));
    row("  Team", std::to_string(b.nTeam));
    row("  Untracked", std::to_string(b.nUntracked));
    row("CM", std::to_string(b.nCM));
    row("  CM (logged pts)", std::to_string(b.nCMLogged));
    if (b.nCMMissing > 0)
        row("  CM (estimated)", std::to_string(b.nCMMissing));

    std::ostringstream cmEquiv;
    cmEquiv << std::fixed << std::setprecision(1) << b.cmEquiv;
    row("CM equiv", cmEquiv.str());

    const double effective = b.regularKC + b.cmEquiv;
    std::ostringstream eff;
    eff << std::fixed << std::setprecision(1) << effective;
    row("Effective (KC+CM)", eff.str());

    row("Personal pts (est.)", std::to_string(b.sumPersonalKnown));

    if (b.nTracked > b.nSoloLogged)
    {
        std::cout << "\n[warn] Joined solos (" << b.nTracked
            << ") > Solo (" << b.nSoloLogged
            << ") — join/filter mismatch?\n";
    }

    std::cout << "\n";
}

void printDeathStats(const DeathStats& d)
{
    if (d.total <= 0)
        return;

    std::cout << "Death estimate (from pts)\n";
    std::cout << std::string(40, '-') << "\n";

    auto row = [](const char* label, const std::string& value)
    {
        std::cout << std::left << std::setw(22) << label
            << std::right << std::setw(16) << value << "\n";
    };

    auto formatLine = [](int deaths, int total)
    {
        std::ostringstream s;
        const double pct = (total > 0)
            ? 100.0 * static_cast<double>(deaths) / total
            : 0.0;
        s << deaths << "/" << total
            << "  (" << std::fixed << std::setprecision(1) << pct << "%)";
        return s.str();
    };

    row("Raids with death", formatLine(d.deaths, d.total));

    if (d.nFullRegular > 0)
        row("  Full regular", formatLine(d.deathsFullRegular, d.nFullRegular));
    if (d.nNormalRegular > 0)
        row("  Regular", formatLine(d.deathsNormalRegular, d.nNormalRegular));
    if (d.nCmSolo > 0)
        row("  CM solo", formatLine(d.deathsCmSolo, d.nCmSolo));
    if (d.nCmTeam > 0)
        row("  CM team", formatLine(d.deathsCmTeam, d.nCmTeam));

    std::cout << "\n";
}

void printSectionDivider(const std::string& title, int width)
{
    const std::string line(width, '=');
    const std::string accent(width, '~');

    std::cout << "\n"
        << COLOR_CYAN << COLOR_DIM << accent << COLOR_RESET << "\n"
        << COLOR_BRIGHT_MAGENTA << line << COLOR_RESET << "\n"
        << COLOR_BRIGHT_MAGENTA << COLOR_BOLD
        << "  ***  " << title << "  ***"
        << COLOR_RESET << "\n"
        << COLOR_DIM << "  (times done — loot, rates & purple history)"
        << COLOR_RESET << "\n"
        << COLOR_BRIGHT_MAGENTA << line << COLOR_RESET << "\n"
        << COLOR_CYAN << COLOR_DIM << accent << COLOR_RESET << "\n\n";
}
