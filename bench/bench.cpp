#include <benchmark/benchmark.h>
#include <dlfcn.h>
#include <iostream>
#include "../src/engine.cpp"

using namespace std;

// Global state for the benchmark
csot::Strategy* global_strategy = nullptr;
vector<csot::Tick> global_ticks;

// The Hot Path Benchmark
static void BM_StrategyOnTick(benchmark::State& state) {
    if (global_ticks.empty() || !global_strategy) {
        state.SkipWithError("Data or strategy failed to load prior to benchmark.");
        return;
    }

    size_t i = 0;
    size_t n = global_ticks.size();

    for (auto _ : state) {
        auto orders = global_strategy->on_tick(global_ticks[i]);
        benchmark::DoNotOptimize(orders);
        benchmark::ClobberMemory;
        i = (i + 1) % n;
    }
}
BENCHMARK(BM_StrategyOnTick);

// Custom benchmark main to load the .so and CSV first
int main(int argc, char** argv) {
    if (argc < 3) {
        cout << "Usage: ./quant_bench <strategy.so> <ticks.csv> [benchmark_args...]\n";
        return 1;
    }

    string strategy_path = argv[1];
    string csv_path = argv[2];

    Engine engine;
    global_ticks = engine.load_ticks(csv_path);

    void* handle = dlopen(strategy_path.c_str(), RTLD_NOW);
    if (!handle) {
        cout << "Fatal: " << dlerror() << '\n';
        return 1;
    }

    using make_strategy_fn = csot::Strategy* (*)();
    auto make_strategy = (make_strategy_fn)dlsym(handle, "create_strategy");
    
    global_strategy = make_strategy();
    global_strategy->on_init();

    // Run Google Benchmark
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    delete global_strategy;
    dlclose(handle);

    return 0;
}