#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <sstream>
#include "json.hpp"  // Single-header nlohmann JSON

using namespace std;
using json = nlohmann::json;

/*
  1) A global or static debug level. 
     NONE=0 -> No debug prints
     LOW=1  -> Basic messages
     HIGH=2 -> More detailed
*/
static int g_debugLevel = 1; // Adjust as needed

// Our centralized debug print function
void debugPrint(const std::string &msg, int level=1)
{
    if (level <= g_debugLevel) // Print only if the requested level <= global setting
    {
        std::cout << msg << std::endl;
    }
}

/*
  This is your function that finds the start and end line of the character block.
  We assume you already have it implemented exactly as you provided (with brace counting).
  We just declare it here so we can use it below.
*/
vector<int> findBlock(const string &filename, const string &charID);

/*
  Global set to track which characters we've already processed,
  so we don't parse the same ID more than once in BFS.
*/
static unordered_set<string> g_visited;

/*
  Helper: remove outer quotes if present.
*/
static inline string stripQuotes(const string &s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
    {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

/*
  Read lines [startLine..endLine] from `filename` and return them as a single string.
  We do this after findBlock(...) gives us the range of the character's block.
*/
string getBlockTextByLines(const string &filename, int startLine, int endLine)
{
    ifstream in(filename);
    if (!in.is_open())
    {
        debugPrint("[ERROR] Could not open file: " + filename, 0);
        return "";
    }

    // We'll read line by line, capturing only the range [startLine..endLine].
    // If startLine or endLine are 0, or startLine > endLine, we just return "".
    if (startLine <= 0 || endLine <= 0 || endLine < startLine)
    {
        debugPrint("[WARN] Invalid line range: start=" + to_string(startLine) 
                   + ", end=" + to_string(endLine), 1);
        return "";
    }

    ostringstream blockBuffer;
    int lineNum = 0;
    string line;
    while (getline(in, line))
    {
        lineNum++;
        if (lineNum < startLine) continue;
        if (lineNum > endLine) break;

        // remove trailing \r if present
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        blockBuffer << line << "\n";
    }

    in.close();
    return blockBuffer.str();
}

/*
  Parse a single character block text to extract relevant data:
    - first_name, dna, ethnicity, faith, dynasty_house
    - mother, father
    - spouses
    - children
    - if we see "dead_data={", parse domain, date (death_date), reason (death_reason)
  Return it as JSON.
*/
json parseCharacterBlock(const string &charID, const string &blockText)
{
    debugPrint("[DEBUG] parseCharacterBlock: ID=" + charID, 1);

    json jResult;
    jResult["char_id"]       = charID;
    jResult["first_name"]    = "";
    jResult["dna"]           = "";
    jResult["ethnicity"]     = "";
    jResult["faith"]         = "";
    jResult["dynasty_house"] = "";
    jResult["mother"]        = "";
    jResult["father"]        = "";
    jResult["spouses"]       = json::array();
    jResult["children"]      = json::array();
    jResult["domain"]        = json::array();
    jResult["death_date"]    = "";
    jResult["death_reason"]  = "";

    // Regex for lines like:  key = value
    regex kvRegex(R"(^\s*([A-Za-z0-9_]+)\s*=\s*(.+)$)");

    bool inDeadData = false;
    int  deadDataBrace = 0;

    istringstream iss(blockText);
    string line;
    while (getline(iss, line))
    {
        // remove trailing \r
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        // Check if we enter "dead_data={"
        if (line.find("dead_data={") != string::npos)
        {
            inDeadData = true;
            for (char c : line)
            {
                if (c == '{') deadDataBrace++;
                if (c == '}') deadDataBrace--;
            }
        }
        else if (inDeadData)
        {
            // Keep track of nested braces within dead_data
            for (char c : line)
            {
                if (c == '{') deadDataBrace++;
                if (c == '}') deadDataBrace--;
            }
            if (deadDataBrace <= 0)
            {
                inDeadData = false;
            }
        }

        smatch m;
        if (regex_search(line, m, kvRegex))
        {
            // key= m[1], val= m[2]
            string key = m[1].str();
            string val = stripQuotes(m[2].str());

            // straightforward key checks
            if (key == "first_name")
            {
                jResult["first_name"] = val;
            }
            else if (key == "dna")
            {
                jResult["dna"] = val;
            }
            else if (key == "ethnicity")
            {
                jResult["ethnicity"] = val;
            }
            else if (key == "faith")
            {
                jResult["faith"] = val;
            }
            else if (key == "dynasty_house")
            {
                jResult["dynasty_house"] = val;
            }
            else if (key == "mother")
            {
                jResult["mother"] = val;
            }
            else if (key == "father")
            {
                jResult["father"] = val;
            }
            else if (key == "spouse")
            {
                // spouse=ID or spouse={ multiple IDs }
                if (val.find('{') != string::npos)
                {
                    auto startPos = val.find('{');
                    auto endPos   = val.find('}');
                    if (startPos != string::npos && endPos != string::npos)
                    {
                        string inside = val.substr(startPos+1, endPos - startPos - 1);
                        regex ws("\\s+");
                        sregex_token_iterator it(inside.begin(), inside.end(), ws, -1);
                        sregex_token_iterator e;
                        for (; it != e; ++it)
                        {
                            if (!it->str().empty())
                            {
                                jResult["spouses"].push_back(it->str());
                            }
                        }
                    }
                }
                else
                {
                    if (!val.empty()) {
                        jResult["spouses"].push_back(val);
                    }
                }
            }
            else if (key == "child")
            {
                // child=ID or child={ multiple IDs }
                if (val.find('{') != string::npos)
                {
                    auto startPos = val.find('{');
                    auto endPos   = val.find('}');
                    if (startPos != string::npos && endPos != string::npos)
                    {
                        string inside = val.substr(startPos+1, endPos - startPos - 1);
                        regex ws("\\s+");
                        sregex_token_iterator it(inside.begin(), inside.end(), ws, -1);
                        sregex_token_iterator e;
                        for (; it != e; ++it)
                        {
                            if (!it->str().empty())
                            {
                                jResult["children"].push_back(it->str());
                            }
                        }
                    }
                }
                else
                {
                    if (!val.empty()) {
                        jResult["children"].push_back(val);
                    }
                }
            }

            // if we're in dead_data, also check domain, date, reason
            if (inDeadData)
            {
                if (key == "domain")
                {
                    // domain={ 1184 1338 ...}
                    auto startPos = val.find('{');
                    auto endPos   = val.find('}');
                    if (startPos != string::npos && endPos != string::npos)
                    {
                        string inside = val.substr(startPos+1, endPos - startPos - 1);
                        regex ws("\\s+");
                        sregex_token_iterator it(inside.begin(), inside.end(), ws, -1);
                        sregex_token_iterator e;
                        for (; it != e; ++it)
                        {
                            if (!it->str().empty())
                            {
                                jResult["domain"].push_back(it->str());
                            }
                        }
                    }
                }
                else if (key == "date")
                {
                    jResult["death_date"] = val;
                }
                else if (key == "reason")
                {
                    jResult["death_reason"] = val;
                }
            }
        }
    }

    debugPrint("[DEBUG] parseCharacterBlock done for " + charID 
               + " (first_name=" + string(jResult["first_name"]) + ")", 1);

    return jResult;
}

/*
  BFS: from `startID`, we:
    1) findBlock(...) => get [startLine, endLine] for that char
    2) read lines [startLine..endLine] => blockText
    3) parse => JSON (now includes start_line, end_line)
    4) push the JSON into a buffer
    5) once the buffer has 50 items, write them out to the output file
    6) queue children => repeat
*/
void traverseDynasty(const string &filename, const string &startID, ofstream &out)
{
    debugPrint("[DEBUG] traverseDynasty starting from " + startID, 1);

    // We'll store up to 50 characters in memory, then flush them
    vector<json> partialBuffer;
    partialBuffer.reserve(50);

    auto flushBuffer = [&](vector<json> &buf){
        for (auto &j : buf)
        {
            out << j.dump() << "\n";
        }
        buf.clear();
    };

    queue<string> toVisit;
    toVisit.push(startID);

    while (!toVisit.empty())
    {
        string currentID = toVisit.front();
        toVisit.pop();

        if (g_visited.count(currentID) > 0)
        {
            debugPrint("[DEBUG] " + currentID + " already visited.", 1);
            continue;
        }
        g_visited.insert(currentID);

        // 1) get block lines
        vector<int> lines = findBlock(filename, currentID);
        int startLine = lines.size() > 0 ? lines[0] : 0;
        int endLine   = lines.size() > 1 ? lines[1] : 0;

        if (startLine == 0 || endLine == 0)
        {
            debugPrint("[DEBUG] No block found for " + currentID + ". Skipping.", 1);
            continue;
        }

        // 2) read lines from startLine..endLine
        string blockText = getBlockTextByLines(filename, startLine, endLine);

        // 3) parse
        json jChar = parseCharacterBlock(currentID, blockText);

        // Add the new fields
        jChar["start_line"] = startLine;
        jChar["end_line"]   = endLine;

        // 4) BFS children
        for (auto &childID : jChar["children"])
        {
            string cID = childID.get<string>();
            debugPrint("[DEBUG] " + currentID + " has child " + cID, 1);
            toVisit.push(cID);
        }

        // Summaries
        debugPrint("[DEBUG] Storing char to partial buffer: " + currentID 
                   + " - " + string(jChar["first_name"]), 1);

        // 5) push the JSON line into our buffer
        partialBuffer.push_back(jChar);

        // If partialBuffer >= 50, flush it
        if (partialBuffer.size() >= 50)
        {
            debugPrint("[DEBUG] partialBuffer reached 50 items. Flushing to file...", 1);
            flushBuffer(partialBuffer);
        }
    }

    debugPrint("[DEBUG] BFS done. Flushing any remaining items in partialBuffer...", 1);

    // BFS is over, flush leftover items
    if (!partialBuffer.empty())
    {
        flushBuffer(partialBuffer);
    }

    debugPrint("[DEBUG] traverseDynasty done.", 1);
}

int main()
{
    // Path to your text-based gamestate
    string inputFile = "gamestate";

    // Output JSON lines
    string outputFile = "New_characters.jsonl";

    // Attempt to open (overwrite) the file for writing
    // If you want to append or do existence checks, see previous examples
    ofstream out(outputFile);
    if (!out.is_open())
    {
        cerr << "[ERROR] Could not open " << outputFile << " for writing!\n";
        return 1;
    }

    // The root ID
    string rootID = "37676";

    // Clear visited
    g_visited.clear();

    // BFS
    traverseDynasty(inputFile, rootID, out);

    out.close();
    debugPrint("[DEBUG] Done. Results in " + outputFile, 1);

    return 0;
}

/*
----------------------------------------------------------------------------------
Note: You must provide the definition of 'findBlock' below or in a separate file:

vector<int> findBlock(const string &filename, const string &charID){
    // your existing code that returns [startLine, endLine] for the block
}
----------------------------------------------------------------------------------
*/


/*
----------------------------------------------------------------------------------
Note: You must provide the definition of 'findBlock' below or in a separate file:

vector<int> findBlock(const string &filename, const string &charID){
    // your existing code that returns [startLine, endLine] for the block
}
----------------------------------------------------------------------------------
*/
vector<int> findBlock(const string &filename, const string &charID){
    vector<int> blockBeginEnd(2);
    ifstream parseStart(filename);
    if (!parseStart.is_open()){
        cerr << "[ERROR] Could not open file: " << filename << endl;
        return blockBeginEnd;
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
            blockBeginEnd[0] = startBlock;
            
        }
        if ((lineNumber == (startBlock + 1)) && (startFound == true)){
            //cout << lineStart << endl;
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
    parseStart.close();

    ifstream parseEnd(filename);

    /*
    ALLOW EDITABLE AREA STARTS HERE
    */
    cout << startFound << endl;
    if (startFound == true){
        //cout << "Start Block Line Number: " << startBlock << endl;

        // We already saw an opening '{' on the startPattern line.
        // So let's track braceDepth starting at 1.
        int braceDepth = 0;  

        while (getline(parseEnd, lineEnd)){
            lineNumber += 1;
            // Only start counting once we reach or pass the block's start line.
            if (lineNumber >= startBlock) {
                //cout << "Brace Depth: " << braceDepth << endl;
                //cout << lineEnd << endl;
                stretch += 1;

                // Count any '{' or '}' on this line:
                for (char c : lineEnd) {
                    if (c == '{') braceDepth++;
                    if (c == '}') braceDepth--;
                }

                // If braceDepth returns to 0 or below, we've closed the block.
                if (braceDepth <= 0) {
                    endFound = true;
                    endBlock = lineNumber;
                    blockBeginEnd[1] = endBlock;
                    break;
                }

                // Safety limit
                if (stretch >= 10000){
                    cout << "TOO LONG" << endl;
                    break;
                }
            }
        }
    }
    /*
    ALLOW EDITABLE AREA END HERE
    */
    // else {
    //     cout << "Char ID Block not found-- " << lineNumber << endl;
    // }

    // if (endFound == true){
    //     cout << "End Block Line Number: " << endBlock << endl;
    // }
    // else {
    //     cout << "End Block not found" << endl;
    // }
    parseEnd.close();
    return blockBeginEnd;
}