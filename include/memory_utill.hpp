#ifndef MEMORY_UTILL_HPP
#define MEMORY_UTILL_HPP

#include <iomanip>
#include <cstddef>
#include <string>
#include <vector>

using namespace std;

vector<string> byteFormat(std::size_t bytes);
std::size_t getMemoryUsage();

string memoryLogging(vector<vector<string>> memorytable, bool devMsg = false, int option = 0);
void recordMemoryUsage(vector<vector<string>> &memoryTable, vector<std::size_t> &memoryValues, bool showSpotLog = false, bool showChangeLog = false, int msg = 0);

#endif