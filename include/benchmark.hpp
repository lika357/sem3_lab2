#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "HashDictionary.hpp"
#include "TreeDictionary.hpp"
#include "board.hpp"
#include "search.hpp"

struct BenchRow {
    std::string name;
    int score;
    long long nodes;
    long long hits;
    double ms;
};

struct DictBenchRow {
    std::string name;
    std::string order;
    int count;
    double ms;
};

inline std::vector<BenchRow> RunSearchBenchmark(const Board& board, int depth, int radius) {
    const Algorithm algorithms[2] = {Algorithm::MINIMAX, Algorithm::ALPHABETA};
    const TableType tables[3] = {TableType::NONE, TableType::HASH, TableType::TREE};
    std::vector<BenchRow> rows;
    for(Algorithm algorithm : algorithms) {
        for(TableType type : tables) {
            Searcher searcher(algorithm, type, depth, radius);
            SearchResult result = searcher.Recommend(board, NOUGHT);
            rows.push_back({AlgorithmName(algorithm) + "+" + TableName(type), result.score, result.nodes,
                            result.hits, result.ms});
        }
    }
    return rows;
}

inline double MeasureDictionary(IDictionary<unsigned long long, int>& dict,
                                const std::vector<unsigned long long>& keys) {
    auto start = std::chrono::steady_clock::now();
    for(unsigned long long key : keys) { dict.Add(key, 1); }
    size_t sum = 0;
    for(unsigned long long key : keys) { sum += dict.Get(key); }
    auto end = std::chrono::steady_clock::now();
    if(sum != keys.size()) {
        throw std::runtime_error("Dictionary lost some keys");
    }
    return std::chrono::duration<double, std::milli>(end - start).count();
}

inline std::vector<DictBenchRow> RunDictionaryBenchmark(int count) {
    std::vector<unsigned long long> ascending;
    for(int i = 0; i < count; i++) { ascending.push_back(i); }
    std::vector<unsigned long long> descending(ascending.rbegin(), ascending.rend());
    std::vector<unsigned long long> shuffled = ascending;
    std::mt19937 generator(42);
    std::shuffle(shuffled.begin(), shuffled.end(), generator);

    std::vector<DictBenchRow> rows;
    const std::vector<unsigned long long>* orders[3] = {&ascending, &descending, &shuffled};
    const char* orderNames[3] = {"ascending", "descending", "random"};
    for(int i = 0; i < 3; i++) {
        TreeDictionary<unsigned long long, int> tree;
        rows.push_back({"TREE", orderNames[i], count, MeasureDictionary(tree, *orders[i])});
        HashDictionary<unsigned long long, int> hash;
        rows.push_back({"HASH", orderNames[i], count, MeasureDictionary(hash, *orders[i])});
    }
    return rows;
}

inline void WriteSearchCsv(const std::vector<BenchRow>& rows, const std::string& path) {
    std::ofstream file(path);
    if(!file) {
        throw std::runtime_error("Can not open file " + path);
    }
    file << "mode,score,nodes,hits,ms\n";
    for(const BenchRow& row : rows) {
        file << row.name << "," << row.score << "," << row.nodes << "," << row.hits << "," << row.ms << "\n";
    }
}

inline void WriteDictionaryCsv(const std::vector<DictBenchRow>& rows, const std::string& path) {
    std::ofstream file(path);
    if(!file) {
        throw std::runtime_error("Can not open file " + path);
    }
    file << "structure,order,count,ms\n";
    for(const DictBenchRow& row : rows) {
        file << row.name << "," << row.order << "," << row.count << "," << row.ms << "\n";
    }
}
