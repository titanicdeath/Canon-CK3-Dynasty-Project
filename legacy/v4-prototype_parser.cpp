#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <list>
#include <cctype>
#include <iomanip>
#include <map>
#include <filesystem>
#include <regex>
#include <set>

// #include "../library/memory_utill.hpp"
// #include "../library/string_utill.hpp"
// #include <memory_utill.cpp>
// #include <string_utill.cpp>

#include "../library/memory_utill.hpp"
#include "../library/string_utill.hpp"

using namespace std;

string loadFileIntoString(const string &filename);
vector<string_view> splitLines(const string &buffer);
void pointerPlayOptimized(const string &filename);

vector<string> findBlock(const vector<string_view> &totalLines, const string &charID);


// Load the entire file into a single string
string loadFileIntoString(const string &filename) {
    ifstream in(filename, ios::in | ios::binary);
    if (!in) {
        throw runtime_error("Could not open file: " + filename);
    }
    ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

// Split the buffer into lines using string_view to avoid copying data.
vector<string_view> splitLines(const string &buffer) {
    vector<string_view> lines;
    size_t start = 0;
    while (start < buffer.size()) {
        size_t end = buffer.find('\n', start);
        if (end == string::npos) {
            end = buffer.size();
        }
        // Trim a trailing '\r' if present
        std::size_t lineLength = (end > start && buffer[end - 1] == '\r') ? end - start - 1 : end - start;
        lines.emplace_back(buffer.data() + start, lineLength);
        start = end + 1;
    }
    return lines;
}

bool firstNameCheck(string_view current, const string& check) {
    if (current.size() < check.size()) {
        return false;
    }

    for (size_t i = 0; i < check.size(); i++) {
        if (current[i] != check[i]) {
            return false;
        }
    }

    return true;
}

vector<string_view> filterCharacterBlocks(const vector<string_view> &allLines, string &buffer){
    // cout << "[DEBUG] fCB 1" << endl;
    vector<string_view> filtered;
    bool inBlock = false;
    bool nameInit = false;
    int braceDepth = 0;

    string blockTypeA = "Character blocks Found";
    string blockTypeB = "Other blocks Found";
    string lineBefore = "Lines BEFORE";
    string lineAfter = "Lines AFTER";
    string *blockTypes[4] = {&blockTypeA, &blockTypeB, &lineBefore, &lineAfter};

    int foundBlockTypeA = 0;
    int foundBlockTypeB = 0;
    int lineNumBefore = allLines.size();
    int lineNumAfter;
    int *counters[4] = {&foundBlockTypeA, &foundBlockTypeB, &lineNumBefore, &lineNumAfter};


    for (std::size_t i = 0; i < allLines.size(); i++) {
        //cout << "[DEBUG] fCB 2." << i << endl;
        // If not in a block, check for the two-line pattern:
        //   line i ends with "={"
        //   line i+1 starts with "first_name="
        if (!inBlock) {
            //cout << "[DEBUG] fCB 4." << i << endl;
            if (i + 1 < allLines.size()) {
                //cout << "[DEBUG] fCB 6." << i << endl;
                // Trim left for next line (to ignore leading spaces/tabs)
                // do a small inline function:
                auto ltrim = [](string_view sv) {
                    std::size_t j = 0;
                    while (j < sv.size() && isspace((unsigned char)sv[j])) {
                        j++;
                    }
                    return sv.substr(j);
                };
                
                // Check if line i ends with "={"
                const string_view &lineA = allLines[i];
                if (lineA.size() >= 2 && lineA.substr(lineA.size() - 2) == "={") {
                    // cout << "[DEBUG] fCB 5." << i << endl;
                    // Check next line for first_name=
                    
                    if (inBlock == false) nameInit = firstNameCheck(allLines[i+1], "\tfirst_name=");
                
                    if (nameInit == true) {
                        // This is the start of a character block
                        foundBlockTypeA++;
                        //cout << "[DEBUG] fCB 7." << i << endl;
                        //cout << "[DEBUG] fCB 7." << i << ":\t" << allLines[i+1] << endl;
                        inBlock = true;
                        braceDepth = 0;

                        // Add line i to filtered, then count braces
                        filtered.push_back(lineA);
                        for (char c : lineA) {
                            if (c == '{') braceDepth++;
                            if (c == '}') braceDepth--;
                        }

                        // If the block opened and closed on the same line
                        if (braceDepth <= 0) {
                            inBlock = false;
                            nameInit = false;
                        }
                        // Done checking; move on
                        continue;
                    }
                    else if (nameInit == false){ 
                        //buffer.erase(i);
                        foundBlockTypeB++;
                        
                    }
                    
                }
            }
            // If the pattern didn't match, skip this line
        }
        else {
            //cout << "[DEBUG] fCB 3." << i << endl;
            // We are inside a character block

            filtered.push_back(allLines[i]);
            // Count braces
            for (char c : allLines[i]) {
                if (c == '{') braceDepth++;
                if (c == '}') braceDepth--;
            }
            // If the block closes, stop
            if (braceDepth <= 0) {
                inBlock = false;
                nameInit = false;
            }
        }
    }

    lineNumAfter = filtered.size();
    int colWidthLabel = 25;
    int colWidthValue = 12;

    int start = (((4 + colWidthLabel + colWidthValue) - 5) / 2) + 1;
    int end = (((4 + colWidthLabel + colWidthValue) - 3) / 2) + 3;
    cout << "|" << string((start), '-') << "START" << string((start-1), '-') << "|" << endl;
    for (int i = 0; i < 4; i++) {
        cout << "| " << left << setw(colWidthLabel) << *blockTypes[i] << " | " << right << setw(colWidthValue) << formatWithCommas(to_string(*counters[i])) << " |\n";
    }
    cout << "|" << string((end), '-') << "END" << string((end-5), '-') << "|" << endl;

    return filtered;
}


map<int, vector<string>> blockMapping(const vector<string_view> &totalLines) {
    regex numericBlockPattern(R"(^\s*([0-9]+)=\{\s*$)");
    string secondPattern = "first_name=";

    map<int, vector<string>> blockMap;

    vector<string> block;
    int currentBlockId = -1;
    int braceDepth = 0;
    bool patternCheck = false;

    for (size_t i = 0; i < totalLines.size(); i++) {
        string line = trim(string(totalLines[i]));
        smatch match;

        if (!patternCheck) {
            if (i + 1 < totalLines.size()) {
                string nextLine = trim(string(totalLines[i + 1]));

                if (regex_match(line, match, numericBlockPattern) &&
                    nextLine.rfind(secondPattern, 0) == 0) {
                    
                    string blockId = match[1];
                    currentBlockId = stoi(blockId);

                    patternCheck = true;
                    braceDepth = 0;
                    block.clear();

                    block.push_back(string(totalLines[i]));

                    for (char c : string(totalLines[i])) {
                        if (c == '{') braceDepth++;
                        if (c == '}') braceDepth--;
                    }

                    continue;
                }
            }
        }
        else {
            block.push_back(string(totalLines[i]));

            for (char c : string(totalLines[i])) {
                if (c == '{') braceDepth++;
                if (c == '}') braceDepth--;
            }

            if (braceDepth <= 0) {
                blockMap[currentBlockId] = block;

                block.clear();
                currentBlockId = -1;
                braceDepth = 0;
                patternCheck = false;
            }
        }
    }

    return blockMap;
}

/*
################################
Progeny
################################
*/

vector<int> extractIdsFromLine(const string &line) {
    vector<int> ids;
    regex numberPattern(R"([0-9]+)");

    sregex_iterator current(line.begin(), line.end(), numberPattern);
    sregex_iterator end;

    while (current != end) {
        ids.push_back(stoi((*current)[0]));
        current++;
    }

    return ids;
}

vector<int> extractSpouseIds(const vector<string> &block) {
    vector<int> spouseIds;

    for (const string &rawLine : block) {
        string line = trim(rawLine);

        if (line.rfind("spouse=", 0) == 0) {
            vector<int> found = extractIdsFromLine(line);

            for (int id : found) {
                spouseIds.push_back(id);
            }
        }
    }

    return spouseIds;
}

vector<int> extractChildIds(const vector<string> &block) {
    vector<int> childIds;

    for (const string &rawLine : block) {
        string line = trim(rawLine);

        if (line.rfind("child=", 0) == 0) {
            vector<int> found = extractIdsFromLine(line);

            for (int id : found) {
                childIds.push_back(id);
            }
        }
    }

    return childIds;
}

void progenySortRecursive(
    map<int, vector<string>> &sourceMap,
    map<int, vector<string>> &resultMap,
    set<int> &processedDescendants,
    int ID
) {
    vector<string> currentBlock;

    // If this character is already copied into resultMap, use that block.
    // This matters if someone was copied earlier as a spouse.
    auto resultIt = resultMap.find(ID);

    if (resultIt != resultMap.end()) {
        currentBlock = resultIt->second;
    }
    else {
        auto sourceIt = sourceMap.find(ID);

        if (sourceIt == sourceMap.end()) {
            cout << "[WARN] Character ID not found: " << ID << endl;
            return;
        }

        currentBlock = sourceIt->second;

        // Copy this descendant/founder into the result map.
        resultMap[ID] = currentBlock;

        // Delete from source map as we go.
        sourceMap.erase(sourceIt);
    }

    // Do not process this descendant's children twice.
    if (processedDescendants.count(ID) > 0) {
        return;
    }

    processedDescendants.insert(ID);

    // Copy spouses, but DO NOT recurse through spouses.
    vector<int> spouseIds = extractSpouseIds(currentBlock);

    for (int spouseID : spouseIds) {
        if (resultMap.find(spouseID) == resultMap.end()) {
            auto spouseIt = sourceMap.find(spouseID);

            if (spouseIt != sourceMap.end()) {
                resultMap[spouseID] = spouseIt->second;
                sourceMap.erase(spouseIt);
            }
            else {
                cout << "[WARN] Spouse ID not found: " << spouseID << endl;
            }
        }
    }

    // Recursively follow children only.
    vector<int> childIds = extractChildIds(currentBlock);

    for (int childID : childIds) {
        progenySortRecursive(sourceMap, resultMap, processedDescendants, childID);
    }
}

map<int, vector<string>> progenySort(map<int, vector<string>> &characterMap, int ID) {
    map<int, vector<string>> progenyMap;
    set<int> processedDescendants;

    progenySortRecursive(characterMap, progenyMap, processedDescendants, ID);

    return progenyMap;
}


/*
vector<string> findBlock(const vector<string_view> &totalLines, const string &charID){
    // The patterns to match
    // example: "37676={" and (on next line) "first_name="
    string firstPattern  = "={"; 
    string secondPattern = "first_name="; // ignoring leading tabs/spaces
    
    vector<string> block;
    int braceDepth      = 0;
    // how many lines added
    int stretch         = 0;         
    // line index where the block started
    int startLineNumber = -1; 
    bool patternCheck   = false;

    for (std::size_t i = 0; i < totalLines.size(); i++){
        // Convert string_view to string, then trim
        string line = trim(string(totalLines[i]));
        
        // 1) Check if the next line exists before we do i+1
        if (i + 1 < totalLines.size()){
            // We'll also trim the next line for comparison
            string nextLine = trim(string(totalLines[i+1]));

            // If line matches "37676={" (or ######={"  ) exactly 
            // AND the following line starts with or equals "first_name="
            // then set patternCheck = true and initialize braceDepth to 1
            if (!patternCheck && (line == firstPattern) && (nextLine.rfind(secondPattern, 0) == 0)){
                patternCheck = true;
                braceDepth   = 1;  // we already have an opening brace on this line
                startLineNumber = (int) i; 
                
                // We'll also push this line into the block
                block.push_back(string(totalLines[i]));
                stretch++;
                continue; // jump to next iteration
            }
        }

        // 2) If we've already found our pattern, accumulate lines
        if (patternCheck){
            // If we just matched the first line of the block in the step above,
            // we skip pushing the line again. Otherwise, push the new line.
            if (startLineNumber != (int) i){
                block.push_back(string(totalLines[i]));
                stretch++;
            }

            // Count braces in the current line (untrimmed might matter, but typically it's the same)
            // We'll do it on the untrimmed version for safety:
            const string &rawLine = string(totalLines[i]);
            for (char c : rawLine){
                if (c == '{') braceDepth++;
                if (c == '}') braceDepth--;
            }

            // If braceDepth less than or equal to 0, parser reached the closing brace
            if (braceDepth <= 0){
                break;
            }

            // Safety limit
            if (stretch >= 1000){
                cout << "[STRETCH ERROR] Block too large. Or for loop running loose from missreading Blokcs\n";
                cout << "[STRETCH ERROR] For loop running loose from Blokcs read error\n";
                break;
            }
        }

    }

    // If never set startLineNumber patten never found
    if (startLineNumber < 0){
        cout << "No block found for " << charID << "\n";
    }

    // Record the start line and number of lines processed
    block.push_back(to_string(startLineNumber));
    block.push_back(to_string(stretch));

    return block;
}
*/
std::string buildCharacterBlocksString(const std::vector<std::string_view>& characterOnly)
{
    std::string newBuffer;
    // You can reserve if you have a rough idea of the total length
    // For example, each line might be ~100 bytes on average times characterOnly.size().
    newBuffer.reserve(characterOnly.size() * 100); 

    for (auto& lineView : characterOnly) {
        newBuffer.append(lineView.data(), lineView.size());
        newBuffer.push_back('\n');
    }
    return newBuffer;
}

vector<string_view> pointerLexicon(const string &buffer) {
    // Static maps for conversion – they persist for the program lifetime.
    static map<char, int> verification = {
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
        {'\r', 82},  {'\t', 83}, {'_', 84}
    };

    static map<int, char> reversedVerification = {
        {0, ' '},    {1, '0'},    {2, '1'},    {3, '2'},
        {4, '3'},    {5, '4'},    {6, '5'},    {7, '6'},
        {8, '7'},    {9, '8'},    {10, '9'},   {11, '+'},
        {12, '-'},   {13, '='},   {14, '/'},   {15, '\\'},
        {16, '.'},   {17, ','},   {18, '['},   {19, ']'},
        {20, '{'},   {21, '}'},   {22, 'A'},   {23, 'a'},
        {24, 'B'},   {25, 'b'},   {26, 'C'},   {27, 'c'},
        {28, 'D'},   {29, 'd'},   {30, 'E'},   {31, 'e'},
        {32, 'F'},   {33, 'f'},   {34, 'G'},   {35, 'g'},
        {36, 'H'},   {37, 'h'},   {38, 'I'},   {39, 'i'},
        {40, 'J'},   {41, 'j'},   {42, 'K'},   {43, 'k'},
        {44, 'L'},   {45, 'l'},   {46, 'M'},   {47, 'm'},
        {48, 'N'},   {49, 'n'},   {50, 'O'},   {51, 'o'},
        {52, 'P'},   {53, 'p'},   {54, 'Q'},   {55, 'q'},
        {56, 'R'},   {57, 'r'},   {66, 'S'},   {67, 's'},
        {60, 'T'},   {61, 't'},   {62, 'U'},   {63, 'u'},
        {64, 'V'},   {65, 'v'},   {68, 'W'},   {69, 'w'},
        {70, 'X'},   {71, 'x'},   {72, 'Y'},   {73, 'y'},
        {74, 'Z'},   {75, 'z'},   {76, '!'},   {77, '?'},
        {78, ':'},   {79, '\''},  {80, '\x08'}, {81, '\n'},
        {82, '\r'},  {83, '\t'}, {84, '_'}, {85, ';'}

    };

    vector<string_view> result;
    // A persistent buffer to accumulate the converted characters.
    // Using a static string ensures the storage lives long enough for the string_views.
    static string fullOutput;
    fullOutput.clear();
    fullOutput.reserve(buffer.size());
    cout << "WHY\n";
    std::size_t lineStart = 0;  // Mark the beginning of the current line

    for (std::size_t i = 0; i < buffer.size(); i++) {
        auto it = verification.find(buffer[i]);
        if (it == verification.end()) {
            // If the character isn't found, ignore it.
            continue;
        }
        int step = it->second;
        char mappedChar = reversedVerification.at(step);
        fullOutput.push_back(mappedChar);
    
        // If we've hit a newline, create a view for the current line.
        if (mappedChar == '\n') {
            std::size_t lineLength = fullOutput.size() - lineStart;
            string_view lineView(fullOutput.data() + lineStart, lineLength);
            result.push_back(lineView);
            lineStart = fullOutput.size();
        }
    }

    // If there is any leftover text without a trailing newline, push that as well.
    if (lineStart < fullOutput.size()) {
        string_view lineView(fullOutput.data() + lineStart, fullOutput.size() - lineStart);
        result.push_back(lineView);
    }

    return result;
}

// string reduceMemory(string &buffer, vector<string_view> characterOnly){
//     string_view proxy;
//     for (int i = 0; i < buffer.size(); i++){
//         proxy = characterOnly[i];
//         if (proxy[i] != buffer[i]){
//             buffer.erase(i);
//         }
//     }
//     return buffer;
// }

void pointerPlayOptimized(const string &filename) {
    try {
        vector<vector<string>> memorytable;
        vector<std::size_t> memoryValues;

        // Load the entire file as one contiguous string.
        // [1]  | Record Memory Usage | Before loading the file.
        recordMemoryUsage(memorytable, memoryValues, true, true, 1);
        string fileBuffer = loadFileIntoString(filename);

        // [2]  |  Record Memory Usage | After loading the file.
        recordMemoryUsage(memorytable, memoryValues, true, true, 2);
        vector<string_view> totallyAssimilatedLines = splitLines(fileBuffer);

        // [3]  |  Record Memory Usage | After splitting lines.
        recordMemoryUsage(memorytable, memoryValues, true, true, 3);
        vector<string_view> characterOnly = filterCharacterBlocks(totallyAssimilatedLines, fileBuffer);

        // [4]  |  Record Memory Usage | After filtering non-character blocks.
        recordMemoryUsage(memorytable, memoryValues, true, true, 4);

        // [5]  |  Record Memory Usage | After building new filtered buffer.
        string reduced = buildCharacterBlocksString(characterOnly);
        recordMemoryUsage(memorytable, memoryValues, true, true, 5);

        // [6]  |  Record Memory Usage | After clearing old buffer.
        fileBuffer.clear();
        fileBuffer.shrink_to_fit(); 
        recordMemoryUsage(memorytable, memoryValues, true, true, 6);

        // [7]  |  Record Memory Usage | After clearing old splitted buffer reference.
        totallyAssimilatedLines.clear();
        totallyAssimilatedLines.shrink_to_fit();
        recordMemoryUsage(memorytable, memoryValues, true, true, 7);

        // [8]  |  Record Memory Usage | After making new filtered reference line
        vector<string_view> finalCharacterLines = splitLines(reduced);
        recordMemoryUsage(memorytable, memoryValues, true, true, 8);

        // [9]  |  Record Memory Usage | After mapping blocks
        map<int, vector<string>> characterMap = blockMapping(finalCharacterLines);
        recordMemoryUsage(memorytable, memoryValues, true, true, 9);

        // [10]  |  Record Memory Usage | After clearing unorganized block views
        finalCharacterLines.clear();
        finalCharacterLines.shrink_to_fit();
        recordMemoryUsage(memorytable, memoryValues, true, true, 10);

        // [11]  |  Record Memory Usage | After sorting founder descendants/spouses
        int founderID = 37676;
        map<int, vector<string>> dynastyMap = progenySort(characterMap, founderID);
        recordMemoryUsage(memorytable, memoryValues, true, true, 11);

        cout << "Dynasty/progeny map size: " << formatWithCommas(to_string(dynastyMap.size()))<< endl;
        cout << "Remaining source map size: " << formatWithCommas(to_string(characterMap.size())) << endl << endl;

        // [12]  |  Record Memory Usage | After clearing unrelated character map
        characterMap.clear();
        map<int, vector<string>>().swap(characterMap);
        recordMemoryUsage(memorytable, memoryValues, true, true, 12);
        // for (string line : characterMap[charID]) {
        //     cout << line << endl;
        // }



        // [#]  |  Record Memory Usage | After finding blocks.
        // string charID = "37676";
        // vector<string> block = findBlock(finalCharacterLines, charID);
        // recordMemoryUsage(memorytable, memoryValues, true, true, 9);
    

        memoryLogging(memorytable);

    } 
    catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
    }
}

void pause(){
    string fuckPointers;
    cout << "\nEnter anything\n> ";
    cin >> fuckPointers;
}

int main() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::filesystem::path exeFile(exePath);
    std::filesystem::path exeDir = exeFile.parent_path();

    // Construct the path to the gamestate file relative to the executable
    std::filesystem::path filePath = exeDir / "save_game" / "gamestate";

    // Convert the path to a string for your function
    string inputFile = filePath.string();
    
    cout << "[INFO] Looking for gamestate file at: " << inputFile << endl;

    pointerPlayOptimized(inputFile);
    return 0;
}

// .\universal_compiler.bat > "

