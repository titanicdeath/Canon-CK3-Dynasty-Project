#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <sstream>
#include "json.hpp"  // Single-header nlohmann JSON
#include <vector>
#include <map>

using namespace std;
using json = nlohmann::json;

bool startsWithKey(const string &line, const string &key);
string parseBraceBlock(const vector<string> &block, size_t &i);
map<string, string> parseBlockValues(const vector<string> &block,
                                     const vector<string> &keys);
map<string, string> driverParseBlockValues(const vector<string> &block);

static inline string stripQuotes(const string &s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
    {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

vector<string> findBlock(const string &filename, const string &charID){
    vector<string> block;
    ifstream parseStart(filename);
    if (!parseStart.is_open()){
        cerr << "[ERROR] Could not open file: " << filename << endl;
        return block;
    }
    const string startPattern = charID + "={", endPattern = "\t}";
    string lineStart, lineEnd;
    bool startFound = false, endFound = false;
    int lineNumber = 0, startBlock = 0, endBlock = 0, stretch = 0;

    // *** First pass: find the line that exactly matches "charID + '={'"
    while (getline(parseStart, lineStart)){
        lineNumber += 1;
        if (lineStart == startPattern){
            startFound = true;
            startBlock = lineNumber;
            block.push_back(lineStart);
        }
        if ((lineNumber == (startBlock + 1)) && (startFound == true)){
            //cout << lineStart << endl;
            block.push_back(lineStart);
            string cur = lineStart;
            string extracted;
            for (int i = 0; i < 12; i++){
                extracted = extracted + cur[i];
            }
            //cout << extracted << endl;
            if (extracted == "\tfirst_name="){
                break;
            }
        }
    }
    lineNumber = 0;
    stretch = 0;
    parseStart.close();
    ifstream parseEnd(filename);
    //startBlock = startBlock - 2;
    if (startFound == true){
        //cout << "Start Block Line Number: " << startBlock << endl;

        // We already saw an opening '{' on the startPattern line.
        // So let's track braceDepth starting at 1.
        int braceDepth = 0;  

        while (getline(parseEnd, lineEnd)){
            lineNumber += 1;
            // Only start counting once we reach or pass the block's start line.
            if (lineNumber >= (startBlock)) {
                //cout << "Brace Depth: " << braceDepth << endl;
                //cout << lineEnd << endl;
                block.push_back(lineEnd);
                stretch += 1;

                // Count any '{' or '}' on this line:
                for (char c : lineEnd) {
                    if (c == '{') braceDepth++;
                    if (c == '}') braceDepth--;
                }

                // If braceDepth returns to 0 or below, we've closed the block.
                if (braceDepth <= 0) {
                    break;
                }

                // Safety limit
                if (stretch >= 100){
                    cout << "TOO LONG" << endl;
                    break;
                }
            }
        }
    }
    parseEnd.close();
    return block;
}

vector<string> getBlock(vector<int> blockRange, const string &filename){
    cout << "Beginning of Function (getBlock)\n";
    int startLine = blockRange[0];
    int endLine = blockRange[1];
    int lineNum = 0, stretch = 0;
    vector<string> blockLines((endLine - startLine) + 1);
    blockLines[0] = to_string((endLine - startLine));
    ifstream in(filename);
    string line;
    while (getline(in, line)) {
        lineNum++;
        // If we haven't yet reached the start, skip
        if (lineNum <= startLine) {
            continue;
        }
        // If we've gone past the end, break out
        if (lineNum >= endLine) {
            break;
        }
        // If we're between startLine and endLine, store the line
        blockLines[stretch] = line;
        stretch++;
    }
    
    in.close();
    cout << "Ending of Function (getBlock)\n";
    return blockLines;
}

void mapping(const string &charID, vector<string> blockRange, const string &filename){
    cout << "Beginning of Function (mapping)\n";
    //vector blockLines = getBlock(blockRange,filename);
    for (int i = 1; i < blockRange.size(); i++){
        cout << blockRange[i] << endl;
    }
    
}




int main()
{
    // The text-based gamestate file (uncompressed from CK3 save)
    string inputFile = "gamestate";

    // Where we write output
    string outputFile = "test.jsonl";

    // open out file
    ofstream out(outputFile);
    if (!out.is_open())
    {
        cerr << "[ERROR] Could not open " << outputFile << " for writing!\n";
        return 1;
    }

    // The root character ID: e.g. "37676" for Maurikios
    string rootID = "37676";
    string captured;

    out.close();
    vector<string> test;

    test = findBlock(inputFile, rootID);
    map<string, string> parsed = driverParseBlockValues(test);

    vector<string> keys = {
        "first_name",
        "birth",
        "ethnicity",
        "faith",
        "dynasty_house",
        "dna",
        "spouse",
        "former_spouses",    // or "former_spouses" if that is the correct name in your text
        "child",
        "date",
        "reason",
        "domain"
    };
    for (int i = 0; i < keys.size(); i++){
        string curKey = keys[i];
        string element = parsed[curKey];
        cout << element << endl; 
    }
    
    
    //mapping(rootID, test, inputFile);

    //pause();

    return 0;
}


















bool startsWithKey(const string &line, const string &key)
{
    // Trim leading whitespace
    size_t pos = line.find_first_not_of(" \t");
    if (pos == string::npos) {
        return false;
    }
    string trimmed = line.substr(pos);

    // Build the patterns to check
    string patternEq = key + "=";   // e.g. birth=
    string patternBr = key + "={";  // e.g. birth={

    // rfind(pattern, 0) == 0 means "pattern" is found at position 0
    return (trimmed.rfind(patternEq, 0) == 0 ||
            trimmed.rfind(patternBr, 0) == 0);
}

// Reads a brace-enclosed block from the current line i onwards
// accumulates all lines until the matching '}' is found.
// Returns the entire block as one string, and advances i so that
// the calling function knows we've consumed multiple lines.
string parseBraceBlock(const vector<string> &block, size_t &i)
{
    string accumulated;
    int braceDepth = 0;

    // We’ll start with line i (the line that already contains key={...}).
    // We append lines until we close the braces.
    for (; i < block.size(); i++) {
        const string &curLine = block[i];
        // For each brace, adjust braceDepth
        for (char c : curLine) {
            if (c == '{') braceDepth++;
            if (c == '}') braceDepth--;
        }
        accumulated += curLine + " ";

        if (braceDepth <= 0) {
            // matched the closing brace
            break;
        }
    }

    return accumulated;
}

// Main parser function: it scans each line in 'block' for the specified keys.
// If the key is brace-enclosed (e.g. key={ ... }), it collects all lines until
// the matching '}' is found. Otherwise, it just extracts what follows the '='.
map<string, string> parseBlockValues(const vector<string> &block, const vector<string> &keys){
    map<string, string> result;

    for (size_t i = 0; i < block.size(); i++) {
        const string &line = block[i];

        // Check if the line starts with any of the desired keys
        for (auto &key : keys) {
            if (startsWithKey(line, key)) {
                // Found a line that begins with e.g. "birth=" or "birth={"
                // Next, find '=' so we can split.
                size_t eqPos = line.find('=');
                if (eqPos == string::npos) {
                    continue; // malformed line
                }

                // Everything after '=' is the "value" portion
                string remainder = line.substr(eqPos + 1);
                // Trim leading/trailing whitespace
                {
                    size_t left = remainder.find_first_not_of(" \t");
                    size_t right = remainder.find_last_not_of(" \t");
                    if (left == string::npos)
                        remainder.clear();
                    else
                        remainder = remainder.substr(left, right - left + 1);
                }

                // If it starts with '{', parse a multi-line block
                if (!remainder.empty() && remainder[0] == '{') {
                    // Use parseBraceBlock to read lines until we close the brace
                    string blockData = parseBraceBlock(block, i);

                    // If the key doesn't exist in the result, set it;
                    // otherwise, append (e.g. multiple "spouse=" lines).
                    if (result.find(key) == result.end()) {
                        result[key] = blockData;
                    } else {
                        result[key] += ", " + blockData;
                    }
                } else {
                    // single-line value
                    if (result.find(key) == result.end()) {
                        result[key] = remainder;
                    } else {
                        result[key] += ", " + remainder;
                    }
                }
            }
        }
    }

    return result;
}

// Driver function that you can call if you always parse the same keys.
// If in the future you want to parse a different set of keys, you can
// create additional driver functions or pass 'keys' as a parameter instead.
map<string, string> driverParseBlockValues(const vector<string> &block)
{
    // Suppose these are the keys you want to parse:
    // "birth", "ethnicity", "faith", "dynasty_house", "dna", "spouse",
    // "former_spouse", "child", "date", "reason", "domain"
    vector<string> keys = {
        "first_name",
        "birth",
        "ethnicity",
        "faith",
        "dynasty_house",
        "dna",
        "spouse",
        "former_spouses",    // or "former_spouses" if that is the correct name in your text
        "child",
        "date",
        "reason",
        "domain"
    };

    // Simply call parseBlockValues with those keys
    map<string, string> parsed = parseBlockValues(block, keys);
    return parsed;
}