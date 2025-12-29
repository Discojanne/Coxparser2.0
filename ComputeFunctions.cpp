#include <iomanip>
#include <sstream>
#include <algorithm>
#include <climits>

#include "ComputeFunctions.h"
#include "PrintFunctions.h"
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

std::map<std::string, Stats> initializeStats()
{
    std::map<std::string, Stats> stats;
    for (const auto& k : DISPLAY_ORDER) {
        stats[k];
    }
	return stats;
}

void computeAllStats(std::map<std::string, Stats>& stats, const std::vector<Raid>& raids, size_t start)
{
    for (const auto& k : DISPLAY_ORDER) {
        processStats(stats, k, raids, start);
    }
}

std::map<std::string, int> computeRecentRaidTimes(const std::vector<Raid>& raids)
{
    std::map<std::string, int> recentVal;

    const auto& rr = raids.back();
    int prep = 0;
    int olm = rr.times.count("Olm") ? rr.times.at("Olm") : 0;
    int total = rr.times.at("Raid Completed");

    for (const auto& room : PREP_ROOMS) {
        if (rr.times.count(room)) prep += rr.times.at(room);
    }

    recentVal["Pre-Olm"] = prep;
    recentVal["Olm"] = olm;
    recentVal["Raid Completed"] = total;
    recentVal["Between room time"] = total - prep - olm;
    for (const auto& [k, v] : rr.times) recentVal[k] = v;

    return recentVal;
}

RoomDistribution computeRoomDistribution(const std::vector<Raid>& raids)
{
    RoomDistribution rd;

    for (size_t i = 0; i < raids.size(); ++i) {
        const auto& r = raids[i];
        int count = 0;
        for (const auto& room : PREP_ROOMS) {
            if (r.times.count(room)) ++count;
        }
        if (count == 5) ++rd.five;
        else if (count == 6) ++rd.six;
        else ++rd.other;
    }

    return rd;
}

int computeCountPad(const std::map<std::string, Stats>& stats)
{
    // Determine width for prep-room count column (for table alignment)
    int maxPrepCount = 0;
    for (const auto& room : PREP_ROOMS) {
        maxPrepCount = std::max(maxPrepCount, stats.at(room).validCount);
    }
    return (maxPrepCount == 0) ? 1 : static_cast<int>(std::to_string(maxPrepCount).length());
}

std::vector<std::tuple<int, std::string, int, std::string>> collectAndSortDiscarded(const std::map<std::string, Stats>& stats)
{
	std::vector<std::tuple<int, std::string, int, std::string>> discarded;

    for (const auto& pair : stats) {
        const auto& st = pair.second;
        for (const auto& d : st.discarded) discarded.push_back(d);
    }

    std::sort(discarded.rbegin(), discarded.rend());
    return discarded;
}

std::vector<std::pair<std::string, const Stats*>> computeMostCommonRooms(const std::map<std::string, Stats>& stats)
{
	std::vector<std::pair<std::string, const Stats*>> common;
    for (const auto& room : PREP_ROOMS) {
        const auto& st = stats.at(room);
        if (st.validCount > 0) common.emplace_back(room, &st);
    }
    std::sort(common.begin(), common.end(), [](const auto& a, const auto& b) {
        return a.second->validCount > b.second->validCount ||
            (a.second->validCount == b.second->validCount && a.first < b.first);
        });

    return common;
}

int computeTotalWidth(bool hasSecondary)
{
    return 2 + NW + TW + AW + RW + (!hasSecondary ? 0 : CW) + SEP * 4;
}

void mapPointsToRaids(std::vector<Raid>& raids, const std::map<int, int>& pointsMap, bool deleteIfNoScore, int numRecent)
{
    // First pass: assign totals for raids that have a points entry.
    for (auto& r : raids) {
        auto it = pointsMap.find(r.kc);
        if (it != pointsMap.end()) {
            r.totalPoints = it->second;
        }
        else {
            // Explicitly mark missing score (optional; keeps original default -1 if preferred)
            r.totalPoints = -1;
        }
        auto tt = r.times.find("Raid Completed");
        if (tt != r.times.end()) r.totalSeconds = tt->second;
        else r.totalSeconds = 0; // fallback if the time key is missing
    }
    // Second pass: remove raids without a score if requested.
    if (deleteIfNoScore) {
        std::erase_if(raids, [&](const Raid& r) {
            return r.totalPoints < 0;
            });
    }
    // Third pass: keep only the numRecent most recent raids (assuming higher kc is more recent)
    if (numRecent > 0) {
        std::sort(raids.begin(), raids.end(), [](const Raid& a, const Raid& b) {
            return a.kc > b.kc;
            });
        if (raids.size() > static_cast<size_t>(numRecent)) {
            raids.erase(raids.begin() + numRecent, raids.end());
        }
        // Restore ascending order (increasing KC) to maintain original assumptions
        std::sort(raids.begin(), raids.end(), [](const Raid& a, const Raid& b) {
            return a.kc < b.kc;
            });
    }
}

std::string formatRecentValue(double recent, double average, bool useSeconds, bool higherIsBetter)
{
    if (recent <= 0 || average <= 0)
        return "-          -";

    double diff = recent - average;
    double absDiff = std::abs(diff);

    std::string sign = " ";
    std::string col = COLOR_RESET;

    if (absDiff >= (useSeconds ? 0.5 : 1.0)) {
        sign = (diff < 0 ? "-" : "+");

        bool better = higherIsBetter ? (diff > 0) : (diff < 0);
        col = better ? COLOR_GREEN : COLOR_RED;
    }

    std::string valStr = useSeconds
        ? secondsToTime(static_cast<int>(std::round(recent)))
        : std::to_string(static_cast<int>(std::round(recent)));

    std::string diffStr = useSeconds
        ? secondsToTime(static_cast<int>(std::round(absDiff)))
        : std::to_string(static_cast<int>(std::round(absDiff)));

    return valStr + " " + col + sign + diffStr + COLOR_RESET;
}

int visibleLength(const std::string& s)
{
    int len = 0;
    bool inEscape = false;

    for (char c : s) {
        if (c == '\x1b')
            inEscape = true;
        else if (inEscape && c == 'm')
            inEscape = false;
        else if (!inEscape)
            ++len;
    }
    return len;
}

PointsAggregate computePointsStats(const std::vector<Raid>& raids)
{
    PointsAggregate out;

    double sumPPH = 0.0;
    double sumPoints = 0.0;
    int countPPH = 0;
    int countPoints = 0;

    for (const auto& r : raids)
    {
        if (r.totalPoints > 0)
        {
            sumPoints += r.totalPoints;
            ++countPoints;
            out.bestPoints = std::max(out.bestPoints, r.totalPoints);
        }

        if (r.totalPoints > 0 && r.totalSeconds > 0)
        {
            double pph = r.totalPoints / (r.totalSeconds / 3600.0);
            sumPPH += pph;
            ++countPPH;
            out.bestPPH = std::max(out.bestPPH, static_cast<int>(pph));
        }
    }

    if (countPoints > 0)
        out.avgPoints = static_cast<int>(sumPoints / countPoints);

    if (countPPH > 0)
        out.avgPPH = static_cast<int>(sumPPH / countPPH);

    // recent
    if (!raids.empty())
    {
        const auto& r = raids.back();
        if (r.totalPoints > 0 && r.totalSeconds > 0)
            out.recentPPH = static_cast<int>(r.totalPoints / (r.totalSeconds / 3600.0));
    }

    return out;
}

std::vector<RoomPPHResult>computeRoomPPH(const std::vector<Raid>& raids)
{
    std::map<std::string, RoomPPHStats> acc;

    for (const auto& r : raids)
    {
        if (r.totalPoints <= 0 || r.totalSeconds <= 0)
            continue;

        double raidPPH = r.totalPoints / (r.totalSeconds / 3600.0);

        for (const auto& [room, time] : r.times)
        {
            if (!isPrepRoom(room))
                continue;

            acc[room].sumPPH += raidPPH;
            acc[room].count++;
        }
    }

    std::vector<RoomPPHResult> result;

    for (const auto& [room, stats] : acc)
    {
        if (stats.count < 5)   // avoid noise from rare rooms
            continue;

        result.push_back({
            room,
            static_cast<int>(std::round(stats.sumPPH / stats.count)),
            stats.count
            });
    }

    std::sort(result.begin(), result.end(),
        [](const RoomPPHResult& a, const RoomPPHResult& b) {
            return a.avgPPH > b.avgPPH;
        });

    return result;
}




