#include <iostream>
#include <map>
#include <string>
#include <string_view>

using namespace std;

const string_view* pointerLexicon(char ch) {
    // Static verification table: char -> int
    static const map<char, int> verification = {
        {' ', 0},    {'0', 1},    {'1', 2},    {'2', 3},
        {'3', 4},    {'4', 5},    {'5', 6},    {'6', 7},
        {'7', 8},    {'8', 9},    {'9', 10},   {'+', 11},
        {'-', 12},   {'=', 13},   {'/', 14},   {'\\', 15},
        {'.', 16},   {',', 17},   {'[', 18},   {']', 19},
        {'{', 20},   {'}', 21},   {'A', 22},   {'a', 23},
        {'B', 24},   {'b', 25},   {'C', 26},   {'c', 27},
        {'D', 28},   {'d', 29},   {'E', 30},   {'e', 31},
        {'F', 32},   {'f', 33},   {'G', 34},   {'g', 35},
        {'H', 36},   {'h', 37},   {'I', 38},   {'i', 39},
        {'J', 40},   {'j', 41},   {'K', 42},   {'k', 43},
        {'L', 44},   {'l', 45},   {'M', 46},   {'m', 47},
        {'N', 48},   {'n', 49},   {'O', 50},   {'o', 51},
        {'P', 52},   {'p', 53},   {'Q', 54},   {'q', 55},
        {'R', 56},   {'r', 57},   {'S', 66},   {'s', 67},
        {'T', 60},   {'t', 61},   {'U', 62},   {'u', 63},
        {'V', 64},   {'v', 65},   {'W', 68},   {'w', 69},
        {'X', 70},   {'x', 71},   {'Y', 72},   {'y', 73},
        {'Z', 74},   {'z', 75},   {'!', 76},   {'?', 77},
        {':', 78},   {'\"', 79},  {'\x08', 80}, {'\n', 81},
        {'\r', 82},  {'\t', 83}
    };
    
    // Static reversed lookup table: int -> string_view
    static const map<int, string_view> reversedVerification = {
        {0, " "},    {1, "0"},    {2, "1"},    {3, "2"},
        {4, "3"},    {5, "4"},    {6, "5"},    {7, "6"},
        {8, "7"},    {9, "8"},    {10, "9"},   {11, "+"},
        {12, "-"},   {13, "="},   {14, "/"},   {15, "\\"},
        {16, "."},   {17, ","},   {18, "["},   {19, "]"},
        {20, "{"},   {21, "}"},   {22, "A"},   {23, "a"},
        {24, "B"},   {25, "b"},   {26, "C"},   {27, "c"},
        {28, "D"},   {29, "d"},   {30, "E"},   {31, "e"},
        {32, "F"},   {33, "f"},   {34, "G"},   {35, "g"},
        {36, "H"},   {37, "h"},   {38, "I"},   {39, "i"},
        {40, "J"},   {41, "j"},   {42, "K"},   {43, "k"},
        {44, "L"},   {45, "l"},   {46, "M"},   {47, "m"},
        {48, "N"},   {49, "n"},   {50, "O"},   {51, "o"},
        {52, "P"},   {53, "p"},   {54, "Q"},   {55, "q"},
        {56, "R"},   {57, "r"},   {66, "S"},   {67, "s"},
        {60, "T"},   {61, "t"},   {62, "U"},   {63, "u"},
        {64, "V"},   {65, "v"},   {68, "W"},   {69, "w"},
        {70, "X"},   {71, "x"},   {72, "Y"},   {73, "y"},
        {74, "Z"},   {75, "z"},   {76, "!"},   {77, "?"},
        {78, ":"},   {79, "\""},  {80, "\x08"}, {81, "\n"},
        {82, "\r"},  {83, "\t"}
    };

    // Look up the character in the verification map.
    auto it = verification.find(ch);
    if (it != verification.end()) {
        int key = it->second;
        auto revIt = reversedVerification.find(key);
        if (revIt != reversedVerification.end()) {
            // Return a pointer to the corresponding string_view.
            return &revIt->second;
        }
    }
    // Return nullptr if the character is not found.
    return nullptr;
}

int main() {
    // string buffer = "Hello, world!";
    string buffer = "Hello my name is Alden Simmons";
    
    // Example: Process each character in buffer.
    for (size_t i = 0; i < buffer.size(); ++i) {
        const string_view* svPtr = pointerLexicon(buffer[i]);
        if (svPtr) {
            cout << "buffer[" << i << "] = '" << buffer[i]
                 << "' => \"" << *svPtr << "\"\n";
        } else {
            cout << "buffer[" << i << "] = '" << buffer[i]
                 << "' not found in lexicon\n";
        }
    }
    
    return 0;
}
