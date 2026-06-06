#include <iostream>
#include "strategy.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <deque>
#include <unordered_map>
using namespace std;

class null_strategy: public csot::Strategy{
    public:
        void on_init() override{
            //
        }
        
        vector<csot::Order> on_tick (const csot::Tick& t) override {
            return {};
        }

        void on_fill(const csot::Order& o,
                         double        fill_price,
                         uint32_t      fill_qty) override {
            //Do nothing
        }
};

extern "C" csot::Strategy* create_strategy(){
    return new null_strategy();
}

