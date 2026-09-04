#pragma once
#include "itemCompute.h"
#include "InputFunctions.h"
#include "PointsLoader.h"

void printPurpleSummary(const PurpleSummary& s);

void printPurpleItemTable(const std::vector<PurpleItemStat>& stats);

void printPurpleHistory(const PurpleHistory& hist,
    double purpleRate,
    int preferredMaxWidth,
    int totalRaidsGlobal,
    int actualPurples);

void printAccountBreakdown(const AccountBreakdown& b);

void printDeathStats(const DeathStats& d);

void printSectionDivider(const std::string& title, int width = 100);
