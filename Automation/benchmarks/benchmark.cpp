#include <benchmark/benchmark.h>

// Funktion som ska benchmarkas
static void BM_StringCreation(benchmark::State& state) {
    for (auto _ : state) {
        std::string empty_string;
    }
}
BENCHMARK(BM_StringCreation);

// En annan funktion som ska benchmarkas
static void BM_StringCopy(benchmark::State& state) {
    std::string x = "hello";
    for (auto _ : state) {
        std::string copy(x);
    }
}
BENCHMARK(BM_StringCopy);

BENCHMARK_MAIN();
