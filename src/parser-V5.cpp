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

#include "../include/memory_utill.hpp"
#include "../include/string_utill.hpp"

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

