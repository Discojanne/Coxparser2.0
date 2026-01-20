#pragma once
#include "itemCompute.h"
#include "InputFunctions.h"

void printPurpleSummary(const PurpleSummary& s);

void printPurpleItemTable(const std::vector<PurpleItemStat>& stats);

void printPurpleHistory(const PurpleHistory& hist, double purpleRate, int rowWidth = 100);

void printSectionDivider(const std::string& title, int width = 100);
