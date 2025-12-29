#include "PointsLoader.h"

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

        if (raidTime > 0 && upperTime > 0 && totalPoints > 0)
            raids.push_back({ raidTime, upperTime, totalPoints });
    }

    return raids;
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

    while (i >= 0 && j >= 0)
    {
        const auto& p = primary[i];
        const auto& q = points[j];

        bool raidMatch = std::abs(p.raidSeconds - q.raidSeconds) <= TOL;
        bool floorMatch = std::abs(p.floor1Seconds - q.upperSeconds) <= TOL;

        if (raidMatch && floorMatch)
        {
            result[p.kc] = q.totalPoints;
            /*std::cout << "Matched KC " << p.kc
                << " | raid " << p.raidSeconds
                << " | floor1 " << p.floor1Seconds
                << " | points " << q.totalPoints << "\n";*/
            --i;
            --j;
        }
        else
        {
            // points file has extra CM runs → skip it
            --j;
        }
    }

    return result;
}