#include <windows.h>
#include <psapi.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../include/memory_utill.hpp"
#include "../include/string_utill.hpp"

using namespace std;

// A character block stored as a half-open range [start, end) into a
// vector<string_view> of filtered lines. The referenced lines must outlive
// any BlockSpan that indexes them.
struct BlockSpan {
    std::size_t start = 0;  // inclusive
    std::size_t end   = 0;  // exclusive
};

string loadFileIntoString(const string &filename);
vector<string_view> splitLines(const string &buffer);
void pointerPlayOptimized(const string &filename);


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

vector<string_view> filterCharacterBlocks(const vector<string_view> &allLines) {
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
    int lineNumBefore = static_cast<int>(allLines.size());
    int lineNumAfter = 0;
    int *counters[4] = {&foundBlockTypeA, &foundBlockTypeB, &lineNumBefore, &lineNumAfter};

    for (std::size_t i = 0; i < allLines.size(); i++) {
        // If not in a block, check for the two-line pattern:
        //   line i ends with "={"
        //   line i+1 starts with "\tfirst_name="
        if (!inBlock) {
            if (i + 1 < allLines.size()) {
                // Check if line i ends with "={"
                const string_view &lineA = allLines[i];
                if (lineA.size() >= 2 && lineA.substr(lineA.size() - 2) == "={") {
                    // Check next line for first_name=
                    if (inBlock == false) nameInit = firstNameCheck(allLines[i+1], "\tfirst_name=");

                    if (nameInit == true) {
                        // This is the start of a character block
                        foundBlockTypeA++;
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
                        foundBlockTypeB++;
                    }
                }
            }
            // If the pattern didn't match, skip this line
        }
        else {
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

    lineNumAfter = static_cast<int>(filtered.size());
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

// Try to parse a line of the form "  12345={  " (whitespace + digits + "=" + "{" + optional trailing whitespace).
// On match, sets outId to the parsed integer and returns true. On no match, returns false.
// Operates directly on string_view to avoid materializing a string per line.
static bool tryParseBlockHeader(string_view line, int& outId) {
    std::size_t i = 0;
    const std::size_t n = line.size();

    // Skip leading whitespace
    while (i < n && isspace(static_cast<unsigned char>(line[i]))) i++;
    if (i >= n) return false;

    // Must start with at least one digit
    if (!isdigit(static_cast<unsigned char>(line[i]))) return false;

    int value = 0;
    while (i < n && isdigit(static_cast<unsigned char>(line[i]))) {
        value = value * 10 + (line[i] - '0');
        i++;
    }

    // Must be followed by exactly "={"
    if (i + 1 >= n) return false;
    if (line[i] != '=' || line[i + 1] != '{') return false;
    i += 2;

    // Allow trailing whitespace, but nothing else
    while (i < n && isspace(static_cast<unsigned char>(line[i]))) i++;
    if (i != n) return false;

    outId = value;
    return true;
}

// Check if a string_view starts with a given prefix. Used in place of
// std::regex_match for the "next line begins with first_name=" check.
static bool startsWithPrefix(string_view sv, string_view prefix) {
    if (sv.size() < prefix.size()) return false;
    for (std::size_t i = 0; i < prefix.size(); i++) {
        if (sv[i] != prefix[i]) return false;
    }
    return true;
}

unordered_map<int, BlockSpan> blockMapping(const vector<string_view> &totalLines) {
    // The leading-whitespace prefix of the first_name= discriminator. We
    // look for it after trimming leading whitespace from the next line.
    static const string_view firstNamePrefix = "first_name=";

    unordered_map<int, BlockSpan> blockMap;

    int currentBlockId = -1;
    std::size_t currentBlockStart = 0;
    int braceDepth = 0;
    bool patternCheck = false;

    for (size_t i = 0; i < totalLines.size(); i++) {
        if (!patternCheck) {
            if (i + 1 < totalLines.size()) {
                int parsedId = 0;
                if (!tryParseBlockHeader(totalLines[i], parsedId)) continue;

                // Check the next line for the first_name= discriminator.
                // We trim leading whitespace inline rather than constructing
                // a temporary string.
                string_view next = totalLines[i + 1];
                std::size_t k = 0;
                while (k < next.size() && isspace(static_cast<unsigned char>(next[k]))) k++;
                if (!startsWithPrefix(next.substr(k), firstNamePrefix)) continue;

                // Confirmed: this is a character block opening.
                currentBlockId = parsedId;
                currentBlockStart = i;
                patternCheck = true;
                braceDepth = 0;

                for (char c : totalLines[i]) {
                    if (c == '{') braceDepth++;
                    if (c == '}') braceDepth--;
                }

                if (braceDepth <= 0) {
                    blockMap[currentBlockId] = BlockSpan{currentBlockStart, i + 1};

                    currentBlockId = -1;
                    currentBlockStart = 0;
                    braceDepth = 0;
                    patternCheck = false;
                }
            }
        }
        else {
            for (char c : totalLines[i]) {
                if (c == '{') braceDepth++;
                if (c == '}') braceDepth--;
            }

            if (braceDepth <= 0) {
                blockMap[currentBlockId] = BlockSpan{currentBlockStart, i + 1};

                currentBlockId = -1;
                currentBlockStart = 0;
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

// Extract every run of decimal digits from a line and return them as ints.
// Replaces an earlier regex-based version; on the hot path this is roughly
// an order of magnitude faster.
vector<int> extractIdsFromLine(const string &line) {
    vector<int> ids;
    const std::size_t n = line.size();
    std::size_t i = 0;
    while (i < n) {
        // Skip non-digits
        while (i < n && !isdigit(static_cast<unsigned char>(line[i]))) {
            i++;
        }
        if (i >= n) break;

        // Accumulate digits
        int value = 0;
        while (i < n && isdigit(static_cast<unsigned char>(line[i]))) {
            value = value * 10 + (line[i] - '0');
            i++;
        }
        ids.push_back(value);
    }
    return ids;
}

vector<int> extractSpouseIds(const vector<string_view> &allLines, const BlockSpan &span) {
    vector<int> spouseIds;

    for (std::size_t i = span.start; i < span.end; i++) {
        string line = trim(string(allLines[i]));

        if (line.rfind("spouse=", 0) == 0) {
            vector<int> found = extractIdsFromLine(line);

            for (int id : found) {
                spouseIds.push_back(id);
            }
        }
    }

    return spouseIds;
}

vector<int> extractChildIds(const vector<string_view> &allLines, const BlockSpan &span) {
    vector<int> childIds;

    for (std::size_t i = span.start; i < span.end; i++) {
        string line = trim(string(allLines[i]));

        if (line.rfind("child=", 0) == 0) {
            vector<int> found = extractIdsFromLine(line);

            for (int id : found) {
                childIds.push_back(id);
            }
        }
    }

    return childIds;
}

vector<int> extractIdsByPrefix(const vector<string> &block, const string &prefix) {
    vector<int> ids;

    for (const string &rawLine : block) {
        string line = trim(rawLine);

        if (line.rfind(prefix, 0) == 0) {
            vector<int> found = extractIdsFromLine(line);

            for (int id : found) {
                ids.push_back(id);
            }
        }
    }

    return ids;
}

/*
################################
Temporary Character Block Stats
################################
*/

struct CharacterBlockStats {
    size_t characterCount = 0;
    size_t totalLines = 0;
    size_t totalSpouseRefs = 0;
    size_t totalChildRefs = 0;
};

struct RelationshipRefCounts {
    size_t spouseRefs = 0;
    size_t childRefs = 0;
};

RelationshipRefCounts countRelationshipRefs(const vector<string_view> &allLines, const BlockSpan &span) {
    RelationshipRefCounts counts;

    for (std::size_t i = span.start; i < span.end; i++) {
        string line = trim(string(allLines[i]));

        if (line.rfind("spouse=", 0) == 0) {
            counts.spouseRefs += extractIdsFromLine(line).size();
        }
        else if (line.rfind("child=", 0) == 0) {
            counts.childRefs += extractIdsFromLine(line).size();
        }
    }

    return counts;
}

RelationshipRefCounts countRelationshipRefs(const vector<string> &block) {
    RelationshipRefCounts counts;

    for (const string &rawLine : block) {
        string line = trim(rawLine);

        if (line.rfind("spouse=", 0) == 0) {
            counts.spouseRefs += extractIdsFromLine(line).size();
        }
        else if (line.rfind("child=", 0) == 0) {
            counts.childRefs += extractIdsFromLine(line).size();
        }
    }

    return counts;
}

CharacterBlockStats collectCharacterBlockStats(
    const unordered_map<int, BlockSpan> &characterBlocks,
    const vector<string_view> &allLines
) {
    CharacterBlockStats stats;
    stats.characterCount = characterBlocks.size();

    for (const auto &entry : characterBlocks) {
        const BlockSpan &span = entry.second;
        RelationshipRefCounts counts = countRelationshipRefs(allLines, span);

        stats.totalLines += span.end - span.start;
        stats.totalSpouseRefs += counts.spouseRefs;
        stats.totalChildRefs += counts.childRefs;
    }

    return stats;
}

CharacterBlockStats collectCharacterBlockStats(const unordered_map<int, vector<string>> &characterBlocks) {
    CharacterBlockStats stats;
    stats.characterCount = characterBlocks.size();

    for (const auto &entry : characterBlocks) {
        const vector<string> &block = entry.second;
        RelationshipRefCounts counts = countRelationshipRefs(block);

        stats.totalLines += block.size();
        stats.totalSpouseRefs += counts.spouseRefs;
        stats.totalChildRefs += counts.childRefs;
    }

    return stats;
}

void printCharacterBlockStats(const CharacterBlockStats &stats, const string &label) {
    cout << "\n";
    cout << "|------------TEMP CHARACTER BLOCK STATS: " << label << "------------|" << endl;

    cout << "Characters counted: "
         << formatWithCommas(to_string(stats.characterCount)) << endl;

    cout << "Total block lines: "
         << formatWithCommas(to_string(stats.totalLines)) << endl;

    cout << "Total spouse references: "
         << formatWithCommas(to_string(stats.totalSpouseRefs)) << endl;

    cout << "Total child references: "
         << formatWithCommas(to_string(stats.totalChildRefs)) << endl;

    cout << "Total spouse + child references: "
         << formatWithCommas(to_string(stats.totalSpouseRefs + stats.totalChildRefs)) << endl;

    if (stats.characterCount > 0) {
        cout << fixed << setprecision(4);

        cout << "Average lines per character block: "
             << static_cast<double>(stats.totalLines) / stats.characterCount << endl;

        cout << "Average spouse references per character: "
             << static_cast<double>(stats.totalSpouseRefs) / stats.characterCount << endl;

        cout << "Average child references per character: "
             << static_cast<double>(stats.totalChildRefs) / stats.characterCount << endl;

        cout << "Average spouse + child references per character: "
             << static_cast<double>(stats.totalSpouseRefs + stats.totalChildRefs) / stats.characterCount << endl;
    }

    cout << "|------------------------------------------------------------|" << endl;
    cout << "\n";
}

void progenySortRecursive(
    unordered_map<int, BlockSpan> &sourceMap,
    unordered_map<int, vector<string>> &resultMap,
    set<int> &processedDescendants,
    const vector<string_view> &allLines,
    int ID
) {
    vector<string> *currentBlock = nullptr;

    // If this character is already copied into resultMap, use that block.
    // This matters if someone was copied earlier as a spouse.
    auto resultIt = resultMap.find(ID);

    if (resultIt != resultMap.end()) {
        currentBlock = &resultIt->second;
    }
    else {
        auto sourceIt = sourceMap.find(ID);

        if (sourceIt == sourceMap.end()) {
            cout << "[WARN] Character ID not found: " << ID << endl;
            return;
        }

        vector<string> blockCopy;
        blockCopy.reserve(sourceIt->second.end - sourceIt->second.start);
        for (std::size_t i = sourceIt->second.start; i < sourceIt->second.end; i++) {
            blockCopy.emplace_back(allLines[i]);
        }

        // Copy this descendant/founder into the result map.
        auto inserted = resultMap.emplace(ID, std::move(blockCopy));
        currentBlock = &inserted.first->second;

        // Delete from source map as we go.
        sourceMap.erase(sourceIt);
    }

    // Do not process this descendant's children twice.
    if (processedDescendants.count(ID) > 0) {
        return;
    }

    processedDescendants.insert(ID);

    // Copy spouses, but DO NOT recurse through spouses.
    vector<int> spouseIds = extractIdsByPrefix(*currentBlock, "spouse=");

    for (int spouseID : spouseIds) {
        if (resultMap.find(spouseID) == resultMap.end()) {
            auto spouseIt = sourceMap.find(spouseID);

            if (spouseIt != sourceMap.end()) {
                vector<string> blockCopy;
                blockCopy.reserve(spouseIt->second.end - spouseIt->second.start);
                for (std::size_t i = spouseIt->second.start; i < spouseIt->second.end; i++) {
                    blockCopy.emplace_back(allLines[i]);
                }

                resultMap[spouseID] = std::move(blockCopy);
                sourceMap.erase(spouseIt);
            }
            else {
                cout << "[WARN] Spouse ID not found: " << spouseID << endl;
            }
        }
    }

    // Recursively follow children only.
    vector<int> childIds = extractIdsByPrefix(*currentBlock, "child=");

    for (int childID : childIds) {
        progenySortRecursive(sourceMap, resultMap, processedDescendants, allLines, childID);
    }
}

unordered_map<int, vector<string>> progenySort(
    unordered_map<int, BlockSpan> &characterMap,
    const vector<string_view> &allLines,
    int ID
) {
    unordered_map<int, vector<string>> progenyMap;
    set<int> processedDescendants;

    progenySortRecursive(characterMap, progenyMap, processedDescendants, allLines, ID);

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
        vector<string_view> characterOnly = filterCharacterBlocks(totallyAssimilatedLines);

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

        // [9]  |  Record Memory Usage | After mapping block spans
        unordered_map<int, BlockSpan> characterMap = blockMapping(finalCharacterLines);
        recordMemoryUsage(memorytable, memoryValues, true, true, 9);

        // TEMP STATS: all character blocks before progenySort mutates characterMap.
        CharacterBlockStats allCharacterStats = collectCharacterBlockStats(characterMap, finalCharacterLines);
        printCharacterBlockStats(allCharacterStats, "ALL CHARACTERS");

        // [10]  |  Record Memory Usage | After collecting all-character stats.
        recordMemoryUsage(memorytable, memoryValues, true, true, 10);

        // [11]  |  Record Memory Usage | After sorting founder descendants/spouses
        int founderID = 37676;
        unordered_map<int, vector<string>> dynastyMap = progenySort(characterMap, finalCharacterLines, founderID);
        recordMemoryUsage(memorytable, memoryValues, true, true, 11);

        // TEMP STATS: only the extracted dynasty/progeny map.
        CharacterBlockStats dynastyStats = collectCharacterBlockStats(dynastyMap);
        printCharacterBlockStats(dynastyStats, "DYNASTY / PROGENY MAP");

        cout << "Dynasty/progeny map size: " << formatWithCommas(to_string(dynastyMap.size()))<< endl;
        cout << "Remaining source map size: " << formatWithCommas(to_string(characterMap.size())) << endl << endl;

        finalCharacterLines.clear();
        finalCharacterLines.shrink_to_fit();

        // [12]  |  Record Memory Usage | After clearing source spans and block views.
        characterMap.clear();
        unordered_map<int, BlockSpan>().swap(characterMap);
        recordMemoryUsage(memorytable, memoryValues, true, true, 12);

        memoryLogging(memorytable, false, 0);

    } 
    catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
    }
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
