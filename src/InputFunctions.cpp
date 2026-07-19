#include <iostream>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cctype>
#include <cstring>

#include "InputFunctions.h"
#include "PointsLoader.h"


std::string getUsername(const std::string& path) {
    std::filesystem::path p(path);
    std::string name = p.filename().stem().string();
    size_t pos = name.find("_CoxTimes");
    if (pos != std::string::npos) name = name.substr(0, pos);
    std::replace(name.begin(), name.end(), '_', ' ');
    return name;
}

bool readRaids(const std::string& filename, std::vector<Raid>& raids) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << "\n";
        return false;
    }

    raids.clear();
    std::map<std::string, int> currentTimes;
    int currentKC = 0;
    std::string line;
    bool validRaid = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        size_t kcPos = line.find("KC");
        if (kcPos != std::string::npos)
        {
            size_t pos = kcPos + 2;

            // skip until digit
            // Cast exists to protect against negative char values
            while (pos < line.size() && !std::isdigit(static_cast<unsigned char>(line[pos])))
                ++pos;

            std::string num;
            while (pos < line.size() && (std::isdigit(static_cast<unsigned char>(line[pos])) || line[pos] == ','))
            {
                if (line[pos] != ',')
                    num += line[pos];
                ++pos;
            }
            if (!num.empty())
                currentKC = std::stoi(num);
        }

        if (line.find("---") != std::string::npos) {
            if (validRaid && !currentTimes.empty()) {
                //std::cout << "Debug: Adding raid KC " << currentKC << "\n";  // Debug line
                raids.push_back({ currentKC, currentTimes });
            }
            currentTimes.clear();
            currentKC = 0;
            validRaid = false;
            continue;
        }

        if (line.find("Raid Completed:") != std::string::npos) {
            if (line.find("Team Size: " + std::to_string(1)) != std::string::npos) {
                validRaid = true;
                size_t pos = line.find("Raid Completed: ") + 16;
                size_t endPos = line.find(" |", pos);
                if (endPos == std::string::npos) endPos = line.size();
                std::string timeStr = line.substr(pos, endPos - pos);
                int seconds = 0;
                size_t colon = timeStr.find(':');
                if (colon != std::string::npos) {
                    try {
                        seconds = std::stoi(timeStr.substr(0, colon)) * 60 + std::stoi(timeStr.substr(colon + 1));
                    }
                    catch (...) {}
                }
                if (seconds > 0) currentTimes["Raid Completed"] = seconds;
            }
            continue;
        }

        size_t colon = line.find(':');
        if (colon != std::string::npos && colon + 1 < line.size()) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 2);
            int seconds = 0;
            size_t c2 = val.find(':');
            if (c2 != std::string::npos) {
                try {
                    seconds = std::stoi(val.substr(0, c2)) * 60 + std::stoi(val.substr(c2 + 1));
                }
                catch (...) {}
            }
            if (seconds > 0) currentTimes[key] = seconds;
        }
    }

    return !raids.empty();
}

PurpleHistory loadPurpleHistory(
    const std::string& logFile,
    const std::string& primaryUser)
{
    PurpleHistory hist;

    std::ifstream file(logFile);
    if (!file.is_open())
        return hist;

    auto gotPurpleForUser = [&](const std::string& line) -> bool
    {
        bool hasLoot = false;
        auto lootPos = line.find("\"specialLoot\":\"");
        if (lootPos != std::string::npos)
        {
            size_t start = lootPos + 15;
            size_t end = line.find('"', start);
            if (end != std::string::npos)
                hasLoot = (end > start);
        }
        if (!hasLoot)
            return false;

        constexpr const char* RECEIVER_KEY = "\"specialLootReceiver\":\"";
        auto recvPos = line.find(RECEIVER_KEY);
        if (recvPos == std::string::npos)
            return true; // loot present, no receiver field

        size_t start = recvPos + std::strlen(RECEIVER_KEY);
        size_t end = line.find('"', start);
        if (end == std::string::npos)
            return false;

        std::string receiver = line.substr(start, end - start);
        return receiver.empty() || receiver == primaryUser;
    };

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

        int teamSize = 0;
        extractInt(line, "\"teamSize\"", teamSize);
        if (teamSize < 1)
            continue;

        bool challenge = false;
        extractBool(line, "\"challengeMode\"", challenge);

        const bool mine = gotPurpleForUser(line);

        if (challenge)
        {
            ++hist.cmRaids;
            if (mine)
                ++hist.cmPurples;
        }

        // Chronological map: solo + team + CM as they appear in the log
        hist.hasPurple.push_back(mine);
    }

    return hist;
}

