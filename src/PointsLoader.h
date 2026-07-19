#pragma once
#include <iostream>
#include <fstream>
#include <cmath>
#include <cctype>

#include "Types.h"

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

// Counts / point sums from the full points log (regular + team + CM).
struct PointsLogStats
{
    int nSoloRegular = 0;              // non-CM, teamSize == 1
    int nTeamRegular = 0;              // non-CM, teamSize > 1
    int nCM = 0;                       // challengeMode
    long long sumPersonalAll = 0;      // all counted rows
    long long sumPersonalSoloRegular = 0;
    long long sumPersonalTeamRegular = 0;
    long long sumPersonalCM = 0;
};

// Account-level breakdown for display / expected purple (later).
struct AccountBreakdown
{
    int regularKC = 0;       // max CoX KC from CoxTimes
    int nTracked = 0;        // solos with times + points joined
    int nSoloLogged = 0;     // solo regular rows in points log
    int nTeam = 0;           // team regular rows in points log
    int nCM = 0;             // max CoX CM KC from CmTimes (ground truth)
    int nCMLogged = 0;       // CM rows with points in the points log
    int nCMMissing = 0;      // nCM - nCMLogged (points estimated from avg)
    int nUntracked = 0;      // regularKC - solos logged - teams logged
    double cmEquiv = 0.0;    // estimated CM personal points / avg tracked solo points
    long long sumPersonalKnown = 0;      // logged personal pts (all modes)
    long long sumPersonalCMEst = 0;      // logged CM pts + estimate for missing
};

int parseTimeMMSS(const std::string& s);

int parseIntWithCommas(const std::string& s);

bool extractInt(const std::string& line, const std::string& key, int& out);

bool extractBool(const std::string& line, const std::string& key, bool& out);

// True if profileType contains "LEAGUE" (e.g. DEMONIC_PACTS_LEAGUE). Always skip these.
bool isLeagueProfile(const std::string& line);

std::vector<PrimaryRaid> loadPrimary(const std::string& path);

std::vector<PointsRaid> loadPointsFile(const std::string& path);

std::map<int, int> loadPoints(
    const std::string& primaryPath,
    const std::string& pointsPath);

int readMaxCoxKC(const std::string& coxTimesPath);

int readMaxCmKC(const std::string& cmTimesPath);

PointsLogStats summarizePointsLog(const std::string& pointsPath);

AccountBreakdown buildAccountBreakdown(
    int regularKC,
    int cmKC,
    int nTracked,
    double avgTrackedSoloPoints,
    const PointsLogStats& pointsStats);