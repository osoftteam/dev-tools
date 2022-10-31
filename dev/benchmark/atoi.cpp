#include <benchmark/benchmark.h>
#include <iostream>
#include "dev-utils.h"

static std::array<std::string, 10> arr = {"1000", "20001", "30001", "4000", "50001", "60001", "7000", "80001", "90001", "1000001"};


std::vector<std::string> create_str_test_vector(size_t num)
{
    std::vector<std::string> rv;
    rv.resize(num);
    size_t n = 0;
    std::generate(rv.begin(), rv.end(), [&n]()
        {
            ++n;
            return std::to_string(1000 + n);
        });
    return rv;
}

static void BM_stoi(benchmark::State& state)
{
    for (auto _ : state){
        auto v = std::stoi("10002");
        benchmark::DoNotOptimize(v);
    }
}

static void BM_stoui(benchmark::State& state) {
    for (auto _ : state){
        auto v = dev::stoui("10002");
        benchmark::DoNotOptimize(v);
    }
}
 

static void BM_stoi_loop(benchmark::State& state) {
    for (auto _ : state){
        for(const auto& s : arr){
            auto v = std::stoi(s);
            benchmark::DoNotOptimize(v);
        }
    }
} 

static void BM_stoui_loop(benchmark::State& state) {
    for (auto _ : state){
        for(const auto& s : arr){
            auto v = dev::stoui(s);
            benchmark::DoNotOptimize(v);
        }
    }
}

 

static void BM_stoi_big_loop(benchmark::State& state) {
    auto big_vec = create_str_test_vector(state.range(0));

    for (auto _ : state){
        for(const auto& s : big_vec){
            auto v = std::stoi(s);
            benchmark::DoNotOptimize(v);
        }
    }
}

 

static void BM_stoui_big_loop(benchmark::State& state) {
    auto big_vec = create_str_test_vector(state.range(0));

    for (auto _ : state){
        for(const auto& s : big_vec){
            auto v = dev::stoui(s);
            benchmark::DoNotOptimize(v);
        }
    }
}

BENCHMARK(BM_stoi);
BENCHMARK(BM_stoui);
BENCHMARK(BM_stoi_loop);
BENCHMARK(BM_stoui_loop);
BENCHMARK(BM_stoi_big_loop)->DenseRange(1000,10000,2000);
BENCHMARK(BM_stoui_big_loop)->DenseRange(1000,10000,2000);


void generate_strings()
{   
    auto big_vec = create_str_test_vector(1000);
    std::cout << "generated-vec [size=" << big_vec.size() << "]";
    int num = 0;
    for(const auto& s : big_vec){
        std::cout << s << " ";
        ++num;
        if(num > 5)break;
    }
    std::cout << std::endl;

    std::vector<std::string> arr = {"5", "50", "50000", "1234567"};
    for(const auto& s : arr)
    {
        std::cout << s << " => " << dev::stoui(s) << std::endl;
    }
};

int main(int argc, char** argv)
{
    generate_strings();
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}
