#pragma once

#include "array_sequence.hpp"

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

#include "HashDictionary.hpp"
#include "TreeDictionary.hpp"
#include "board.hpp"
#include "evaluator.hpp"
#include "i_dictionary.hpp"

enum class Algorithm { MINIMAX, ALPHABETA };
enum class TableType { NONE, HASH, TREE };

enum class BoundType {
    Exact = 0,
    Lower = 1,
    Upper = 2
};

struct TableEntry {
    int value = 0;
    size_t depth = 0;  
    BoundType bound = BoundType::Exact;
};

struct SearchResult {
    bool found = false;
    Move move = {0, 0};
    int score = 0;
    size_t nodes = 0; 
    size_t hits = 0;  
    double ms = 0;
};

inline std::string AlgorithmName(Algorithm algorithm) {
    return algorithm == Algorithm::MINIMAX ? "MINIMAX" : "ALPHABETA";
}

inline std::string TableName(TableType type) {
    if(type == TableType::HASH) {
        return "HASH";
    }
    if(type == TableType::TREE) {
        return "TREE";
    }
    return "NONE";
}

class Searcher {
   private:
    const int INF = 1000000000;

    Algorithm algorithm;
    size_t depth;  
    size_t radius; 
    std::unique_ptr<IDictionary<unsigned long long, TableEntry>> table;
    size_t nodes = 0;  
    size_t hits = 0;  

    static CellState Other(CellState player) {
        return player == CellState::Nought ? CellState::Cross : CellState::Nought;
    }

    bool UseTable(const TableEntry& entry, int remaining, int& alpha, int& beta, int& value) const {
        if(entry.depth != static_cast<size_t>(remaining)) {  
            return false;
        }
        value = entry.value;
        if(entry.bound == BoundType::Exact) {
            return true;
        }
        if(entry.bound == BoundType::Lower && entry.value > alpha) {
            alpha = entry.value;
        } else if(entry.bound == BoundType::Upper && entry.value < beta) {
            beta = entry.value;
        }
        return alpha >= beta;
    }

    int Run(Board& board, int remaining, int alpha, int beta, CellState player) {
        nodes++;
        CellState winner = board.Winner();
        if(winner == CellState::Nought) {
            return WIN + remaining;
        }
        if(winner == CellState::Cross) {
            return -(WIN + remaining);
        }
        if(board.IsFull()) {
            return 0;
        }
        if(remaining == 0) {
            return Evaluate(board);
        }

        unsigned long long key = board.GetKey(player);
        int origAlpha = alpha;
        int origBeta = beta;
        if(table) {
            TableEntry* entry = table->Find(key);
            int value = 0;
            if(entry != nullptr && UseTable(*entry, remaining, alpha, beta, value)) {
                hits++;
                return value;
            }
        }

        bool prune = algorithm == Algorithm::ALPHABETA;
        bool maximizing = player == CellState::Nought;
        int best = maximizing ? -INF : INF;
        ArraySequence<Move> moves = board.GetMoves(static_cast<int>(radius));  
        for(const Move& move : moves) {
            board.Place(move.row, move.col, player);
            int value = Run(board, remaining - 1, alpha, beta, Other(player));
            board.Erase(move.row, move.col);
            if(maximizing && value > best) {
                best = value;
            } else if(!maximizing && value < best) {
                best = value;
            }
            if(prune && maximizing && best > alpha) {
                alpha = best;
            } else if(prune && !maximizing && best < beta) {
                beta = best;
            }
            if(prune && alpha >= beta) {
                break;
            }
        }

        if(table) {
            TableEntry entry;
            entry.value = best;
            entry.depth = static_cast<size_t>(remaining);  
            entry.bound = BoundType::Exact;
            if(best <= origAlpha) {
                entry.bound = BoundType::Upper;
            } else if(best >= origBeta) {
                entry.bound = BoundType::Lower;
            }
            table->Add(key, entry);
        }
        return best;
    }

   public:
    Searcher(Algorithm algorithm, TableType tableType, size_t depth, size_t radius)  
        : algorithm(algorithm), depth(depth), radius(radius) {
        if(depth < 1) {
            throw std::invalid_argument("Depth must be positive");
        }
        if(radius < 1) {
            throw std::invalid_argument("Radius must be positive");
        }
        if(tableType == TableType::HASH) {
            table.reset(new HashDictionary<unsigned long long, TableEntry>());
        } else if(tableType == TableType::TREE) {
            table.reset(new TreeDictionary<unsigned long long, TableEntry>());
        }
    }

    SearchResult Recommend(Board board, CellState player = CellState::Nought) {
        SearchResult result;
        nodes = 0;
        hits = 0;
        if(table) {
            table->Clear();
        }
        auto start = std::chrono::steady_clock::now();

        ArraySequence<Move> moves;
        if(board.Winner() == CellState::Empty && !board.IsFull()) {
            moves = board.GetMoves(static_cast<int>(radius)); 
        }
        bool prune = algorithm == Algorithm::ALPHABETA;
        bool maximizing = player == CellState::Nought;
        int alpha = -INF;
        int beta = INF;
        int best = maximizing ? -INF : INF;
        for(const Move& move : moves) {
            board.Place(move.row, move.col, player);
            int value = Run(board, static_cast<int>(depth) - 1, alpha, beta, Other(player));
            board.Erase(move.row, move.col);
            bool better = maximizing ? value > best : value < best;
            if(better || !result.found) {
                best = value;
                result.move = move;
                result.found = true;
            }
            if(prune && maximizing && best > alpha) {
                alpha = best;
            } else if(prune && !maximizing && best < beta) {
                beta = best;
            }
        }

        auto end = std::chrono::steady_clock::now();
        result.score = result.found ? best : Evaluate(board);
        result.nodes = nodes;
        result.hits = hits;
        result.ms = std::chrono::duration<double, std::milli>(end - start).count();
        return result;
    }

    size_t GetTableSize() const { return table ? table->GetCount() : 0; }
};