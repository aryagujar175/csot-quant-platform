#include <iostream>
#include <chrono>
#include "strategy.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <deque>
#include "histogram.hpp"
#include <unordered_map>
using namespace std;

class Engine {
private:
    deque<string> symbol_storage;
    unordered_map<string, string_view> symbol_lookup;

public:
    vector<csot::Tick> load_ticks(string filename) {
        vector<csot::Tick> ans;
        fstream csvf(filename, ios::in);
        
        if (csvf.is_open()) {
            string s;
            getline(csvf, s); // Safely skip header

            while (getline(csvf, s)) {
                string pt = "";
                int ptr = 0;

                // 1. timestamp_ns
                while (ptr < s.length()) {
                    if (s[ptr] == ',') break;
                    pt += s[ptr];
                    ptr++;
                }
                uint64_t n1 = stoull(pt);
                
                // 2. symbol
                pt = "";
                ptr++;
                while (ptr < s.length()) {
                    if (s[ptr] == ',') break;
                    pt += s[ptr];
                    ptr++;
                }

                if (symbol_lookup.find(pt) == symbol_lookup.end()) {
                    symbol_storage.push_back(pt);
                    symbol_lookup[pt] = symbol_storage.back();
                }
                string_view n2 = symbol_lookup[pt];

                // 3. bid_px
                pt = "";
                ptr++;
                while (ptr < s.length()) {
                    if (s[ptr] == ',') break;
                    pt += s[ptr];
                    ptr++;
                }
                double n3 = stod(pt);

                // 4. ask_px
                pt = "";
                ptr++;
                while (ptr < s.length()) {
                    if (s[ptr] == ',') break;
                    pt += s[ptr];
                    ptr++;
                }
                double n4 = stod(pt);

                // 5. bid_qty
                pt = "";
                ptr++;
                while (ptr < s.length()) {
                    if (s[ptr] == ',') break;
                    pt += s[ptr];
                    ptr++;
                }
                uint32_t n5 = stoul(pt);

                // 6. ask_qty (Fixed the duplicate n5 block)
                pt = "";
                ptr++;
                while (ptr < s.length()) {
                    if (s[ptr] == ',' || s[ptr] == '\r') break;
                    pt += s[ptr];
                    ptr++;
                }
                uint32_t n6 = stoul(pt);

                csot::Tick tt = {n1, n2, n3, n4, n5, n6};
                ans.push_back(tt);
            }
            csvf.close();
        }
        return ans;
    }

    // Inside engine.cpp (part of your Engine class)
    void run_replay(csot::Strategy* strategy, const vector<csot::Tick>& ticks) {
        csot::LatencyHistogram hist;

        for (const auto& tick : ticks) {
            auto t1 = chrono::steady_clock::now();
            strategy->on_tick(tick);
            auto t2 = chrono::steady_clock::now();
        
            hist.record(chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count());
        }

        hist.print(cout);
}
};