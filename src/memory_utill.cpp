#ifndef MEMORY_UTILL_CPP
#define MEMORY_UTILL_CPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif


#include <windows.h>
#include <psapi.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>
#include <iostream>

#include "../library/memory_utill.hpp"
#include "../library/string_utill.hpp"

using namespace std;

// #define WIN32_LEAN_AND_MEAN
// #define NOMINMAX


vector<string> byteFormat(std::size_t bytes){
    std::size_t operation = 0;
    std::size_t place[5] = {bytes, 0, 0, 0, 0};
    string byteType[5] = {"bytes", "kilobytes", "megabytes", "gigabytes", "petabytes"};\
    vector<string> byteValueType;
    for (std::size_t i = 0; i < 4; i++){
        if (place[i] >= 1024) {
            operation = place[i];
            operation = operation / 1024;
            place[i+1] = operation;
        }
        else {
            break;
        }
    }
    string formattedValues[5];
    string current, running = "";
    for (std::size_t i = 0; i < 4; i++){
        current = to_string(place[i]);
        current = formatWithCommas(current);
        formattedValues[i] = current;
    }
    string formatted = "";
    //cout << setw(20) << right;
    for (std::size_t i = 0; i < 4; i++){
        formatted = formattedValues[i] + " " + byteType[i]; 
        byteValueType.push_back(formatted);
    }
    return byteValueType;
}

// Helper function: Get current memory usage (WorkingSetSize) in bytes.
std::size_t getMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        std::size_t memory = pmc.WorkingSetSize;
        //cout << format << endl;
        return memory; // in bytes
    }
    throw runtime_error("GetProcessMemoryInfo failed");
}


/*
########################
OTHER MEMORY FUNCTIONS
########################
*/

string memoryLogging(vector<vector<string>> memorytable, bool devMsg, int option){
    string msg = "";
    vector<string> row;
    map<int, string> verification = {
        {1, "|   [1]   |   Before loading the file."},
        {2, "|   [2]   |   After loading the file."},
        {3, "|   [3]   |   After splitting lines"},
        {4, "|   [4]   |   After filtering other blocks"},
        {5, "|   [5]   |   After building new filtered buffer"},
        {6, "|   [6]   |   After clearing old buffer"},
        {7, "|   [7]   |   After building filter blocks"},
        {8, "|   [8]   |   After clearing buffer"},
        {9, "|   [9]   |   After mapping blocks"},
        {10, "[10]  |  Record Memory Usage | After Clearing unorganized blocks"},
        {11, "[11]  |  Record Memory Usage | After sorting founder descendants/spouses"},
        {12, "[12]  |  Record Memory Usage | After Clearing unrelated blocks"}
    };
    if (devMsg == false){
        for (int i = 0; i < (memorytable.size()); i++) {
            row = memorytable[i];
            cout << "\n|   [" << i + 1 << "]   |   ";
            cout << setw(20) << row[2] << "   |   ";
        }
        return "sorry c++";
    }
    else if (devMsg == true) {
        msg = verification[option];
        return msg;
    }
    
}

// A simple function to record memory usage into a table
void recordMemoryUsage(vector<vector<string>> &memoryTable, vector<std::size_t> &memoryValues, bool showSpotLog, bool showChangeLog, int msg) {
    // 1) Get current usage
    std::size_t current = getMemoryUsage();
    memoryValues.push_back(current);

    // 2) Push it into the table as well
    // For example, the "byteFormat" function returns a vector<string>.
    auto usageStrings = byteFormat(current);
    memoryTable.push_back(usageStrings);
    // 
    

    // Optionally show the user the spot usage
    if (showSpotLog) {
    // usageStrings[1] might be the "kilobytes" string if that’s how you set up byteFormat
    cout << "[DEV] Current Program Memory: " << usageStrings[2] << "\n";
    }

    // If there's a previous measurement, show the difference
    if (showChangeLog && memoryValues.size() > 1) {
    std::size_t prev = memoryValues[memoryValues.size() - 2];
    std::size_t delta = current > prev ? current - prev : prev - current;
    auto deltaStrings = byteFormat(delta);
    char sign = (current >= prev) ? '+' : '-';
    cout << "[DEV] Program Memory Change: " << sign << deltaStrings[1] << "\n";
    cout << "[DEV] Operation: " << memoryLogging(memoryTable, true, msg)<< "\n\n";
    }
}



// #define STRING_MEMORY_LOG memorytable.push_back(byteFormat(getMemoryUsage()));
// #define T_SIZE_MEMORY_LOG memoryValues.push_back(getMemoryUsage());
// #define FINAL_INDEX memoryValues.size() - 1
// #define INCREA_MEMORY_LOG memorytable.push_back(byteFormat(memoryValues[FINAL_INDEX] - memoryValues[FINAL_INDEX-1]));
// #define DECREA_MEMORY_LOG memorytable.push_back(byteFormat(memoryValues[FINAL_INDEX-1] - memoryValues[FINAL_INDEX]));
// #define MEM_CUR return ;

// void memoryDefineCall(vector<vector<string>> &memorytable, vector<std::size_t> &memoryValues, bool spotLog, bool changeLog){
//     string flag = "";
//     STRING_MEMORY_LOG;
//     T_SIZE_MEMORY_LOG;

//     //cout << "\n\n[]\n" << memoryValues[FINAL_INDEX] << " - " << memoryValues[FINAL_INDEX-1] << " = " << memoryValues[FINAL_INDEX] - memoryValues[FINAL_INDEX-1] << endl;

//     if (spotLog == true){
//         vector<string> spotLast = memorytable[memorytable.size()-3];
//         cout << "[DEV] Current Program Memory Allocation:   " << spotLast[spotLast.size()-1] << endl;
//     }
//     if ((memoryValues[FINAL_INDEX] - memoryValues[FINAL_INDEX-1]) > 0) {
//         flag = "+";
//         INCREA_MEMORY_LOG;
//     }
//     else{
//         flag = "-";
//         DECREA_MEMORY_LOG;
//     }
//     if (changeLog == true){
//         std::size_t change  = memoryValues[memoryValues.size()-1];
//         vector<string> format = byteFormat(change);
//         cout << "[DEV] Program Memory  Change:    " << flag << format[1] << endl;
//     }
    
    

// } 

#endif