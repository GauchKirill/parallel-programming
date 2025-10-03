#include "../include/argparse.hpp"
#include <omp.h>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>

std::tuple<uint16_t, uint32_t> getParam(int argc, char* argv[])
{
    argparse::ArgumentParser parser("Balance", "1.0.0");

    parser.add_argument("-t", "--threads")
        .help("Num of threads")
        .default_value(static_cast<uint16_t>(omp_get_max_threads()))
        .scan<'u', uint16_t>();

    parser.add_argument("-i", "--iterations")
        .help("Num of iteratuons")
        .default_value(uint32_t{129})
        .scan<'u', uint32_t>();

    try
    {
        parser.parse_args(argc, argv);
        
        if (parser.is_used("--help")) {
            std::cout << parser.help().str();
            std::exit(0);
        }
        
        uint16_t threads = parser.get<uint16_t>("-t");
        uint32_t iterations = parser.get<uint32_t>("-i");
        
        return {threads, iterations};
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        std::exit(1);
    }
}

void showResults(const std::vector<std::vector<uint32_t>> &results, std::string disrcription);

int main(int argc, char* argv[])
{
    auto [numThreads, numIt] = getParam(argc, argv);

    uint16_t numChanks[3] = {1, 2, 4};
    std::vector<std::vector<uint32_t>> results;
    results.clear();
    results.reserve(numThreads);
    results.resize(numThreads);

    std::vector<uint32_t> threadBuf;

    #pragma omp parallel private(threadBuf)
    {
        #pragma omp for
        for (uint32_t i = 0; i < numIt; ++i)
        {
            threadBuf.push_back(i);
        }
        uint numThread = omp_get_thread_num();

        #pragma omp critical
        {
            results[numThread] = std::move(threadBuf);
            threadBuf.clear();
        }

        #pragma omp single
        showResults(results, std::string("Поведение по умолчанию"));

        //-------------------------------
        // results.clear();
        results.reserve(numThreads);
        results.resize(numThreads);

        #pragma omp barier

        #pragma omp for schedule(static, numChanks[0])
        for (uint32_t i = 0; i < numIt; ++i)
        {
            threadBuf.push_back(i);
        }

        #pragma omp critical
        {
            results[numThread] = std::move(threadBuf);
            threadBuf.clear();
        }

        #pragma omp single
        showResults(results, std::string("\nStatic chanks: " + std::to_string(numChanks[0])));

        //------------------------
        // results.clear();
        results.reserve(numThreads);
        results.resize(numThreads);
        
        #pragma omp barier

        #pragma omp for schedule(static, numChanks[1])
        for (uint32_t i = 0; i < numIt; ++i)
        {
            threadBuf.push_back(i);
        }

        #pragma omp critical
        {
            results[numThread] = std::move(threadBuf);
            threadBuf.clear();
        }

        #pragma omp single
        showResults(results, std::string("\nStatic chanks: " + std::to_string(numChanks[1])));

        //------------------------
        // results.clear();
        results.reserve(numThreads);
        results.resize(numThreads);
        
        #pragma omp barier

        #pragma omp for schedule(static, numChanks[2])
        for (uint32_t i = 0; i < numIt; ++i)
        {
            threadBuf.push_back(i);
        }

        #pragma omp critical
        {
            results[numThread] = std::move(threadBuf);
            threadBuf.clear();
        }

        #pragma omp single
        showResults(results, std::string("\nStatic chanks: " + std::to_string(numChanks[2])));

        //-------------------------------
        // results.clear();
        results.reserve(numThreads);
        results.resize(numThreads);
        
        #pragma omp barier

        #pragma omp for schedule(dynamic, numChanks[0])
        for (uint32_t i = 0; i < numIt; ++i)
        {
            threadBuf.push_back(i);
        }

        #pragma omp critical
        {
            results[numThread] = std::move(threadBuf);
            threadBuf.clear();
        }

        #pragma omp single
        showResults(results, std::string("\nDynamic chanks: " + std::to_string(numChanks[0])));

        //------------------------
        // results.clear();
        results.reserve(numThreads);
        results.resize(numThreads);
        
        #pragma omp barier

        #pragma omp for schedule(dynamic, numChanks[1])
        for (uint32_t i = 0; i < numIt; ++i)
        {
            threadBuf.push_back(i);
        }

        #pragma omp critical
        {
            results[numThread] = std::move(threadBuf);
            threadBuf.clear();
        }

        #pragma omp single
        showResults(results, std::string("\nDynamic chanks: " + std::to_string(numChanks[1])));

        //------------------------
        // results.clear();
        results.reserve(numThreads);
        results.resize(numThreads);
        
        #pragma omp barier

        #pragma omp for schedule(dynamic, numChanks[2])
        for (uint32_t i = 0; i < numIt; ++i)
        {
            threadBuf.push_back(i);
        }

        #pragma omp critical
        {
            results[numThread] = std::move(threadBuf);
            threadBuf.clear();
        }

        #pragma omp single
        showResults(results, std::string("\nDynamic chanks: " + std::to_string(numChanks[2])));

        //------------------------
        // results.clear();
        results.reserve(numThreads);
        results.resize(numThreads);
        
        #pragma omp barier

        #pragma omp for schedule(guided, 4)
        for (uint32_t i = 0; i < numIt; ++i)
        {
            threadBuf.push_back(i);
        }

        #pragma omp critical
        {
            results[numThread] = std::move(threadBuf);
            threadBuf.clear();
        }

        #pragma omp single
        showResults(results, std::string("\nGuided"));
    }
}

void showResults(const std::vector<std::vector<uint32_t>> &results, std::string disrcription)
{
    std::cout << disrcription << std::endl;
    size_t numThreads = results.size();
    for (size_t thrIdx = 0; thrIdx < numThreads; ++thrIdx)
    {
        std::cout << "Поток " << thrIdx << ":\n";
        auto &buf = results[thrIdx];
        for (size_t i = 0; i < buf.size(); ++i)
        {
            std::cout << buf[i] << " ";
        }
        std::cout << std::endl;
    }
}
