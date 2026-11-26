#pragma once

#include "board.hpp"

const int WIN = 100000000;

inline int PowerOfTen(size_t degree) {
    int result = 1;
    for(size_t i = 0; i < degree; i++) { result *= 10; }
    return result;
}

inline int Evaluate(const Board& board) {
    const int dRow[4] = {0, 1, 1, 1};
    const int dCol[4] = {1, 0, 1, -1};
    size_t size = board.GetSize();
    size_t length = board.GetLineLength();
    int total = 0;
    for(size_t row = 0; row < size; row++) {
        for(size_t col = 0; col < size; col++) {
            for(int d = 0; d < 4; d++) {
                int endRow = static_cast<int>(row) + static_cast<int>(length - 1) * dRow[d];
                int endCol = static_cast<int>(col) + static_cast<int>(length - 1) * dCol[d];
                if(endRow < 0 || endRow >= static_cast<int>(size) || endCol < 0 || endCol >= static_cast<int>(size)) {
                    continue;
                }
                size_t crosses = 0;
                size_t noughts = 0;
                for(size_t i = 0; i < length; i++) {
                    CellState cell = board.Get(static_cast<int>(row) + static_cast<int>(i) * dRow[d], 
                                              static_cast<int>(col) + static_cast<int>(i) * dCol[d]);
                    if(cell == CellState::Cross) {
                        crosses++;
                    } else if(cell == CellState::Nought) {
                        noughts++;
                    }
                }
                if(crosses > 0 && noughts > 0) {
                    continue;
                }
                if(noughts > 0) {
                    total += PowerOfTen(noughts);
                } else if(crosses > 0) {
                    total -= PowerOfTen(crosses);
                }
            }
        }
    }
    return total;
}