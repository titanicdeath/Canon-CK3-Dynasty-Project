#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

vector<json> readJsonlFile(const string& filename) {
    vector<json> entries;
    ifstream infile(filename);
    string line;

    while (getline(infile, line)) {
        if (!line.empty()) {
            try {
                json j = json::parse(line);
                entries.push_back(j);
            } catch (const json::parse_error& e) {
                cerr << "JSON parse error: " << e.what() << endl;
            }
        }
    }

    return entries;
}



int main() {
    // The text-based gamestate file (uncompressed from CK3 save)
    //cout << "Current Path:  " << filesystem::current_path() << endl; 
    //cout << typeid(filesystem::current_path()).name() << endl;
    // Move up one directory with ".." and then reference the file
    string files[4] = {"character-archive_1.jsonl", "character-archive_2.jsonl","character-archive_3.jsonl", "character-archive_4.jsonl"};
    string pave = "../character-archive/";
    string cur;

    for (int i = 0; i < 4; i++){
        cur = pave + files[i];
        files[i] = cur;
        cout << files[i] << endl;
    }

    for (int i = 0; i < 4; i++){
        ifstream verify(files[i]);
        if (!verify.is_open()){
            cerr << "[ERROR] Could not open file: " << files[i] << endl;
        }
        else {
            cerr << "[Pass] file: " << files[i] << endl;
        }
        verify.close();
    }


    return 0;
}