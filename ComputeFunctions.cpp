#include <iomanip>
#include <sstream>
#include <algorithm>
#include <climits>

#include "ComputeFunctions.h"
#include "Types.h"

std::string secondsToTime(int seconds) {
    if (seconds <= 0) return "--:--";
    int m = seconds / 60;
    int s = seconds % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << m << ":"
        << std::setw(2) << std::setfill('0') << s;
    return oss.str();
}

void processStats(std::map<std::string, Stats>& stats, const std::string& key, const std::vector<Raid>& raids, size_t start) {
    auto& s = stats[key];

    // 1) Collect raw time entries
    for (size_t i = start; i < raids.size(); ++i) {
        const auto& r = raids[i];
        int val = 0;
        bool has = false;

        if (key == "Pre-Olm" || key == "Between room time") {
            int prep = 0;
            int olm = r.times.count("Olm") ? r.times.at("Olm") : 0;
            int total = r.times.at("Raid Completed");

            for (const auto& room : PREP_ROOMS) {
                if (r.times.count(room)) prep += r.times.at(room);
            }

            val = (key == "Pre-Olm") ? prep : total - prep - olm;
            has = true;
        }
        else if (r.times.count(key)) {
            val = r.times.at(key);
            has = true;
        }

        if (has) s.entries.push_back({ r.kc, val });
    }

    size_t n = s.entries.size();
    if (n == 0) return;

    // 2) Detect and mark invalid / outlier entries (do not erase yet)
    std::vector<bool> keep(n, true);

    for (size_t j = 0; j < n; ++j) {
        int t = s.entries[j].value;
        if (ROOM_REFERENCE_FOR_OUTLIERS.contains(key))
        {
            if (t < ROOM_REFERENCE_FOR_OUTLIERS.at(key)) {
                keep[j] = false;
                s.discarded.emplace_back(s.entries[j].kc, key, t, "too short " + std::to_string(ROOM_REFERENCE_FOR_OUTLIERS.at(key)));
            }
        }
        else if (t < 20) {
            keep[j] = false;
            s.discarded.emplace_back(s.entries[j].kc, key, t, "too short (<20s)");
        }
        if (t > 240 && std::find(PREP_ROOMS.begin(), PREP_ROOMS.end(), key) != PREP_ROOMS.end()) {
            keep[j] = false;
            s.discarded.emplace_back(s.entries[j].kc, key, t, ">4 min hard cap");
        }
        if (key == "Between room time" && (t > 600)) {
            keep[j] = false;
            s.discarded.emplace_back(s.entries[j].kc, key, t, "invalid between time");
        }
    }

    // 3) Aggregate valid entries (average, fastest, count)
    double sum = 0.0;
    int best = INT_MAX;
    s.validCount = 0;

    for (size_t j = 0; j < n; ++j) {
        if (keep[j]) {
            int val = s.entries[j].value;
            sum += val;
            s.validCount++;
            if (val < best) best = val;
        }
    }

    if (s.validCount > 0) {
        s.avg = sum / s.validCount;
        s.fastest = best;
    }
}



