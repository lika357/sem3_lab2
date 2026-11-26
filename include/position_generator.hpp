#pragma once

#include <random>

#include "board.hpp"

inline Board RandomBoard(int size, int lineLength, int moves, unsigned int seed) {
    Board board(size, lineLength);
    std::mt19937 generator(seed);
    CellState mark = CellState::Cross;
    int placed = 0;
    int attempts = 0;
    while(placed < moves && attempts < 10000) {
        attempts++;
        int row = generator() % size;
        int col = generator() % size;
        if(board.Get(row, col) != CellState::Empty) {
            continue;
        }
        board.Place(row, col, mark);
        if(board.Winner() != CellState::Empty) {
            board.Erase(row, col);
            continue;
        }
        placed++;
        mark = mark == CellState::Cross ? CellState::Nought : CellState::Cross;
    }
    return board;
}
