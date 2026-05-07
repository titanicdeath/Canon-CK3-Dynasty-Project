#ifndef STRING_UTIL_cPP
#define STRING_UTIL_CPP
#include <windows.h>
#include <psapi.h>
#include <stdexcept>
#include <sstream>   // if needed
#include <vector>
#include <string>

#include "../library/string_utill.hpp"

using namespace std;

string formatWithCommas(string numStr) {
    string result;
    int count = 0;
    // Iterate backwards over the number string.
    for (int i = numStr.size() - 1; i >= 0; i--) {
        result.insert(result.begin(), numStr[i]);
        count++;
        // Insert a comma every three digits, unless at the beginning.
        if (count % 3 == 0 && i != 0) {
            result.insert(result.begin(), ',');
        }
    }
    return result;
}

// trim leading/trailing whitespace
string trim(const string &s) {
    // Find first non-whitespace
    std::size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start])) {
        start++;
    }
    // Find last non-whitespace
    std::size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end - 1])) {
        end--;
    }
    return s.substr(start, end - start);
}

#endif 
