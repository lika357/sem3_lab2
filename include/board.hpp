#pragma once

#include "array_sequence.hpp"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

struct Move {
    int row;
    int col;
};

enum class CellState {
    Empty = 0,
    Cross = 1,
    Nought = 2
};

class Board {
   private:
    size_t size;
    size_t lineLength;
    ArraySequence<CellState> cells;

    bool IsInside(int row, int col) const {
        return row >= 0 && row < static_cast<int>(size) &&
               col >= 0 && col < static_cast<int>(size);
    }

    void CheckCell(int row, int col) const {
        if(!IsInside(row, col)) {
            throw std::out_of_range("Cell is out of the board");
        }
    }

    bool HasLine(int row, int col, int dRow, int dCol) const {
        CellState mark = cells[row * size + col];
        if(mark == CellState::Empty) {
            return false;
        }
        for(size_t i = 1; i < lineLength; i++) {
            int r = row + static_cast<int>(i) * dRow;
            int c = col + static_cast<int>(i) * dCol;
            if(!IsInside(r, c) || cells[r * size + c] != mark) {
                return false;
            }
        }
        return true;
    }

    bool HasNeighbour(int row, int col, int radius) const {
        for(int r = row - radius; r <= row + radius; r++) {
            for(int c = col - radius; c <= col + radius; c++) {
                if(IsInside(r, c) && cells[r * size + c] != CellState::Empty) {
                    return true;
                }
            }
        }
        return false;
    }

   public:
    Board(size_t size = 3, size_t lineLength = 3)
        : size(size), lineLength(lineLength) {
        if(size < 1 || size * size > 36) {
            throw std::invalid_argument("Board size must be from 1 to 6");
        }
        if(lineLength < 1 || lineLength > size) {
            throw std::invalid_argument("Line length must be from 1 to board size");
        }
        cells.assign(size * size, CellState::Empty);
    }

    size_t GetSize() const { return size; }
    size_t GetLineLength() const { return lineLength; }

    CellState Get(int row, int col) const {
        CheckCell(row, col);
        return cells[row * size + col];
    }

    void Place(int row, int col, CellState mark) {
        CheckCell(row, col);
        if(mark != CellState::Cross && mark != CellState::Nought) {
            throw std::invalid_argument("Mark must be Cross or Nought");
        }
        if(cells[row * size + col] != CellState::Empty) {
            throw std::invalid_argument("Cell is already taken");
        }
        cells[row * size + col] = mark;
    }

    void Erase(int row, int col) {
        CheckCell(row, col);
        cells[row * size + col] = CellState::Empty;
    }

    void Clear() { cells.assign(size * size, CellState::Empty); }

    size_t CountPieces() const {
        size_t count = 0;
        for(CellState cell : cells) {
            if(cell != CellState::Empty) {
                count++;
            }
        }
        return count;
    }

    bool IsFull() const { return CountPieces() == size * size; }

    CellState Winner() const {
        const int dRow[4] = {0, 1, 1, 1};
        const int dCol[4] = {1, 0, 1, -1};
        for(int row = 0; row < static_cast<int>(size); row++) {
            for(int col = 0; col < static_cast<int>(size); col++) {
                for(int d = 0; d < 4; d++) {
                    if(HasLine(row, col, dRow[d], dCol[d])) {
                        return cells[row * size + col];
                    }
                }
            }
        }
        return CellState::Empty;
    }

    unsigned long long GetKey(CellState sideToMove) const {
        unsigned long long key = 0;
        unsigned long long power = 1;
        for(CellState cell : cells) {
            key += power * static_cast<unsigned long long>(cell);
            power *= 3;
        }
        return key * 2 + (sideToMove == CellState::Nought ? 1 : 0);
    }

    ArraySequence<Move> GetMoves(int radius) const {
        if(radius < 1) {
            throw std::invalid_argument("Radius must be positive");
        }
        ArraySequence<Move> moves;
        ArraySequence<Move> all;
        bool empty = CountPieces() == 0;
        for(int row = 0; row < static_cast<int>(size); row++) {
            for(int col = 0; col < static_cast<int>(size); col++) {
                if(cells[row * size + col] != CellState::Empty) {
                    continue;
                }
                all.Append({row, col});
                if(!empty && HasNeighbour(row, col, radius)) {
                    moves.Append({row, col});
                }
            }
        }
        if(empty) {
            ArraySequence<Move> result;
            result.Append({static_cast<int>(size / 2), static_cast<int>(size / 2)});
            return result;
        }
        if(moves.empty()) {
            moves = all;
        }
        int center = static_cast<int>(size / 2);
        std::sort(moves.begin(), moves.end(), [center](const Move& a, const Move& b) {
            int distA = std::max(std::abs(a.row - center), std::abs(a.col - center));
            int distB = std::max(std::abs(b.row - center), std::abs(b.col - center));
            if(distA != distB) {
                return distA < distB;
            }
            if(a.row != b.row) {
                return a.row < b.row;
            }
            return a.col < b.col;
        });
        return moves;
    }
};