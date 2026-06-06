#include <iostream>
#include "strategy.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include "engine.cpp"
#include <chrono>
#include <dlfcn.h> // Required for dlopen / dlsym
#include "histogram.hpp"
using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: ./quant_runner <strategy.so> <ticks.csv>\n";
        return 1;
    }

    string strategy_path = argv[1];
    string csv_path = argv[2];

    // 1. Load Data
    Engine engine;
    auto ticks = engine.load_ticks(csv_path);
    if (ticks.empty()) {
        cout << "Failed to load ticks or file is empty.\n";
        return 1;
    }
    cout << "Loaded " << ticks.size() << " ticks.\n";

    // 2. Load Strategy dynamically
    void* handle = dlopen(strategy_path.c_str(), RTLD_NOW);
    if (!handle) {
        cout << "Error loading strategy: " << dlerror() << '\n';
        return 1;
    }

    // Map the factory function
    using make_strategy_fn = csot::Strategy* (*)();
    auto make_strategy = (make_strategy_fn)dlsym(handle, "create_strategy");
    if (!make_strategy) {
        cout << "Error finding create_strategy: " << dlerror() << '\n';
        return 1;
    }

    csot::Strategy* strategy = make_strategy();
    strategy->on_init();

    // 3. The Hot Loop (Measurement)
    engine.run_replay(strategy, ticks);

    // Cleanup
    delete strategy;
    dlclose(handle);

    return 0;
}