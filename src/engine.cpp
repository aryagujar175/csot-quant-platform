#include <iostream>
#include "strategy.hpp"
#include <fstream>
#include <vector>
#include <string>

using namespace std;

vector<csot::Tick> load_ticks(string filename) {
    vector<csot::Tick> ans;
    fstream csvf(filename, ios::in);
    string s;
    getline(csvf, s);

    if (csvf.is_open()){
        while (getline(csvf, s)){
            //Processing
            int comas = 0;
            
        }
        csvf.close();
    }

    return ans;
}