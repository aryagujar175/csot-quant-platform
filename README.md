This is a setup that executes a fixed strategy with low latency (p99 = 48 ns).
This was created as a part of CAIC Summer of Tech (CSOT) IIT Delhi, Low- Latency Track.
This is (as of 6 June 2026), the first week's output.

Building instructions (for Linux, things get really messy in WSL2):
With this folder opened in the terminal, 

1. cmake -B build && cmake --build build -j
2. Depending on how you want to run it and which benchmarks you want to use (if any), you can use any of 
a. /build/quant_runner ./build/spec_strategy.so data/public.csv (This uses the histogram.hpp provided)
b. ./build/quant_bench ./build/spec_strategy.so data/synthetic_small.csv (This uses Google Benchmark)

These are the numbers obtained from running the following for performance benchmarks:
perf stat -d -d -d ./build/quant_bench ./build/spec_strategy.so data/synthetic_small.csv (you can replace quant_bench with quant_runner)


 Performance counter stats for './build/quant_bench ./build/spec_strategy.so data/synthetic_small.csv':

                 7      context-switches                 #     10.3 cs/sec  cs_per_second     
                 1      cpu-migrations                   #      1.5 migrations/sec  migrations_per_second
               478      page-faults                      #    701.1 faults/sec  page_faults_per_second
            681.79 msec task-clock                       #      1.0 CPUs  CPUs_utilized       
         1,070,195      cpu_core/L1-dcache-load-misses/  #      nan %  l1d_miss_rate            (30.26%)
     <not counted>      cpu_core/L1-icache-load-misses/  #      nan %  l1i_miss_rate          
            28,704      cpu_core/LLC-loads/              #     39.6 %  llc_miss_rate            (15.25%)
         5,823,076      cpu_core/branch-misses/          #      0.4 %  branch_miss_rate         (22.88%)
     1,586,900,867      cpu_core/branches/               #   2327.5 M/sec  branch_frequency     (30.50%)
     2,696,924,488      cpu_core/cpu-cycles/             #      4.0 GHz  cycles_frequency       (38.12%)
     9,623,103,953      cpu_core/instructions/           #      3.6 instructions  insn_per_cycle  (45.75%)
     2,924,133,884      cpu_core/dTLB-loads/             #      0.0 %  dtlb_miss_rate           (45.75%)
        32,955,137      cpu_atom/L1-icache-load-misses/  #     16.0 %  l1i_miss_rate            (0.06%)
         1,789,970      cpu_atom/LLC-loads/              #      9.4 %  llc_miss_rate            (0.21%)
         6,030,208      cpu_atom/branch-misses/          #      3.3 %  branch_miss_rate         (0.35%)
       314,447,794      cpu_atom/branches/               #    461.2 M/sec  branch_frequency     (0.44%)
       951,597,608      cpu_atom/cpu-cycles/             #      1.4 GHz  cycles_frequency       (0.58%)
     2,163,897,452      cpu_atom/instructions/           #      2.3 instructions  insn_per_cycle  (0.58%)
       566,499,253      cpu_atom/dTLB-loads/             #      0.0 %  dtlb_miss_rate           (0.60%)
             TopdownL1 (cpu_core)                        #     12.8 %  tma_bad_speculation    
                                                         #     20.2 %  tma_frontend_bound       (60.94%)
                                                         #     11.1 %  tma_backend_bound      
                                                         #     56.0 %  tma_retiring             (60.94%)
             TopdownL1 (cpu_atom)                        #     11.1 %  tma_backend_bound        (0.31%)
                                                         #     25.9 %  tma_frontend_bound       (0.17%)
                                                         #     28.3 %  tma_bad_speculation    
                                                         #     34.7 %  tma_retiring             (0.02%)

       0.683350979 seconds time elapsed

       0.678294000 seconds user
       0.005002000 seconds sys

Headline latency numbers:
p50 = 21 ns (reference: 152 ns)
p99 = 48 ns (reference: 176 ns)
p999 = 104 ns 
Throughput = 18.81 M/s
(Ranked #3 in the CSOT week 1 leaderboard out of 37 participants)

This was my introduction into low-latency development. What surprised me most was:

1. The fact that dividing by 64.0 is much more heavier on the CPU than multiplying by 0.015625, and that multiplying by 0.015625 and by 64 is equally heavy on the CPU. A significant part of my improvement over the reference for p50 can be attributed to this. Also, sqrt also is really time-consuming. 

2. Deleting my build folder and restoring it by cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo, and then building it again in the standard way reduced p99 by 25 ns, while not at all affecting p50 or p999 noticably. Frankly, I have absolutely zero idea how and why.


