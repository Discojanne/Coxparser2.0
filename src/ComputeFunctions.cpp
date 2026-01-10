#include <iomanip>
#include <sstream>
#include <algorithm>
#include <climits>

#include "ComputeFunctions.h"
#include "PrintFunctions.h"

std::string secondsToTime(int seconds)
{
    if (seconds <= 0) return "00:00";
    int m = seconds / 60;
    int s = seconds % 60;
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << m << ":"
        << std::setw(2) << std::setfill('0') << s;
    return oss.str();
}

void processStats(std::map<std::string, Stats>& stats, const std::string& key, const std::vector<Raid>& raids, size_t start)
{
    auto& s = stats[key];

    double sum = 0.0;
    int best = INT_MAX;

    const bool hasMinRef = ROOM_REFERENCE_FOR_OUTLIERS.count(key);
    const bool hasMaxRef = ROOM_MAX_REFERENCE.count(key);

    const int minRef = hasMinRef ? ROOM_REFERENCE_FOR_OUTLIERS.at(key) : 0;
    const int maxRef = hasMaxRef ? ROOM_MAX_REFERENCE.at(key) : INT_MAX;

    for (size_t i = start; i < raids.size(); ++i)
    {
        const auto& r = raids[i];
        auto it = r.times.find(key);
        if (it == r.times.end())
            continue;

        int t = it->second;
        bool valid = true;
        std::string reason;

        // --- too short ---
        if (t < 20)
        {
            valid = false;
            reason = "<20s";
        }
        else if (hasMinRef && t < minRef)
        {
            valid = false;
            reason = "below min";
        }
        // --- too long ---
        else if (hasMaxRef && t > maxRef)
        {
            valid = false;
            reason = "above max";
        }

        if (!valid)
        {
            s.discarded.emplace_back(r.kc, key, t, reason);
            continue;
        }

        // valid entry
        s.entries.push_back({ r.kc, t });
        sum += t;
        best = std::min(best, t);
        ++s.validCount;
    }

    if (s.validCount > 0)
    {
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

void aggregateStats(std::map<std::string, Stats>& stats, const std::vector<Raid>& raids, size_t start)
{
    for (const auto& k : DISPLAY_ORDER) {
        processStats(stats, k, raids, start);
    }
}

std::map<std::string, int> computeRecentRaidTimes(const std::vector<Raid>& raids)
{
    std::map<std::string, int> recentVal;
    const auto& rr = raids.back();

    for (const auto& [k, v] : rr.times)
        recentVal[k] = v;

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
    const int columnCount = hasSecondary ? 6 : 5;
    const int totalCols =
        NW + TW + AW + RW + LW +
        (hasSecondary ? CW : 0);

    return totalCols + (columnCount - 1) * SEP;
}


void attachPointsToRaids(std::vector<Raid>& raids, const std::map<int, int>& pointsMap)
{
    for (auto& r : raids)
    {
        auto it = pointsMap.find(r.kc);
        if (it != pointsMap.end())
        {
            r.totalPoints = it->second;
        }
    }
}

void filterRaidsWithPoints(std::vector<Raid>& raids)
{
    raids.erase(
        std::remove_if(
            raids.begin(),
            raids.end(),
            [](const Raid& r)
            {
                return r.totalPoints < 0;
            }),
        raids.end());
}

void keepMostRecentRaids(std::vector<Raid>& raids, int maxCount)
{
    if (maxCount < 0 || raids.size() <= static_cast<size_t>(maxCount))
        return;

    raids.erase(
        raids.begin(),
        raids.end() - maxCount);
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

PointsToPrint makePointsToPrint(int best, int average, int recent)
{
    PointsToPrint p{};
    p.best = best;
    p.average = average;
    p.recent = recent;
    p.recentDiff = recent - average;
    return p;
}

std::vector<RoomPPHResult> computeRoomPPH(const std::vector<Raid>& raids)
{
    std::map<std::string, RoomPPHStats> acc;

    for (const auto& r : raids)
    {
        if (r.totalPoints <= 0 || r.totalSeconds <= 0)
            continue;

        for (const auto& room : PREP_ROOMS)
        {
            auto it = r.times.find(room);
            if (it == r.times.end())
                continue;

            acc[room].raids++;
            double share = static_cast<double>(it->second) / r.totalSeconds;
            acc[room].totalPoints += r.totalPoints * share;

            acc[room].totalSeconds += it->second;
        }
    }

    std::vector<RoomPPHResult> result;
    result.reserve(acc.size());

    for (const auto& [room, stats] : acc)
    {
        if (stats.totalSeconds <= 0)
            continue;

        int avgPPH = static_cast<int>(
            stats.totalPoints / (stats.totalSeconds / 3600.0)
            );

        result.push_back({
            room,
            avgPPH,
            stats.raids
            });
    }

    std::sort(result.begin(), result.end(),
        [](const auto& a, const auto& b)
        {
            return a.avgPPH > b.avgPPH;
        });

    return result;
}


double computeLastNTimeAvg(const std::vector<Raid>& raids, const std::string& key, int N)
{
    if (raids.empty())
        return 0.0;

    int start = std::max(0, (int)raids.size() - N);
    double sum = 0.0;
    int count = 0;

    for (int i = start; i < (int)raids.size(); ++i)
    {
        auto it = raids[i].times.find(key);
        if (it != raids[i].times.end() && it->second > 0)
        {
            sum += it->second;
            ++count;
        }
    }

    return (count > 0) ? (sum / count) : 0.0;
}

double computeLastNPPH(const std::vector<Raid>& raids, int N)
{
    int start = std::max(0, (int)raids.size() - N);
    double sum = 0.0;
    int count = 0;

    for (int i = start; i < (int)raids.size(); ++i)
    {
        const auto& r = raids[i];
        if (r.totalPoints > 0 && r.totalSeconds > 0)
        {
            sum += r.totalPoints / (r.totalSeconds / 3600.0);
            ++count;
        }
    }

    return (count > 0) ? (sum / count) : 0.0;
}

double computeLastNPoints(const std::vector<Raid>& raids, int N)
{
    double sum = 0.0;
    int count = 0;

    for (int i = static_cast<int>(raids.size()) - 1;
        i >= 0 && count < N;
        --i)
    {
        if (raids[i].totalPoints > 0)
        {
            sum += raids[i].totalPoints;
            ++count;
        }
    }

    return (count > 0) ? (sum / count) : 0.0;
}

void finalizeDerivedRaidTimes(std::vector<Raid>& raids)
{
    for (auto& r : raids)
    {
        int prep = 0;

        for (const auto& room : PREP_ROOMS)
        {
            auto it = r.times.find(room);
            if (it != r.times.end())
                prep += it->second;
        }

        int total = r.times.count("Raid Completed")
            ? r.times.at("Raid Completed")
            : 0;

        int olm = r.times.count("Olm")
            ? r.times.at("Olm")
            : 0;

        r.totalSeconds = total;

        if (prep > 0)
            r.times["Pre-Olm"] = prep;

        if (total > 0 && prep > 0 && olm > 0)
            r.times["Between room time"] = total - prep - olm;
    }
}

std::map<std::string, double> computeLastNStats(const std::vector<Raid>& raids, int lastN)
{
    std::map<std::string, double> result;

    // Time-based rows
    for (const auto& key : DISPLAY_ORDER)
    {
        if (key == "Total Points" || key == "PPH")
            continue;

        result[key] = computeLastNTimeAvg(raids, key, lastN);
    }

    // Points-based rows
    result["Total Points"] = computeLastNPoints(raids, lastN);
    result["PPH"] = computeLastNPPH(raids, lastN);

    return result;
}

int countPrepRooms(const Raid& r)
{
    int count = 0;
    for (const auto& room : PREP_ROOMS)
        if (r.times.count(room))
            ++count;
    return count;
}

void filterByLayout(std::vector<Raid>& raids, LayoutFilter mode)
{
    if (mode == LayoutFilter::All)
        return;

    raids.erase(
        std::remove_if(raids.begin(), raids.end(),
            [&](const Raid& r)
            {
                int prepCount = countPrepRooms(r);

                if (mode == LayoutFilter::NormalOnly)
                    return prepCount >= static_cast<int>(PREP_ROOMS.size());

                if (mode == LayoutFilter::FullOnly)
                    return prepCount < static_cast<int>(PREP_ROOMS.size());

                return false;
            }),
        raids.end());
}



