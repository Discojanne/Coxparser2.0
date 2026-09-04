#include "PointsLoader.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

int parseTimeMMSS(const std::string& s)
{
    int m = 0, sec = 0;
    sscanf_s(s.c_str(), "%d:%d", &m, &sec);
    return m * 60 + sec;
}

int parseIntWithCommas(const std::string& s)
{
    std::string clean;
    for (char c : s)
        if (std::isdigit(c)) clean.push_back(c);
    return std::stoi(clean);
}

bool extractInt(const std::string& line, const std::string& key, int& out)
{
    auto p = line.find(key);
    if (p == std::string::npos) return false;
    p = line.find(':', p);
    if (p == std::string::npos) return false;
    out = std::stoi(line.substr(p + 1));
    return true;
}

bool extractBool(const std::string& line, const std::string& key, bool& out)
{
    auto p = line.find(key);
    if (p == std::string::npos) return false;
    p = line.find(':', p);
    out = line.substr(p + 1, 5).find("true") != std::string::npos;
    return true;
}

bool extractString(const std::string& line, const std::string& key, std::string& out)
{
    auto p = line.find(key);
    if (p == std::string::npos)
        return false;

    p += key.size();
    auto end = line.find('"', p);
    if (end == std::string::npos)
        return false;

    out = line.substr(p, end - p);
    return true;
}

bool isLeagueProfile(const std::string& line)
{
    constexpr const char* KEY = "\"profileType\":\"";
    auto p = line.find(KEY);
    if (p == std::string::npos)
        return false;

    p += std::strlen(KEY);
    auto end = line.find('"', p);
    if (end == std::string::npos)
        return false;

    for (size_t i = p; i + 5 < end; ++i)
    {
        // Case-insensitive "LEAGUE"
        if ((line[i] == 'L' || line[i] == 'l') &&
            (line[i + 1] == 'E' || line[i + 1] == 'e') &&
            (line[i + 2] == 'A' || line[i + 2] == 'a') &&
            (line[i + 3] == 'G' || line[i + 3] == 'g') &&
            (line[i + 4] == 'U' || line[i + 4] == 'u') &&
            (line[i + 5] == 'E' || line[i + 5] == 'e'))
        {
            return true;
        }
    }
    return false;
}

bool gotPurpleForUser(const std::string& line, const std::string& primaryUser)
{
    std::string loot;
    if (!extractString(line, "\"specialLoot\":\"", loot) || loot.empty())
        return false;

    std::string receiver;
    if (!extractString(line, "\"specialLootReceiver\":\"", receiver))
        return true; // loot present, no receiver field

    return receiver.empty() || receiver == primaryUser;
}

std::vector<PrimaryRaid> loadPrimary(const std::string& path)
{
    std::ifstream file(path);
    std::string line;

    std::vector<PrimaryRaid> raids;

    int raidTime = -1;
    int floor1Time = -1;
    int kc = -1;

    while (std::getline(file, line))
    {
        if (line.rfind("Floor 1:", 0) == 0)
        {
            floor1Time = parseTimeMMSS(line.substr(8));
        }
        else if (line.rfind("Raid Completed:", 0) == 0)
        {
            if (line.find("Team Size: 1") == std::string::npos)
            {
                raidTime = -1;     // mark invalid raid
                floor1Time = -1;
                continue;
            }
            auto colon = line.find(':');
            auto pipe = line.find('|');
            raidTime = parseTimeMMSS(line.substr(colon + 1, pipe - colon - 1));
        }

        else if (line.rfind("CoX KC:", 0) == 0)
        {
            kc = parseIntWithCommas(line.substr(7));
            if (raidTime > 0 && floor1Time > 0)
            {
                raids.push_back({ kc, raidTime, floor1Time });
            }
            raidTime = floor1Time = -1;
        }
    }

    return raids;
}

std::vector<PointsRaid> loadPointsFile(const std::string& path)
{
    std::ifstream file(path);
    std::string line;

    std::vector<PointsRaid> raids;

    while (std::getline(file, line))
    {
        if (isLeagueProfile(line))
            continue;

        bool challenge = true;
        int teamSize = -1;
        int raidTime = -1;
        int upperTime = -1;
        int totalPoints = -1;

        extractBool(line, "\"challengeMode\"", challenge);
        extractInt(line, "\"teamSize\"", teamSize);

        if (challenge || teamSize != 1)
            continue;

        extractInt(line, "\"raidTime\"", raidTime);
        extractInt(line, "\"upperTime\"", upperTime);
        extractInt(line, "\"totalPoints\"", totalPoints);

        // Do NOT require upperTime > 0. Raid-data tracker sometimes writes
        // "upperTime": -1 for valid solos; dropping those rows left the
        // newest CoxTimes raids unmatched and desynced the whole join chain
        // (analysis collapsed to ~1 raid). Keep them; match on raidTime only.
        if (raidTime > 0 && totalPoints > 0)
            raids.push_back({ raidTime, upperTime, totalPoints });
    }

    return raids;
}

namespace {
// Prefer raidTime + Floor1/upperTime (±tol). If upperTime is missing/invalid
// (plugin bug), accept raidTime alone so those rows still join.
bool pointsTimesMatch(const PrimaryRaid& p, const PointsRaid& q, int tol)
{
    if (std::abs(p.raidSeconds - q.raidSeconds) > tol)
        return false;

    if (q.upperSeconds <= 0)
        return true;

    return std::abs(p.floor1Seconds - q.upperSeconds) <= tol;
}
}

std::map<int, int> loadPoints(
    const std::string& primaryPath,
    const std::string& pointsPath)
{
    auto primary = loadPrimary(primaryPath);
    auto points = loadPointsFile(pointsPath);

    std::map<int, int> result;

    int i = (int)primary.size() - 1;
    int j = (int)points.size() - 1;

    constexpr int TOL = 3;
    // Limited scan: points log has extras vs CoxTimes. Looking ahead skips
    // those without advancing primary. If still no match, skip primary —
    // never only burn points while stuck on one CoxTimes tip raid (that used
    // to false-match an old row and desync everything after a bad upperTime).
    constexpr int LOOKAHEAD = 30;

    while (i >= 0 && j >= 0)
    {
        const auto& p = primary[i];

        if (pointsTimesMatch(p, points[j], TOL))
        {
            result[p.kc] = points[j].totalPoints;
            --i;
            --j;
            continue;
        }

        int found = -1;
        const int lim = std::max(0, j - LOOKAHEAD);
        for (int k = j - 1; k >= lim; --k)
        {
            if (pointsTimesMatch(p, points[k], TOL))
            {
                found = k;
                break;
            }
        }

        if (found >= 0)
            j = found; // skip intervening points extras, then match next iter
        else
            --i;       // this CoxTimes raid has no nearby points row
    }

    return result;
}

int readMaxCoxKC(const std::string& coxTimesPath)
{
    std::ifstream file(coxTimesPath);
    if (!file.is_open())
        return 0;

    int maxKc = 0;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.rfind("CoX KC:", 0) != 0)
            continue;
        int kc = parseIntWithCommas(line.substr(7));
        if (kc > maxKc)
            maxKc = kc;
    }
    return maxKc;
}

int readMaxCmKC(const std::string& cmTimesPath)
{
    std::ifstream file(cmTimesPath);
    if (!file.is_open())
        return 0;

    int maxKc = 0;
    std::string line;
    while (std::getline(file, line))
    {
        // Cox Analytics labels CM as "CoX CM KC: N"
        if (line.rfind("CoX CM KC:", 0) != 0)
            continue;
        int kc = parseIntWithCommas(line.substr(10));
        if (kc > maxKc)
            maxKc = kc;
    }
    return maxKc;
}

PointsLogStats summarizePointsLog(const std::string& pointsPath)
{
    PointsLogStats stats;
    std::ifstream file(pointsPath);
    if (!file.is_open())
        return stats;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        if (isLeagueProfile(line))
            continue;

        int personalPoints = 0;
        if (!extractInt(line, "\"personalPoints\"", personalPoints) || personalPoints <= 0)
            continue;

        bool challenge = false;
        extractBool(line, "\"challengeMode\"", challenge);

        int teamSize = 0;
        extractInt(line, "\"teamSize\"", teamSize);
        if (teamSize < 1)
            continue;

        stats.sumPersonalAll += personalPoints;

        if (challenge)
        {
            ++stats.nCM;
            stats.sumPersonalCM += personalPoints;
        }
        else if (teamSize == 1)
        {
            ++stats.nSoloRegular;
            stats.sumPersonalSoloRegular += personalPoints;
        }
        else
        {
            ++stats.nTeamRegular;
            stats.sumPersonalTeamRegular += personalPoints;
        }
    }

    return stats;
}

namespace {
constexpr const char* PREP_TIME_KEYS[] = {
    "\"tektonTime\"",
    "\"crabsTime\"",
    "\"iceDemonTime\"",
    "\"shamansTime\"",
    "\"vanguardsTime\"",
    "\"thievingTime\"",
    "\"vespulaTime\"",
    "\"tightropeTime\"",
    "\"guardiansTime\"",
    "\"vasaTime\"",
    "\"mysticsTime\"",
    "\"muttadilesTime\"",
};

// CoxTimes full layout = 12 prep rooms; the tracker log rarely logs all 12
// (typically 11 when full). Treat 11+ logged prep times as full layout.
constexpr int FULL_PREP_ROOMS_IN_LOG = 11;

int countPrepRoomsInLogLine(const std::string& line)
{
    int count = 0;
    for (const char* key : PREP_TIME_KEYS)
    {
        int t = 0;
        if (extractInt(line, key, t) && t > 0)
            ++count;
    }
    return count;
}
}

DeathStats summarizeDeathStats(
    const std::string& pointsPath,
    int thresholdFullRegular,
    int thresholdRegular,
    int thresholdCmSolo,
    int thresholdCmTeam)
{
    DeathStats stats;
    std::ifstream file(pointsPath);
    if (!file.is_open())
        return stats;

    const int fullPrepCount = FULL_PREP_ROOMS_IN_LOG;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || isLeagueProfile(line))
            continue;

        int personalPoints = 0;
        if (!extractInt(line, "\"personalPoints\"", personalPoints) || personalPoints <= 0)
            continue;

        int teamSize = 0;
        extractInt(line, "\"teamSize\"", teamSize);
        if (teamSize < 1)
            continue;

        bool challenge = false;
        extractBool(line, "\"challengeMode\"", challenge);

        if (challenge)
        {
            if (teamSize == 1)
            {
                ++stats.nCmSolo;
                ++stats.total;
                if (personalPoints < thresholdCmSolo)
                {
                    ++stats.deathsCmSolo;
                    ++stats.deaths;
                }
            }
            else
            {
                ++stats.nCmTeam;
                ++stats.total;
                if (personalPoints < thresholdCmTeam)
                {
                    ++stats.deathsCmTeam;
                    ++stats.deaths;
                }
            }
            continue;
        }

        if (teamSize != 1)
            continue;

        const int prepCount = countPrepRoomsInLogLine(line);
        const bool fullLayout = (prepCount >= fullPrepCount);

        if (fullLayout)
        {
            ++stats.nFullRegular;
            ++stats.total;
            if (personalPoints < thresholdFullRegular)
            {
                ++stats.deathsFullRegular;
                ++stats.deaths;
            }
        }
        else
        {
            ++stats.nNormalRegular;
            ++stats.total;
            if (personalPoints < thresholdRegular)
            {
                ++stats.deathsNormalRegular;
                ++stats.deaths;
            }
        }
    }

    return stats;
}

AccountBreakdown buildAccountBreakdown(
    int regularKC,
    int cmKC,
    int nTracked,
    double avgTrackedSoloPoints,
    const PointsLogStats& pointsStats)
{
    AccountBreakdown b;
    b.regularKC = regularKC;
    b.nTracked = nTracked;
    b.nSoloLogged = pointsStats.nSoloRegular;
    b.nTeam = pointsStats.nTeamRegular;
    b.nCM = cmKC;
    b.nCMLogged = pointsStats.nCM;
    b.nCMMissing = std::max(0, cmKC - pointsStats.nCM);
    b.sumPersonalKnown = pointsStats.sumPersonalAll;

    const int attributedRegular = pointsStats.nSoloRegular + pointsStats.nTeamRegular;
    b.nUntracked = std::max(0, regularKC - attributedRegular);

    // Estimate missing CM points from average of logged CM runs (few gaps; fine).
    b.sumPersonalCMEst = pointsStats.sumPersonalCM;
    if (b.nCMMissing > 0 && pointsStats.nCM > 0)
    {
        const double avgCmPts =
            static_cast<double>(pointsStats.sumPersonalCM) / pointsStats.nCM;
        b.sumPersonalCMEst += static_cast<long long>(
            std::llround(avgCmPts * b.nCMMissing));
    }

    // Include estimated CM pts in the "known" total used for expectation later.
    b.sumPersonalKnown += (b.sumPersonalCMEst - pointsStats.sumPersonalCM);

    if (avgTrackedSoloPoints > 0.0 && b.sumPersonalCMEst > 0)
        b.cmEquiv = static_cast<double>(b.sumPersonalCMEst) / avgTrackedSoloPoints;

    return b;
}

PurpleEraAnalysis loadPurpleEraAnalysis(
    const std::string& pointsPath,
    const std::string& primaryUser,
    int rateChangeKc,
    int rateChangeCmKc,
    int regularKc,
    int cmKc)
{
    PurpleEraAnalysis out;

    std::ifstream file(pointsPath);
    if (!file.is_open())
        return out;

    struct Row
    {
        bool challenge = false;
        int personalPoints = 0;
        bool minePurple = false;
        std::string loot; // only set when minePurple
    };

    std::vector<Row> rows;
    std::vector<size_t> regularIdx;
    std::vector<size_t> cmIdx;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || isLeagueProfile(line))
            continue;

        int personalPoints = 0;
        if (!extractInt(line, "\"personalPoints\"", personalPoints) || personalPoints <= 0)
            continue;

        int teamSize = 0;
        extractInt(line, "\"teamSize\"", teamSize);
        if (teamSize < 1)
            continue;

        bool challenge = false;
        extractBool(line, "\"challengeMode\"", challenge);

        Row row;
        row.challenge = challenge;
        row.personalPoints = personalPoints;
        row.minePurple = gotPurpleForUser(line, primaryUser);
        if (row.minePurple)
            extractString(line, "\"specialLoot\":\"", row.loot);

        const size_t idx = rows.size();
        rows.push_back(std::move(row));
        if (challenge)
            cmIdx.push_back(idx);
        else
            regularIdx.push_back(idx);
    }

    int nPostReg = std::max(0, regularKc - rateChangeKc);
    int nPostCm = std::max(0, cmKc - rateChangeCmKc);
    nPostReg = std::min(nPostReg, static_cast<int>(regularIdx.size()));
    nPostCm = std::min(nPostCm, static_cast<int>(cmIdx.size()));

    std::vector<char> isPost(rows.size(), 0);
    for (int i = 0; i < nPostReg; ++i)
        isPost[regularIdx[regularIdx.size() - 1 - i]] = 1;
    for (int i = 0; i < nPostCm; ++i)
        isPost[cmIdx[cmIdx.size() - 1 - i]] = 1;

    auto addItem = [](std::map<std::string, int>& m, const std::string& name)
    {
        if (!name.empty())
            ++m[name];
    };

    for (size_t i = 0; i < rows.size(); ++i)
    {
        const Row& r = rows[i];
        const bool post = isPost[i] != 0;

        if (r.challenge)
        {
            ++out.historyAll.cmRaids;
            if (r.minePurple)
                ++out.historyAll.cmPurples;
        }
        out.historyAll.hasPurple.push_back(r.minePurple);

        if (!post)
            continue;

        if (r.challenge)
        {
            ++out.nPostCm;
            out.pointsPostCm += r.personalPoints;
            ++out.historyPost.cmRaids;
            if (r.minePurple)
            {
                ++out.purplesPostCm;
                ++out.historyPost.cmPurples;
                addItem(out.itemsPostCm, r.loot);
                addItem(out.itemsPost, r.loot);
            }
        }
        else
        {
            ++out.nPostRegular;
            out.pointsPostRegular += r.personalPoints;
            if (r.minePurple)
            {
                ++out.purplesPostRegular;
                addItem(out.itemsPostRegular, r.loot);
                addItem(out.itemsPost, r.loot);
            }
        }
        out.historyPost.hasPurple.push_back(r.minePurple);
    }

    return out;
}