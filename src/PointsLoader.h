#pragma once
#include <iostream>
#include <fstream>
#include <cmath>
#include <cctype>
#include <map>
#include <string>

#include "Types.h"
#include "InputFunctions.h"

struct PrimaryRaid
{
    int kc;
    int raidSeconds;
    int floor1Seconds;
};

struct PointsRaid
{
    int raidSeconds;
    // Floor-1 seconds from the tracker. May be -1 when the plugin fails to
    // write upperTime (seen on some full-layout solos); join must tolerate that.
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

// Post-cutoff slice of the points log (unique-table weight change).
struct PurpleEraAnalysis
{
    long long pointsPostRegular = 0;
    long long pointsPostCm = 0;
    int nPostRegular = 0;
    int nPostCm = 0;
    int purplesPostRegular = 0;
    int purplesPostCm = 0;
    std::map<std::string, int> itemsPostRegular;
    std::map<std::string, int> itemsPostCm;
    std::map<std::string, int> itemsPost; // reg + CM combined
    PurpleHistory historyAll;
    PurpleHistory historyPost;
};

// Raids flagged as "died" when personal points fall below layout-specific cutoffs.
struct DeathStats
{
    int total = 0;
    int deaths = 0;
    int nFullRegular = 0;
    int deathsFullRegular = 0;
    int nNormalRegular = 0;
    int deathsNormalRegular = 0;
    int nCmSolo = 0;
    int deathsCmSolo = 0;
    int nCmTeam = 0;
    int deathsCmTeam = 0;
};

int parseTimeMMSS(const std::string& s);

int parseIntWithCommas(const std::string& s);

bool extractInt(const std::string& line, const std::string& key, int& out);

bool extractBool(const std::string& line, const std::string& key, bool& out);

bool extractString(const std::string& line, const std::string& key, std::string& out);

// True if profileType contains "LEAGUE" (e.g. DEMONIC_PACTS_LEAGUE). Always skip these.
bool isLeagueProfile(const std::string& line);

// True if this row's specialLoot is attributed to primaryUser (or no receiver field).
bool gotPurpleForUser(const std::string& line, const std::string& primaryUser);

std::vector<PrimaryRaid> loadPrimary(const std::string& path);

std::vector<PointsRaid> loadPointsFile(const std::string& path);

std::map<int, int> loadPoints(
    const std::string& primaryPath,
    const std::string& pointsPath);

int readMaxCoxKC(const std::string& coxTimesPath);

int readMaxCmKC(const std::string& cmTimesPath);

PointsLogStats summarizePointsLog(const std::string& pointsPath);

DeathStats summarizeDeathStats(
    const std::string& pointsPath,
    int thresholdFullRegular,
    int thresholdRegular,
    int thresholdCmSolo,
    int thresholdCmTeam);

AccountBreakdown buildAccountBreakdown(
    int regularKC,
    int cmKC,
    int nTracked,
    double avgTrackedSoloPoints,
    const PointsLogStats& pointsStats);

PurpleEraAnalysis loadPurpleEraAnalysis(
    const std::string& pointsPath,
    const std::string& primaryUser,
    int rateChangeKc,
    int rateChangeCmKc,
    int regularKc,
    int cmKc);