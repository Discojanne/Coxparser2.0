#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <cctype>

struct PrimaryRaid
{
    int kc;
    int raidSeconds;
    int floor1Seconds;
};

struct PointsRaid
{
    int raidSeconds;
    int upperSeconds;
    int totalPoints;
};

struct PointsToPrint
{
	int best;
    int average;
	int recent;
	int recentDiff;
};

int parseTimeMMSS(const std::string& s);

int parseIntWithCommas(const std::string& s);

bool extractInt(const std::string& line, const std::string& key, int& out);

bool extractBool(const std::string& line, const std::string& key, bool& out);

std::vector<PrimaryRaid> loadPrimary(const std::string& path);

std::vector<PointsRaid> loadPointsFile(const std::string& path);

std::map<int, int> loadPoints(
    const std::string& primaryPath,
    const std::string& pointsPath);