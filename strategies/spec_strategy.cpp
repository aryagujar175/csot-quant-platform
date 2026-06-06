#include <iostream>
#include "strategy.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <cmath>
#include <xmmintrin.h>
#include <pmmintrin.h>
using namespace std;
#include <new>
#include <cstddef>

struct alignas(64) SymbolState {
    double sum = 0.0;
    double sqsum = 0.0;
    uint32_t count = 0;
    uint32_t head = 0;
    int32_t position = 0;
    uint8_t _pad[36];
    double mids[64] = {0.0};
};

class spec_strategy: public csot::Strategy{
    private:
    // Replaced unordered_map with a fixed array to satisfy the spec's memory 
    // requirement of zero heap allocations during on_tick.
    SymbolState states[4];
    string_view known_symbols[4];
    size_t num_symbols = 0;

    __attribute__((always_inline)) inline SymbolState& state_for(string_view symbol) {
        // Cache the raw memory address pointer locally to avoid repeated struct member access
        const char* target_ptr = symbol.data();

        if (known_symbols[0].data() == target_ptr) [[likely]] return states[0];
        if (known_symbols[1].data() == target_ptr) return states[1];
        if (known_symbols[2].data() == target_ptr) return states[2];
        if (known_symbols[3].data() == target_ptr) return states[3];

        // Cold path: Brand new symbol
        known_symbols[num_symbols] = symbol;
        return states[num_symbols++];
    }
    public:
        void on_init() override {
            _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
            _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

            // Force page faults during initialization, not during live trading
            volatile char* p = reinterpret_cast<volatile char*>(states);
            for (size_t i = 0; i < sizeof(states); i += 64) {
                p[i] = 0; 
            }
        }
        
        __attribute__((flatten, hot))
        vector<csot::Order> on_tick(const csot::Tick& t) override {
            auto& st = state_for(t.symbol);
            const double mid = (t.bid_px + t.ask_px) * 0.5;
            const double oldest_mid = st.mids[st.head];
            const double diff = mid - oldest_mid;

            st.mids[st.head] = mid;
            st.head = (st.head + 1) & 63;       // valid because WINDOW == 64
            st.sum += diff;
            st.sqsum += diff * (mid + oldest_mid);

            if (st.count < 64) [[unlikely]]{
                ++st.count;
                if (st.count < 64) [[likely]] {
                    return {};
                }
            }

            const double sum_sq = st.sum * st.sum;
            const double rhs    = st.sqsum - sum_sq * 0.015625;

            /*if (rhs < 64e-18) [[unlikely]] {
                return {};
            }*/

            if (st.position == 0) [[likely]] {
                const double lhs_base_16 = mid * 4.0 - st.sum * 0.0625;
                if (lhs_base_16 * lhs_base_16 >= rhs) [[unlikely]] {
                    if (mid * 64.0 > st.sum) {
                        return {csot::Order{csot::Order::Side::SELL, t.symbol, t.bid_px, 1}};
                    } else {
                        return {csot::Order{csot::Order::Side::BUY, t.symbol, t.ask_px, 1}};
                    }
                }
                return {};
            }

            const double lhs_base_256 = mid * 16.0 - st.sum * 0.25;
            if (lhs_base_256 * lhs_base_256 <= rhs) [[unlikely]] {
                if (st.position > 0) {
                    return {csot::Order{csot::Order::Side::SELL, t.symbol, t.bid_px, static_cast<uint32_t>(st.position)}};
                } else {
                    return {csot::Order{csot::Order::Side::BUY, t.symbol, t.ask_px, static_cast<uint32_t>(-st.position)}};
                }
}

            return {};
        }

        void on_fill(const csot::Order& o, double fill_price, uint32_t fill_qty) override {
        auto& st = state_for(o.symbol);
        
        if (o.side == csot::Order::Side::BUY) {
            st.position += fill_qty;
        } else if (o.side == csot::Order::Side::SELL) {
            st.position -= fill_qty;
        }
    }
};

extern "C" csot::Strategy* create_strategy(){
    return new spec_strategy();
}

