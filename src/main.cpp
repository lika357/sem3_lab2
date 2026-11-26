#include <cstdio>
#include <exception>

#include "ttt_window.hpp"

sf::Font Widget::defaultFont;

int main() {
    
    Board board(3,3);
    board.Place(1,1,1);
    try {
        TttWindow window(static_cast<size_t>(Window::WIDTH), static_cast<size_t>(Window::HEIGHT),
                         "TIC TAC TOE");
        window.run();
    } catch(const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
    return 0;
}
