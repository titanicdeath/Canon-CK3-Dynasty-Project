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
  Global set to track which characters we've already processed.
  So we don't parse the same character block multiple times or loop infinitely.
*/
static unordered_set<string> g_visited;

/*
  Helper: Remove outer quotes from a string, if present.
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
  On-demand read:
  - Open 'filename'
  - Search line by line for something matching "^\\s*<charID>\\s*=\\s*\\{"
  - Once found, read lines until braces match up.
  - Return that entire chunk as a string.
  - If not found, return "".
*/
string getCharacterBlockFromFile(const string &filename, const string &charID)
{
    cout << "[DEBUG] getCharacterBlockFromFile: Searching for charID=" << charID << " in " << filename << "\n";

    ifstream in(filename);
    if (!in.is_open())
    {
        cerr << "[ERROR] Could not open file: " << filename << endl;
        return "";
    }

    // Build a regex to find, for example, "37676={"
    const string pattern = "^\\s*" + charID + "\\s*=\\s*\\{";
    regex startRegex(pattern);

    bool inBlock = false;
    int braceDepth = 0;
    ostringstream blockBuffer;

    string line;
    while (getline(in, line))
    {
        // Remove trailing \r if present
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (!inBlock)
        {
            // Look for "charID={"
            smatch m;
            if (regex_search(line, m, startRegex))
            {
                cout << "[DEBUG] Found start of block for ID " << charID << ":\n";
                cout << "        " << line << "\n";

                inBlock = true;
                braceDepth = 0;
                blockBuffer.str("");
                blockBuffer.clear();

                // Count braces on this line
                for (char c : line)
                {
                    if (c == '{') braceDepth++;
                    if (c == '}') braceDepth--;
                }

                blockBuffer << line << "\n";
            }
        }
        else
        {
            // Already in the block
            blockBuffer << line << "\n";

            // Count braces
            for (char c : line)
            {
                if (c == '{') braceDepth++;
                if (c == '}') braceDepth--;
            }

            if (braceDepth <= 0)
            {
                // End of this block
                cout << "[DEBUG] End of block for ID " << charID << "\n";
                break;
            }
        }
    }

    in.close();

    // If we never entered the block or we still have braceDepth > 0, 
    // we probably didn't capture the entire block or didn't find it at all.
    if (!inBlock)
    {
        cout << "[DEBUG] Did NOT find a block for ID " << charID << "\n";
        return "";
    }
    if (braceDepth > 0)
    {
        cout << "[WARN] Possibly incomplete block for ID " << charID << ". braceDepth=" << braceDepth << "\n";
        // We still return what we have
    }

    string captured = blockBuffer.str();
    // You can debug-print the entire captured text if you want:
    // cout << "[DEBUG] Captured block text for " << charID << ":\n" << captured << "\n";

    return captured;
}

/*
  Parse a single character block to extract:
    - first_name
    - dna
    - ethnicity
    - faith
    - dynasty_house
    - mother, father
    - spouses
    - children
    - dead_data => domain, date (death_date), reason (death_reason)
  Return it as a JSON.
*/
json parseCharacterBlock(const string &charID, const string &blockText)
{
    cout << "[DEBUG] parseCharacterBlock: ID=" << charID << "\n";

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

    // For lines like:  first_name="Maurikios"
    // We'll use a naive regex that captures "key = value"
    regex kvRegex(R"(^\s*([A-Za-z0-9_]+)\s*=\s*(.+)$)");

    bool inDeadData = false;
    int deadDataBrace = 0;

    istringstream iss(blockText);
    string line;
    while (getline(iss, line))
    {
        // remove trailing \r
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        // We can optionally print every line we parse:
        //cout << "   [DEBUG] block line: " << line << "\n";

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
            string key = m[1].str();
            string val = m[2].str();
            val = stripQuotes(val);

            // debug
            //cout << "   [DEBUG] Found key=" << key << ", val=" << val << "\n";

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
                // spouse=XYZ or spouse={ multiple IDs }
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
                    // single ID
                    if (!val.empty())
                    {
                        jResult["spouses"].push_back(val);
                    }
                }
            }
            else if (key == "child")
            {
                // child=XYZ or child={ ... }
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
                    // single child
                    if (!val.empty())
                    {
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

    cout << "[DEBUG] parseCharacterBlock done for " << charID 
         << " (first_name=" << jResult["first_name"] << ")\n";

    return jResult;
}

/*
  BFS or recursion to traverse the dynasty starting from `startID`.
  Each time we see a new ID, we:
   1) getCharacterBlockFromFile(...) to find the block
   2) parseCharacterBlock(...) to get data
   3) write to outFile
   4) queue the children
*/
void traverseDynasty(const string &filename, const string &startID, ofstream &out)
{
    cout << "[DEBUG] traverseDynasty starting from " << startID << "\n";

    queue<string> toVisit;
    toVisit.push(startID);

    while (!toVisit.empty())
    {
        string currentID = toVisit.front();
        toVisit.pop();

        if (g_visited.count(currentID) > 0)
        {
            // Already processed this ID
            cout << "[DEBUG] " << currentID << " already visited.\n";
            continue;
        }
        g_visited.insert(currentID);

        // Get the block text
        string blockText = getCharacterBlockFromFile(filename, currentID);
        if (blockText.empty())
        {
            cout << "[DEBUG] No block found for " << currentID << ". Skipping.\n";
            continue;
        }

        // Parse
        json jChar = parseCharacterBlock(currentID, blockText);

        // BFS add the children from jChar
        for (auto &childID : jChar["children"])
        {
            string cID = childID.get<string>();
            cout << "[DEBUG] " << currentID << " has child " << cID << "\n";
            toVisit.push(cID);
        }

        // Print a quick summary to console:  "37676 - Maurikios"
        cout << "[DEBUG] Writing to JSON: " << currentID << " - "
             << jChar["first_name"] << "\n";

        // Write the JSON line
        out << jChar.dump() << "\n";
    }

    cout << "[DEBUG] traverseDynasty done.\n";
}

int main()
{
    // The text-based gamestate file (uncompressed from CK3 save)
    string inputFile = "gamestate";

    // Where we write output
    string outputFile = "characters.jsonl";

    // open out file
    ofstream out(outputFile);
    if (!out.is_open())
    {
        cerr << "[ERROR] Could not open " << outputFile << " for writing!\n";
        return 1;
    }

    // The root character ID: e.g. "37676" for Maurikios
    string rootID = "37676";

    // Clear visited set
    g_visited.clear();

    // Start BFS from the root
    traverseDynasty(inputFile, rootID, out);

    out.close();

    cout << "[DEBUG] Done. Results in " << outputFile << endl;
    return 0;
}
