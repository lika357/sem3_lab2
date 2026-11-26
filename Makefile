CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Iinclude
BUILD = build
SFML = -lsfml-graphics -lsfml-window -lsfml-system

HEADERS = $(wildcard include/*.hpp)
GUI_HEADERS = $(wildcard src/*.hpp)

.PHONY: test valgrind app run format clean

test: $(BUILD)/tests
	./$(BUILD)/tests

$(BUILD)/tests: tests/tests.cpp tests/assert.cpp tests/assert.hpp $(HEADERS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) tests/tests.cpp tests/assert.cpp -o $@

valgrind: $(BUILD)/tests
	valgrind --leak-check=full --error-exitcode=1 ./$(BUILD)/tests

app: $(BUILD)/app

$(BUILD)/app: src/main.cpp $(HEADERS) $(GUI_HEADERS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -O2 src/main.cpp -o $@ $(SFML)

run: $(BUILD)/app
	./$(BUILD)/app

format:
	clang-format -i include/*.hpp src/*.cpp src/*.hpp tests/*.cpp tests/*.hpp

clean:
	rm -rf $(BUILD)