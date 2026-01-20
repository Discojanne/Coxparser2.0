#pragma once

#include "Types.h"

struct PurpleHistory
{
    std::vector<bool> hasPurple; // true = YOU got a purple this raid
};

std::string getUsername(const std::string& path);

bool readRaids(const std::string& filename, std::vector<Raid>& raids);

PurpleHistory loadPurpleHistory(
    const std::string& logFile,
    const std::string& primaryUser);