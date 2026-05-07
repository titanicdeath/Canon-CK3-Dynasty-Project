#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <psapi.h>

#include <iostream>
#include <iomanip>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "../include/memory_utill.hpp"
#include "../include/string_utill.hpp"

using namespace std;

// Format a byte count into a vector of human-readable strings, one per
// magnitude tier. byteFormat(1500) -> ["1,500 bytes", "1 kilobytes", ...]
// Index 0 = bytes, 1 = KB, 2 = MB, 3 = GB.
vector<string> byteFormat(std::size_t bytes) {
    std::size_t place[4] = { bytes, 0, 0, 0 };
    static const char* byteType[4] = { "bytes", "kilobytes", "megabytes", "gigabytes" };

    for (std::size_t i = 0; i + 1 < 4; i++) {
        if (place[i] >= 1024) {
            place[i + 1] = place[i] / 1024;
        } else {
            break;
        }
    }

    vector<string> result;
    result.reserve(4);
    for (std::size_t i = 0; i < 4; i++) {
        result.push_back(formatWithCommas(to_string(place[i])) + " " + byteType[i]);
    }
    return result;
}

// Get current process memory usage (WorkingSetSize) in bytes.
std::size_t getMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    throw runtime_error("GetProcessMemoryInfo failed");
}

// Two behaviors controlled by devMsg:
//   devMsg == false  -> print the full memory table (used at end of run).
//                       Returns empty string.
//   devMsg == true   -> look up the human-readable label for a logged
//                       operation step. Returns the label, or empty string
//                       if the step number isn't in the table.
string memoryLogging(const vector<vector<string>>& memorytable, bool devMsg, int option) {
    static const map<int, string> verification = {
        { 1,  "|   [1]   |   Before loading the file." },
        { 2,  "|   [2]   |   After loading the file." },
        { 3,  "|   [3]   |   After splitting lines" },
        { 4,  "|   [4]   |   After filtering other blocks" },
        { 5,  "|   [5]   |   After building new filtered buffer" },
        { 6,  "|   [6]   |   After clearing old buffer" },
        { 7,  "|   [7]   |   After building filter blocks" },
        { 8,  "|   [8]   |   After clearing buffer" },
        { 9,  "|   [9]   |   After mapping blocks" },
        { 10, "[10]  |  Record Memory Usage | After Clearing unorganized blocks" },
        { 11, "[11]  |  Record Memory Usage | After sorting founder descendants/spouses" },
        { 12, "[12]  |  Record Memory Usage | After Clearing unrelated blocks" }
    };

    if (!devMsg) {
        // Print the full table.
        for (std::size_t i = 0; i < memorytable.size(); i++) {
            const vector<string>& row = memorytable[i];
            cout << "\n|   [" << (i + 1) << "]   |   "
                 << setw(20) << row[2] << "   |   ";
        }
        cout << "\n";
        return "";
    }

    // Look up the label for the given step.
    auto it = verification.find(option);
    return (it != verification.end()) ? it->second : string();
}

// Record current memory usage and optionally print spot/change info.
void recordMemoryUsage(vector<vector<string>>& memoryTable,
                       vector<std::size_t>& memoryValues,
                       bool showSpotLog, bool showChangeLog, int msg) {
    std::size_t current = getMemoryUsage();
    memoryValues.push_back(current);

    auto usageStrings = byteFormat(current);
    memoryTable.push_back(usageStrings);

    if (showSpotLog) {
        // usageStrings[2] is the megabytes tier.
        cout << "[DEV] Current Program Memory: " << usageStrings[2] << "\n";
    }

    if (showChangeLog && memoryValues.size() > 1) {
        std::size_t prev = memoryValues[memoryValues.size() - 2];
        std::size_t delta = current > prev ? current - prev : prev - current;
        auto deltaStrings = byteFormat(delta);
        char sign = (current >= prev) ? '+' : '-';
        cout << "[DEV] Program Memory Change: " << sign << deltaStrings[1] << "\n";
        cout << "[DEV] Operation: " << memoryLogging(memoryTable, true, msg) << "\n\n";
    }
}