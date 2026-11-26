#pragma once

#include "board.hpp"

const int WIN = 100000000;

inline int PowerOfTen(int degree) {
    int result = 1;
    for(int i = 0; i < degree; i++) { result *= 10; }
    return result;
}

inline int Evaluate(const Board& board) {
    const int dRow[4] = {0, 1, 1, 1};
    const int dCol[4] = {1, 0, 1, -1};
    int size = board.GetSize();
    int length = board.GetLineLength();
    int total = 0;
    for(int row = 0; row < size; row++) {
        for(int col = 0; col < size; col++) {
            for(int d = 0; d < 4; d++) {
                int endRow = row + (length - 1) * dRow[d];
                int endCol = col + (length - 1) * dCol[d];
                if(endRow < 0 || endRow >= size || endCol < 0 || endCol >= size) {
                    continue;
                }
                int crosses = 0;
                int noughts = 0;
                for(int i = 0; i < length; i++) {
                    int cell = board.Get(row + i * dRow[d], col + i * dCol[d]);
                    if(cell == CROSS) {
                        crosses++;
                    } else if(cell == NOUGHT) {
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
