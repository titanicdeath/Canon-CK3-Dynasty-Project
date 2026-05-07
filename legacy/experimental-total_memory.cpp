#include <windows.h>
#include <psapi.h>
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
#include <typeinfo>
#include <stdexcept>
#include <algorithm>



using namespace std;
using json = nlohmann::json;

string formatWithCommas(const string &numStr) {
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

string byteFormat(size_t bytes){
    size_t operation = 0;
    size_t place[5] = {bytes, 0, 0, 0, 0};
    string byteType[5] = {"bytes", "kilobytes", "megabytes", "gigabytes", "petabytes"};
    for (size_t i = 0; i < 4; i++){
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
    for (size_t i = 0; i < 4; i++){
        current = to_string(place[i]);
        current = formatWithCommas(current);
        formattedValues[i] = current;
    }
    

    string formatted = "";
    for (size_t i = 0; i < 4; i++){
        if (place[i] != 0){
            formatted = formatted + "   |   " + formattedValues[i] + " " + byteType[i];
        }
        else {
            formatted = formatted + "   |";
            break;
        }
    }
    
    return formatted;

}

// Helper function: Get current memory usage (WorkingSetSize) in bytes.
string getMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        size_t memory = pmc.WorkingSetSize;
        string format = byteFormat(memory);
        //cout << format << endl;
        return format; // in bytes
    }
    throw runtime_error("GetProcessMemoryInfo failed");
}

vector<string> totalAssimilation(const string &filename){
    vector<string> block;
    ifstream parseStart(filename);
    
    if (!parseStart.is_open()){
        cerr << "[ERROR] Could not open file: " << filename << endl;
        return block;
    }

    string line;
    cout << "[BEFORE] Size of vector parsing in bytes: " << sizeof(block) << endl;
    
    while (getline(parseStart, line)){
        block.push_back(line);
    }

    cout << "[AFTER] Size of vector parsing in bytes: " << sizeof(block) << endl;
    return block;
}

void pointerPlay(){
    string keys = " 0123456789+-=/\\.,[]{}AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvSsWwXxYyZz!?\"\'\b\n\r\t\v";
    string name = "Alden";
    int unsegLength = size(keys);
    char* pointerKeys[unsegLength];
    char* letterName[size(name)];
    map<char, int> lexicon;

    for (int i = 0; i < unsegLength; i++){
        char curKey = keys[i];
        pointerKeys[i] = &keys[i];
        lexicon[curKey] = i;
    }

    for (int i = 0; i < size(name); i++){
        char curletter = name[i];
        letterName[i] = pointerKeys[lexicon.at(curletter)];
        cout << "Size of " << curletter << ": " << sizeof(curletter) << endl;
        cout << "Size of " << letterName[i] << ": " << sizeof(letterName[i]) << endl;
        cout << "Size of " << &letterName[i] << ": " << sizeof(&letterName[i]) << endl;
        cout << "Size of " << *letterName[i] << ": " << sizeof(*letterName[i]) << endl << endl;
        //cout << type_info(letterName[i]).name() << endl;
        //cout << curletter << " : " << lexicon.at(curletter) << endl; 
    }
    /*
    char* why;
    char **huh;
    char interesting;
    for (int i = 0; i < size(name); i++){
        why = letterName[i];
        cout << why << " ";
        //cout << sizeof(why) << " ";
    }
    cout << endl;
    cout << endl;
    for (int i = 0; i < size(name); i++){
        huh = &letterName[i];
        cout << huh << " ";
        //cout << sizeof(huh) << " ";
    }
    cout << endl;
    cout << endl;
    for (int i = 0; i < size(name); i++){
        interesting = *letterName[i];
        cout << interesting << " ";
        //cout << sizeof(interesting) << " ";
    }
    cout << "|" <<endl;
    cout << endl;
    */
    // cout << letterName[0] << " " << letterName[1] << " " << letterName[2] << " " << letterName[3] << " " << letterName[4] << " " << endl;
    // cout << &letterName[0] << " " << &letterName[1] << " " << &letterName[2] << " " << &letterName[3] << " " << &letterName[4] << " " << endl;
    // cout << *letterName[0] << " " << *letterName[1] << " " << *letterName[2] << " " << *letterName[3] << " " << *letterName[4] << " " << endl;
 

    

    // cout << "keys: " << keys << endl;
    // cout << "Length: " << unsegLength << endl;
    // cout << "Size: " << sizeof(unsegLength) << endl;
    // string a = "a", b = "b", c = "c", d = "d", e = "e";
    // char* ptrA = &keys[22];
    // char* ptrL = &keys[45];
    // char* ptrD = &keys[29];
    // char* ptrE = &keys[31];
    // char* ptrN = &keys[49];

}

void sample(){
    string speech = "Growing up as a military brat, I didn't just learn about service-I lived it. My father, a US Marine Corps Colonel, served for 26 years, and by the time I was 12, I had already moved across six states and one country. Constant relocation didn't just make me adaptable; it made me resilient, resourceful, and highly capable of integrating into new environments-key traits for any role in defense, military, or intelligence work. I understand the culture, the mission, and the mindset because I was raised in it. I don't just respect structure and discipline-I thrive in it. Unlike others who may need time to adjust, I can hit the ground running, bringing a lifetime of firsthand experience in a world dedicated to security, commitment, and service. I don't just want to serve-I'm already built for it";
    
    string line = "";
    vector<string> paragraph;
    for (char c : speech) {
        line.push_back(c);
        if (c == '.') {
            paragraph.push_back(line);
            line.clear();
        }
    }

    // Display the capacity of each line's string, which indicates how many characters are allocated
    for (size_t i = 0; i < paragraph.size(); i++){
        string &currentLine = paragraph[i];
        cout << "Line " << i+1 << ": " << currentLine << "\n";
        cout << "  Allocated memory (capacity): " << currentLine.capacity() << " bytes\n";
    }
}
 



void NEWpointerPlay(const string &filename) {
    try {
        // Record memory usage before loading the file.
        string memoryBefore = getMemoryUsage();
        cout << "Memory usage before loading file: \t\t" << memoryBefore << endl;

        vector<string> test;
        test = totalAssimilation(filename);

        // Record memory usage after loading the file.
        string memoryAfterLoad = getMemoryUsage();
        cout << "Memory usage after loading file: \t\t" << memoryAfterLoad << endl;

        // Split into lines using string_views (no data copying).
        // vector<string_view> lines = splitLines(fileBuffer);
        // cout << "Number of lines: " << lines.size() << "\n";

        // Record memory usage after splitting lines.
        string memoryAfterSplit = getMemoryUsage();
        cout << "Memory usage after splitting into lines: \t" << memoryAfterSplit << endl;

        // For demonstration, print the first few lines.
        // for (size_t i = 0; i < min<size_t>(5, lines.size()); i++) {
        //     cout << "Line " << i + 1 << ": " << lines[i] << "\n";
        // }
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
    }
}

int main() {
    // The text-based gamestate file (uncompressed from CK3 save)
    //cout << "Current Path:  " << filesystem::current_path() << endl; 
    //cout << typeid(filesystem::current_path()).name() << endl;
    // Move up one directory with ".." and then pointerKeyserence the file
    string inputFile = "../gamestate";
    string rootID = "37676";
    string sustain;
    bool asim = false;
    sample();
    //pointerPlay();
    //NEWpointerPlay(inputFile);
    if (asim == true){
        vector<string> test;

        cout << "Enter anything to begin\n> ";
        cin >> sustain;
    
        test = totalAssimilation(inputFile);
    
        cout << "Enter anything to end\n> ";
        cin >> sustain;
    }
    return 0;
}


/*
1. Pass line
2. Discet line
3. Pass individual char to mapping function
4. Char maps to pointer


*/