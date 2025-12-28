#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <tuple>
#include <fstream>
#include <filesystem>

#include "InputFunctions.h"

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