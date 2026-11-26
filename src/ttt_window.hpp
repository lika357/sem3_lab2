#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../include/benchmark.hpp"
#include "../include/board.hpp"
#include "../include/position_generator.hpp"
#include "../include/search.hpp"
#include "button.hpp"
#include "config.hpp"
#include "horizontal_layout.hpp"
#include "label.hpp"
#include "vertical_layout.hpp"

enum class Screen { MENU, PLAY, ANALYSIS, BENCH };

const int RADIUS_ALL = 99;
const int DICT_BENCH_COUNT = 2000;
const float BOARD_X = 50.0f;
const float BOARD_Y = 200.0f;
const float BOARD_AREA = 440.0f;
const float WINDOW_SCALE = 0.75f;

class TttWindow {
   private:
    sf::RenderWindow window;
    sf::Font font;
    Label titleLabel{"TIC TAC TOE"};

    VerticalLayout menuLayout{LayoutPositions::CHOOSE_X, LayoutPositions::CHOOSE_Y,
                              LayoutPositions::CHOOSE_GAP};
    Button btnPlay{"PLAY VS COMPUTER", ButtonSizes::CHOOSE_W, ButtonSizes::CHOOSE_H};
    Button btnAnalysis{"ANALYSIS", ButtonSizes::CHOOSE_W, ButtonSizes::CHOOSE_H};
    Button btnBench{"BENCHMARK", ButtonSizes::CHOOSE_W, ButtonSizes::CHOOSE_H};

    HorizontalLayout settingsLayout{LayoutPositions::WORK_X, LayoutPositions::FUNC_Y,
                                    LayoutPositions::FUNC_GAP};
    Button btnSize{"", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnLine{"", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnDepth{"", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnRadius{"", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnAlgorithm{"", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnTable{"", ButtonSizes::STACK_W, ButtonSizes::STACK_H};

    HorizontalLayout actionLayout{LayoutPositions::WORK_X, LayoutPositions::FUNC_Y + 60.0f,
                                  LayoutPositions::FUNC_GAP};
    Button btnNew{"NEW GAME", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnRecommend{"RECOMMEND", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnRandom{"RANDOM", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnClear{"CLEAR", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnRun{"RUN", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnSave{"SAVE CSV", ButtonSizes::STACK_W, ButtonSizes::STACK_H};
    Button btnBack{"BACK", ButtonSizes::STACK_W, ButtonSizes::STACK_H};

    Label statusLabel{""};
    Label infoLabel{""};
    Label hintLabel{""};

    std::vector<std::unique_ptr<Button>> cells;
    std::vector<std::unique_ptr<Label>> searchLabels;
    std::vector<std::unique_ptr<Label>> dictLabels;
    std::vector<BenchRow> searchRows;
    std::vector<DictBenchRow> dictRows;

    Screen screen = Screen::MENU;
    Screen pendingScreen = Screen::MENU;
    bool hasPendingScreen = false;

    int size = 3;
    int line = 3;
    int depth = 9;
    int radius = 1;
    Algorithm algorithm = Algorithm::ALPHABETA;
    TableType tableType = TableType::HASH;

    Board board{3, 3};
    bool gameOver = false;
    bool hasRecommendation = false;
    Move recommended = {0, 0};

    static int DefaultDepth(int boardSize) {
        if(boardSize == 3) {
            return 9;
        }
        if(boardSize == 4) {
            return 5;
        }
        if(boardSize == 5) {
            return 4;
        }
        return 3;
    }

    static std::string FormatResult(const SearchResult& result, size_t tableSize) {
        std::stringstream ss;
        ss << "score=" << result.score << "  nodes=" << result.nodes << "  hits=" << result.hits
           << "  table=" << tableSize << "  time=" << std::fixed << std::setprecision(2) << result.ms
           << " ms";
        return ss.str();
    }

    void UpdateSettingsTexts() {
        btnSize.setText("SIZE N: " + std::to_string(size));
        btnLine.setText("LINE K: " + std::to_string(line));
        btnDepth.setText("DEPTH: " + std::to_string(depth));
        btnRadius.setText("RADIUS: " + (radius == RADIUS_ALL ? std::string("ALL") : std::to_string(radius)));
        btnAlgorithm.setText(AlgorithmName(algorithm));
        btnTable.setText("TABLE: " + TableName(tableType));
    }

    int GetRadius() const { return radius == RADIUS_ALL ? size : radius; }

    void ApplySizeChange() {
        UpdateSettingsTexts();
        if(screen == Screen::BENCH) {
            ClearBench();
            statusLabel.setText("Settings changed, press RUN");
        } else {
            ResetBoard();
        }
    }

    void OnSizeClick() {
        size = size >= 6 ? 3 : size + 1;
        line = size;
        depth = DefaultDepth(size);
        ApplySizeChange();
    }

    void OnLineClick() {
        line = line >= size ? 3 : line + 1;
        if(line > size) {
            line = size;
        }
        ApplySizeChange();
    }

    void OnDepthClick() {
        depth = depth >= 9 ? 1 : depth + 1;
        UpdateSettingsTexts();
    }

    void OnRadiusClick() {
        if(radius == RADIUS_ALL) {
            radius = 1;
        } else if(radius >= 3) {
            radius = RADIUS_ALL;
        } else {
            radius++;
        }
        UpdateSettingsTexts();
    }

    void OnAlgorithmClick() {
        algorithm = algorithm == Algorithm::MINIMAX ? Algorithm::ALPHABETA : Algorithm::MINIMAX;
        UpdateSettingsTexts();
    }

    void OnTableClick() {
        if(tableType == TableType::NONE) {
            tableType = TableType::HASH;
        } else if(tableType == TableType::HASH) {
            tableType = TableType::TREE;
        } else {
            tableType = TableType::NONE;
        }
        UpdateSettingsTexts();
    }

    void BuildBoard() {
        cells.clear();
        float cellSize = std::min(80.0f, BOARD_AREA / size);
        for(int row = 0; row < size; row++) {
            for(int col = 0; col < size; col++) {
                auto cell = std::make_unique<Button>("", cellSize - 4.0f, cellSize - 4.0f);
                cell->setPosition(BOARD_X + col * cellSize, BOARD_Y + row * cellSize);
                cell->setOnClick(std::bind(&TttWindow::OnCellClick, this, row, col));
                cells.push_back(std::move(cell));
            }
        }
        RefreshBoard();
    }

    void RefreshBoard() {
        for(int row = 0; row < size; row++) {
            for(int col = 0; col < size; col++) {
                Button& cell = *cells[row * size + col];
                int value = board.Get(row, col);
                if(value == CROSS) {
                    cell.setText("X");
                    cell.setColor(Colors::ELEMENT);
                } else if(value == NOUGHT) {
                    cell.setText("O");
                    cell.setColor(Colors::CELL_O);
                } else {
                    cell.setText("");
                    cell.setColor(Colors::BUTTON_FILL);
                }
                if(hasRecommendation && recommended.row == row && recommended.col == col) {
                    cell.setText("?");
                    cell.setColor(Colors::CELL_BEST);
                }
            }
        }
    }

    void ResetBoard() {
        board = Board(size, line);
        gameOver = false;
        hasRecommendation = false;
        infoLabel.setText("");
        if(screen == Screen::PLAY) {
            statusLabel.setText("Your move (X)");
        } else {
            statusLabel.setText("Click a cell: empty -> X -> O -> empty");
        }
        BuildBoard();
    }

    bool CheckEnd() {
        int winner = board.Winner();
        if(winner == CROSS) {
            statusLabel.setText(screen == Screen::PLAY ? "You win!" : "X wins");
        } else if(winner == NOUGHT) {
            statusLabel.setText(screen == Screen::PLAY ? "Computer wins" : "O wins");
        } else if(board.IsFull()) {
            statusLabel.setText("Draw");
        } else {
            return false;
        }
        gameOver = true;
        return true;
    }

    void OnCellClick(int row, int col) {
        if(screen == Screen::PLAY) {
            OnPlayClick(row, col);
        } else if(screen == Screen::ANALYSIS) {
            OnAnalysisClick(row, col);
        }
    }

    void OnPlayClick(int row, int col) {
        if(gameOver || board.Get(row, col) != EMPTY) {
            return;
        }
        board.Place(row, col, CROSS);
        if(!CheckEnd()) {
            Searcher searcher(algorithm, tableType, depth, GetRadius());
            SearchResult result = searcher.Recommend(board, NOUGHT);
            if(result.found) {
                board.Place(result.move.row, result.move.col, NOUGHT);
            }
            infoLabel.setText(FormatResult(result, searcher.GetTableSize()));
            if(!CheckEnd()) {
                statusLabel.setText("Your move (X)");
            }
        }
        RefreshBoard();
    }

    void OnAnalysisClick(int row, int col) {
        int value = board.Get(row, col);
        if(value == EMPTY) {
            board.Place(row, col, CROSS);
        } else if(value == CROSS) {
            board.Erase(row, col);
            board.Place(row, col, NOUGHT);
        } else {
            board.Erase(row, col);
        }
        hasRecommendation = false;
        gameOver = false;
        infoLabel.setText("");
        if(!CheckEnd()) {
            statusLabel.setText("Click a cell: empty -> X -> O -> empty");
        }
        RefreshBoard();
    }

    void OnRecommendClick() {
        if(board.Winner() != EMPTY || board.IsFull()) {
            statusLabel.setText("The game is over, nothing to recommend");
            return;
        }
        Searcher searcher(algorithm, tableType, depth, GetRadius());
        SearchResult result = searcher.Recommend(board, NOUGHT);
        hasRecommendation = result.found;
        recommended = result.move;
        statusLabel.setText("O should play: row " + std::to_string(result.move.row + 1) + ", col " +
                            std::to_string(result.move.col + 1));
        infoLabel.setText(FormatResult(result, searcher.GetTableSize()));
        RefreshBoard();
    }

    void OnRandomClick() {
        int moves = std::max(2, size * size / 3);
        board = RandomBoard(size, line, moves, std::random_device{}());
        hasRecommendation = false;
        gameOver = false;
        infoLabel.setText("");
        statusLabel.setText("Random position, press RECOMMEND");
        RefreshBoard();
    }

    void OnClearClick() { ResetBoard(); }

    void OnRunClick() {
        searchRows = RunSearchBenchmark(Board(size, line), depth, GetRadius());
        dictRows = RunDictionaryBenchmark(DICT_BENCH_COUNT);
        BuildBenchLabels();
        statusLabel.setText("Done. Bars: log scale of time. Press SAVE CSV to export");
    }

    void OnSaveClick() {
        if(searchRows.empty()) {
            statusLabel.setText("Nothing to save, press RUN first");
            return;
        }
        WriteSearchCsv(searchRows, "bench_search.csv");
        WriteDictionaryCsv(dictRows, "bench_dictionary.csv");
        statusLabel.setText("Saved to bench_search.csv and bench_dictionary.csv");
    }

    void BuildBenchLabels() {
        searchLabels.clear();
        dictLabels.clear();
        for(size_t i = 0; i < searchRows.size(); i++) {
            const BenchRow& row = searchRows[i];
            std::stringstream ss;
            ss << row.name << " nodes=" << row.nodes << " hits=" << row.hits << " " << std::fixed
               << std::setprecision(2) << row.ms << "ms";
            auto label = std::make_unique<Label>(ss.str());
            label->setPosition(50.0f, 200.0f + i * 32.0f);
            searchLabels.push_back(std::move(label));
        }
        for(size_t i = 0; i < dictRows.size(); i++) {
            const DictBenchRow& row = dictRows[i];
            std::stringstream ss;
            ss << row.name << " " << row.order << " n=" << row.count << " " << std::fixed
               << std::setprecision(2) << row.ms << "ms";
            auto label = std::make_unique<Label>(ss.str());
            label->setPosition(50.0f + (i % 2) * 400.0f, 440.0f + (i / 2) * 32.0f);
            dictLabels.push_back(std::move(label));
        }
    }

    void ClearBench() {
        searchRows.clear();
        dictRows.clear();
        searchLabels.clear();
        dictLabels.clear();
    }

    void RequestScreen(Screen next) {
        pendingScreen = next;
        hasPendingScreen = true;
    }

    void ShowScreen(Screen next) {
        screen = next;
        while(actionLayout.getWidgetCount() > 0) { actionLayout.removeLast(); }
        menuLayout.setVisible(next == Screen::MENU);
        settingsLayout.setVisible(next != Screen::MENU);
        actionLayout.setVisible(next != Screen::MENU);
        cells.clear();
        ClearBench();
        hintLabel.setText("");

        if(next == Screen::MENU) {
            titleLabel.setText("TIC TAC TOE");
            statusLabel.setText("");
            infoLabel.setText("");
        } else if(next == Screen::PLAY) {
            titleLabel.setText("PLAY VS COMPUTER");
            actionLayout.append(btnNew);
            actionLayout.append(btnBack);
            ResetBoard();
        } else if(next == Screen::ANALYSIS) {
            titleLabel.setText("ANALYSIS");
            actionLayout.append(btnRecommend);
            actionLayout.append(btnRandom);
            actionLayout.append(btnClear);
            actionLayout.append(btnBack);
            ResetBoard();
        } else {
            titleLabel.setText("BENCHMARK");
            actionLayout.append(btnRun);
            actionLayout.append(btnSave);
            actionLayout.append(btnBack);
            statusLabel.setText("Press RUN: all 6 modes solve the empty board with current settings");
            infoLabel.setText("");
            hintLabel.setText("Big N with big DEPTH can take a long time without alpha-beta");
        }
    }

    void UpdateView(sf::Vector2u pixels) {
        if(pixels.x == 0 || pixels.y == 0) {
            return;
        }
        float windowRatio = static_cast<float>(pixels.x) / pixels.y;
        float viewRatio = Window::WIDTH / Window::HEIGHT;
        float sizeX = 1.0f;
        float sizeY = 1.0f;
        if(windowRatio > viewRatio) {
            sizeX = viewRatio / windowRatio;
        } else {
            sizeY = windowRatio / viewRatio;
        }
        sf::View view(sf::FloatRect({0.0f, 0.0f}, {Window::WIDTH, Window::HEIGHT}));
        view.setViewport(sf::FloatRect({(1.0f - sizeX) / 2, (1.0f - sizeY) / 2}, {sizeX, sizeY}));
        window.setView(view);
    }

    sf::Event ToLogical(const sf::Event& event) const {
        if(const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
            sf::Vector2f point = window.mapPixelToCoords(mouse->position);
            sf::Event::MouseButtonPressed mapped{mouse->button,
                                                 {static_cast<int>(point.x), static_cast<int>(point.y)}};
            return mapped;
        }
        return event;
    }

    void HandleEvent(const sf::Event& rawEvent) {
        sf::Event event = ToLogical(rawEvent);
        if(screen == Screen::MENU) {
            menuLayout.handleEvent(event);
            return;
        }
        if(settingsLayout.handleEvent(event) || actionLayout.handleEvent(event)) {
            return;
        }
        for(size_t i = 0; i < cells.size(); i++) {
            if(cells[i]->handleEvent(event)) {
                return;
            }
        }
    }

    void DrawBars() {
        float maxWidth = 500.0f;
        for(size_t i = 0; i < searchRows.size(); i++) {
            double microseconds = searchRows[i].ms * 1000.0;
            float width = static_cast<float>(40.0 * std::log10(1.0 + microseconds));
            if(width > maxWidth) {
                width = maxWidth;
            }
            sf::RectangleShape bar;
            bar.setSize({std::max(width, 2.0f), 20.0f});
            bar.setPosition({700.0f, 206.0f + i * 32.0f});
            bar.setFillColor(Colors::BAR);
            window.draw(bar);
        }
    }

    void Draw() {
        window.clear(Colors::BACKGROUND);
        titleLabel.draw(window);
        menuLayout.draw(window);
        settingsLayout.draw(window);
        actionLayout.draw(window);
        for(auto& cell : cells) { cell->draw(window); }
        if(screen == Screen::BENCH) {
            for(auto& label : searchLabels) { label->draw(window); }
            for(auto& label : dictLabels) { label->draw(window); }
            DrawBars();
        }
        statusLabel.draw(window);
        infoLabel.draw(window);
        hintLabel.draw(window);
        window.display();
    }

    void LoadFont() {
        if(font.openFromFile("arial.ttf")) {
            return;
        }
        if(font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
            return;
        }
        throw std::runtime_error("Failed NOT open font: put arial.ttf next to the program");
    }

   public:
    TttWindow(size_t width, size_t height, const std::string& title) {
        window.create(sf::VideoMode({static_cast<unsigned int>(width * WINDOW_SCALE),
                                     static_cast<unsigned int>(height * WINDOW_SCALE)}),
                      title);
        UpdateView(window.getSize());
        LoadFont();
        Widget::getDefaultFont() = font;

        titleLabel.setPosition(ElementPositions::TITLE_X, ElementPositions::TITLE_Y);
        statusLabel.setPosition(ElementPositions::RESULT_X, ElementPositions::RESULT_Y);
        infoLabel.setPosition(ElementPositions::RESULT_X, ElementPositions::RESULT_Y + 35.0f);
        hintLabel.setPosition(ElementPositions::RESULT_X, ElementPositions::RESULT_Y - 35.0f);

        menuLayout.append(btnPlay);
        menuLayout.append(btnAnalysis);
        menuLayout.append(btnBench);
        btnPlay.setOnClick(std::bind(&TttWindow::RequestScreen, this, Screen::PLAY));
        btnAnalysis.setOnClick(std::bind(&TttWindow::RequestScreen, this, Screen::ANALYSIS));
        btnBench.setOnClick(std::bind(&TttWindow::RequestScreen, this, Screen::BENCH));

        settingsLayout.append(btnSize);
        settingsLayout.append(btnLine);
        settingsLayout.append(btnDepth);
        settingsLayout.append(btnRadius);
        settingsLayout.append(btnAlgorithm);
        settingsLayout.append(btnTable);
        btnSize.setOnClick(std::bind(&TttWindow::OnSizeClick, this));
        btnLine.setOnClick(std::bind(&TttWindow::OnLineClick, this));
        btnDepth.setOnClick(std::bind(&TttWindow::OnDepthClick, this));
        btnRadius.setOnClick(std::bind(&TttWindow::OnRadiusClick, this));
        btnAlgorithm.setOnClick(std::bind(&TttWindow::OnAlgorithmClick, this));
        btnTable.setOnClick(std::bind(&TttWindow::OnTableClick, this));
        UpdateSettingsTexts();

        btnNew.setOnClick(std::bind(&TttWindow::OnClearClick, this));
        btnRecommend.setOnClick(std::bind(&TttWindow::OnRecommendClick, this));
        btnRandom.setOnClick(std::bind(&TttWindow::OnRandomClick, this));
        btnClear.setOnClick(std::bind(&TttWindow::OnClearClick, this));
        btnRun.setOnClick(std::bind(&TttWindow::OnRunClick, this));
        btnSave.setOnClick(std::bind(&TttWindow::OnSaveClick, this));
        btnBack.setOnClick(std::bind(&TttWindow::RequestScreen, this, Screen::MENU));

        ShowScreen(Screen::MENU);
    }

    void run() {
        while(window.isOpen()) {
            while(auto event = window.pollEvent()) {
                if(event->is<sf::Event::Closed>()) {
                    window.close();
                }
                if(const auto* resized = event->getIf<sf::Event::Resized>()) {
                    UpdateView(resized->size);
                }
                HandleEvent(*event);
            }
            if(hasPendingScreen) {
                hasPendingScreen = false;
                ShowScreen(pendingScreen);
            }
            Draw();
        }
    }
};