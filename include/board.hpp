#pragma once

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <vector>

const int EMPTY = 0;
const int CROSS = 1;
const int NOUGHT = 2;

struct Move {
    int row;
    int col;
};

enum class CellType : int {
    Empty = 0
};

class Board {
   private:
    int size;
    int lineLength;
    std::vector<int> cells; 

    bool IsInside(int row, int col) const { 
        return row >= 0 && row < size && col >= 0 && col < size;
     }

    void CheckCell(int row, int col) const {
        if(!IsInside(row, col)) {
            throw std::out_of_range("Cell is out of the board");
        }
    }

    bool HasLine(int row, int col, int dRow, int dCol) const {
        int mark = cells[row * size + col];
        if(mark == EMPTY) {
            return false;
        }
        for(int i = 1; i < lineLength; i++) {
            int r = row + i * dRow;
            int c = col + i * dCol;
            if(!IsInside(r, c) || cells[r * size + c] != mark) {
                return false;
            }
        }
        return true;
    }

    bool HasNeighbour(int row, int col, int radius) const {
        for(int r = row - radius; r <= row + radius; r++) {
            for(int c = col - radius; c <= col + radius; c++) {
                if(IsInside(r, c) && cells[r * size + c] != EMPTY) {
                    return true;
                }
            }
        }
        return false;
    }

   public:
    Board(int size = 3, int lineLength = 3) : size(size), lineLength(lineLength) {
        if(size < 1 || size * size > 36) {
            throw std::invalid_argument("Board size must be from 1 to 6");
        }
        if(lineLength < 1 || lineLength > size) {
            throw std::invalid_argument("Line length must be from 1 to board size");
        }
        cells.assign(size * size, EMPTY);
    }

    int GetSize() const { return size; }
    int GetLineLength() const { return lineLength; }

    int Get(int row, int col) const {
        CheckCell(row, col);
        return cells[row * size + col];
    }

    void Place(int row, int col, int mark) {
        CheckCell(row, col);
        if(mark != CROSS && mark != NOUGHT) {
            throw std::invalid_argument("Mark must be CROSS or NOUGHT");
        }
        if(cells[row * size + col] != EMPTY) {
            throw std::invalid_argument("Cell is already taken");
        }
        cells[row * size + col] = mark;
    }

    void Erase(int row, int col) {
        CheckCell(row, col);
        cells[row * size + col] = EMPTY;
    }

    void Clear() { cells.assign(size * size, EMPTY); }

    int CountPieces() const {
        int count = 0;
        for(int cell : cells) {
            if(cell != EMPTY) {
                count++;
            }
        }
        return count;
    }

    bool IsFull() const { return CountPieces() == size * size; }

    int Winner() const {
        const int dRow[4] = {0, 1, 1, 1};
        const int dCol[4] = {1, 0, 1, -1};
        for(int row = 0; row < size; row++) {
            for(int col = 0; col < size; col++) {
                for(int d = 0; d < 4; d++) {
                    if(HasLine(row, col, dRow[d], dCol[d])) {
                        return cells[row * size + col];
                    }
                }
            }
        }
        return EMPTY;
    }

    unsigned long long GetKey(int sideToMove) const {
        unsigned long long key = 0;
        unsigned long long power = 1;
        for(int cell : cells) {
            key += power * static_cast<unsigned long long>(cell);
            power *= 3;
        }
        return key * 2 + (sideToMove == NOUGHT ? 1 : 0);
    }

    std::vector<Move> GetMoves(int radius) const {
        if(radius < 1) {
            throw std::invalid_argument("Radius must be positive");
        }
        std::vector<Move> moves;
        std::vector<Move> all;
        bool empty = CountPieces() == 0;
        for(int row = 0; row < size; row++) {
            for(int col = 0; col < size; col++) {
                if(cells[row * size + col] != EMPTY) {
                    continue;
                }
                all.push_back({row, col});
                if(!empty && HasNeighbour(row, col, radius)) {
                    moves.push_back({row, col});
                }
            }
        }
        if(empty) {
            return {{size / 2, size / 2}};
        }
        if(moves.empty()) {
            moves = all;
        }
        int center = size / 2;
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
