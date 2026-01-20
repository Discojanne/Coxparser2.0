#pragma once
#include "itemCompute.h"
#include "InputFunctions.h"

void printPurpleSummary(const PurpleSummary& s);

void printPurpleItemTable(const std::vector<PurpleItemStat>& stats);

void printPurpleHistory(const PurpleHistory& hist,
    double purpleRate,
    int rowWidth,
    int totalRaidsGlobal,
    int actualPurples);

void printSectionDivider(const std::string& title, int width = 100);
