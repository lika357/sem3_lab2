#include <cstdio>
#include <string>

#include "../include/HashDictionary.hpp"
#include "../include/TreeDictionary.hpp"
#include "../include/array_sequence.hpp"
#include "../include/benchmark.hpp"
#include "../include/board.hpp"
#include "../include/evaluator.hpp"
#include "../include/list_sequence.hpp"
#include "../include/position_generator.hpp"
#include "../include/search.hpp"
#include "../tests/assert.hpp"

template <typename Exception, typename Function>
bool Throws(Function function) {
    try {
        function();
    } catch(const Exception&) { return true; } catch(...) {
        return false;
    }
    return false;
}

void TestArraySequence() {
    ArraySequence<int> seq;
    assert_func(seq.GetLength() == 0);
    for(int i = 0; i < 100; i++) { seq.Append(i); }
    assert_func(seq.GetLength() == 100);
    assert_func(seq[0] == 0);
    assert_func(seq.Get(99) == 99);
    assert_func(seq.GetLast() == 99);
    assert_func(Throws<std::out_of_range>([&] { seq.Get(100); }));

    seq.RemoveAt(0);
    assert_func(seq.GetLength() == 99 && seq[0] == 1);
    seq.RemoveAt(98);
    assert_func(seq.GetLength() == 98 && seq.GetLast() == 98);
    assert_func(Throws<std::out_of_range>([&] { seq.RemoveAt(98); }));

    ArraySequence<int> copy = seq;
    copy.Append(500);
    assert_func(copy.GetLength() == 99 && seq.GetLength() == 98);

    int sum = 0;
    for(int value : copy) { sum += value; }
    assert_func(sum == 5351);

    seq.Clear();
    assert_func(seq.GetLength() == 0);
    ArraySequence<int> empty;
    assert_func(Throws<std::out_of_range>([&] { empty.GetLast(); }));
}

void TestListSequence() {
    ListSequence<int> list;
    assert_func(list.GetLength() == 0);
    assert_func(Throws<std::out_of_range>([&] { list.GetLast(); }));
    assert_func(Throws<std::out_of_range>([&] { list.PopFront(); }));
    for(int i = 1; i <= 5; i++) { list.Append(i); }
    assert_func(list.GetLength() == 5);
    assert_func(list[0] == 1 && list[4] == 5 && list.GetLast() == 5);
    assert_func(Throws<std::out_of_range>([&] { list[5]; }));

    int sum = 0;
    for(int& value : list) { sum += value; }
    assert_func(sum == 15);

    list.PopFront();
    assert_func(list.GetLength() == 4 && list[0] == 2);

    ListSequence<int> copy;
    copy = list;
    copy.Append(10);
    assert_func(copy.GetLength() == 5 && list.GetLength() == 4);
}

void TestTreeDictionary() {
    TreeDictionary<int, std::string> tree;
    assert_func(tree.GetCount() == 0);
    assert_func(!tree.ContainsKey(5));
    assert_func(tree.Find(5) == nullptr);

    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    for(int key : keys) { tree.Add(key, std::to_string(key)); }
    assert_func(tree.GetCount() == 7);
    assert_func(tree.Get(40) == "40");
    assert_func(tree.ContainsKey(80));
    assert_func(tree.GetCapacity() == tree.GetCount());

    tree.Add(40, "new");
    assert_func(tree.GetCount() == 7 && tree.Get(40) == "new");

    std::string* found = tree.Find(60);
    assert_func(found != nullptr && *found == "60");

    tree.Remove(20);
    assert_func(!tree.ContainsKey(20) && tree.GetCount() == 6);
    tree.Remove(30);
    assert_func(!tree.ContainsKey(30) && tree.ContainsKey(40) && tree.GetCount() == 5);
    tree.Remove(50);
    assert_func(!tree.ContainsKey(50) && tree.ContainsKey(60) && tree.ContainsKey(70) &&
                tree.GetCount() == 4);

    assert_func(Throws<std::out_of_range>([&] { tree.Remove(999); }));
    assert_func(Throws<std::out_of_range>([&] { tree.Get(999); }));

    tree.Clear();
    assert_func(tree.GetCount() == 0 && !tree.ContainsKey(70));
    tree.Add(1, "one");
    assert_func(tree.Get(1) == "one");

    IDictionary<int, std::string>* dict = new TreeDictionary<int, std::string>();
    dict->Add(1, "a");
    assert_func(dict->Get(1) == "a");
    delete dict;
}

void TestHashFunctions() {
    assert_func(GetHashCode(12345) == GetHashCode(12345));
    assert_func(GetHashCode(1) != GetHashCode(2));
    assert_func(GetHashCode(-7) >= 0);
    assert_func(GetHashCode(18446744073709551615ULL) >= 0);
    assert_func(GetHashCode(std::string("abc")) == GetHashCode(std::string("abc")));
    assert_func(GetHashCode(std::string("abc")) != GetHashCode(std::string("abd")));
    assert_func(GetHashCode(std::string("")) >= 0);
}

void TestHashDictionary() {
    HashDictionary<int, std::string> hash;
    assert_func(hash.GetCount() == 0 && hash.GetCapacity() == 8);
    assert_func(!hash.ContainsKey(1));
    assert_func(hash.Find(1) == nullptr);

    hash.Add(1, "one");
    hash.Add(2, "two");
    assert_func(hash.GetCount() == 2);
    assert_func(hash.Get(1) == "one" && hash.ContainsKey(2));

    hash.Add(1, "uno");
    assert_func(hash.GetCount() == 2 && hash.Get(1) == "uno");
    std::string* found = hash.Find(2);
    assert_func(found != nullptr && *found == "two");

    hash.Remove(1);
    assert_func(!hash.ContainsKey(1) && hash.GetCount() == 1);
    assert_func(Throws<std::out_of_range>([&] { hash.Remove(1); }));
    assert_func(Throws<std::out_of_range>([&] { hash.Get(1); }));

    hash.Clear();
    assert_func(hash.GetCount() == 0 && !hash.ContainsKey(2));
}

void TestHashCollisions() {
    HashDictionary<int, int> hash([](const int&) { return 7; });
    for(int i = 0; i < 20; i++) { hash.Add(i, i * 10); }
    bool allFound = true;
    for(int i = 0; i < 20; i++) {
        if(hash.Get(i) != i * 10) {
            allFound = false;
        }
    }
    assert_func(allFound);
    assert_func(hash.GetLongestChain() == 20);
    hash.Remove(5);
    assert_func(!hash.ContainsKey(5) && hash.GetCount() == 19 && hash.Get(6) == 60);
}

void TestHashResize() {
    HashDictionary<int, int> hash(nullptr, 4, 2);
    for(int i = 0; i < 4; i++) { hash.Add(i, i); }
    assert_func(hash.GetCapacity() == 4);
    hash.Add(4, 4);
    assert_func(hash.GetCapacity() == 8);

    hash.Remove(0);
    hash.Remove(1);
    assert_func(hash.GetCapacity() == 8);
    hash.Remove(2);
    assert_func(hash.GetCount() == 2 && hash.GetCapacity() == 4);
    assert_func(hash.Get(3) == 3 && hash.Get(4) == 4);

    HashDictionary<int, int> big;
    for(int i = 0; i < 1000; i++) {
        hash.Add(i, i);
        big.Add(i, i + 1);
    }
    bool allFound = true;
    for(int i = 0; i < 1000; i++) {
        if(big.Get(i) != i + 1) {
            allFound = false;
        }
    }
    assert_func(allFound && big.GetCount() == 1000 && big.GetCapacity() >= 1000);
    for(int i = 0; i < 1000; i++) { big.Remove(i); }
    assert_func(big.GetCount() == 0 && big.GetCapacity() < 100);
}

void TestHashWrongParameters() {
    assert_func(Throws<std::invalid_argument>([] { HashDictionary<int, int> d(nullptr, 0); }));
}

void TestHashStringKeys() {
    HashDictionary<std::string, int> hash;
    hash.Add("Ivanov", 1);
    hash.Add("Petrov", 2);
    hash.Add("", 3);
    assert_func(hash.Get("Ivanov") == 1 && hash.Get("Petrov") == 2 && hash.Get("") == 3);
    assert_func(!hash.ContainsKey("Sidorov"));
}

void TestBoardCreate() {
    Board board(3, 3);
    assert_func(board.GetSize() == 3 && board.GetLineLength() == 3);
    assert_func(board.CountPieces() == 0 && !board.IsFull());
    assert_func(Throws<std::invalid_argument>([] { Board b(0, 1); }));
    assert_func(Throws<std::invalid_argument>([] { Board b(7, 3); }));
    assert_func(Throws<std::invalid_argument>([] { Board b(3, 4); }));
    assert_func(Throws<std::invalid_argument>([] { Board b(3, 0); }));
    Board big(6, 4);
    assert_func(big.GetSize() == 6);
}

void TestBoardPlace() {
    Board board(3, 3);
    board.Place(1, 1, CellState::Cross);
    assert_func(board.Get(1, 1) == CellState::Cross && board.CountPieces() == 1);
    assert_func(Throws<std::invalid_argument>([&] { board.Place(1, 1, CellState::Nought); }));
    assert_func(Throws<std::out_of_range>([&] { board.Place(3, 0, CellState::Cross); }));
    assert_func(Throws<std::out_of_range>([&] { board.Place(0, -1, CellState::Cross); }));
    assert_func(Throws<std::invalid_argument>([&] { board.Place(0, 0, CellState::Empty); }));
    assert_func(Throws<std::out_of_range>([&] { board.Get(5, 5); }));
    board.Erase(1, 1);
    assert_func(board.Get(1, 1) == CellState::Empty);
    board.Place(0, 0, CellState::Nought);
    board.Clear();
    assert_func(board.CountPieces() == 0);
}

void TestBoardWinner() {
    Board row(3, 3);
    for(int c = 0; c < 3; c++) { row.Place(1, c, CellState::Cross); }
    assert_func(row.Winner() == CellState::Cross);

    Board col(3, 3);
    for(int r = 0; r < 3; r++) { col.Place(r, 2, CellState::Nought); }
    assert_func(col.Winner() == CellState::Nought);

    Board diag(3, 3);
    for(int i = 0; i < 3; i++) { diag.Place(i, i, CellState::Cross); }
    assert_func(diag.Winner() == CellState::Cross);

    Board anti(3, 3);
    for(int i = 0; i < 3; i++) { anti.Place(i, 2 - i, CellState::Nought); }
    assert_func(anti.Winner() == CellState::Nought);

    Board none(3, 3);
    none.Place(0, 0, CellState::Cross);
    none.Place(0, 1, CellState::Cross);
    none.Place(0, 2, CellState::Nought);
    assert_func(none.Winner() == CellState::Empty);

    Board shortLine(5, 3);
    shortLine.Place(2, 1, CellState::Cross);
    shortLine.Place(2, 2, CellState::Cross);
    shortLine.Place(2, 3, CellState::Cross);
    assert_func(shortLine.Winner() == CellState::Cross);

    Board two(5, 4);
    two.Place(2, 1, CellState::Cross);
    two.Place(2, 2, CellState::Cross);
    two.Place(2, 3, CellState::Cross);
    assert_func(two.Winner() == CellState::Empty);
}

void TestBoardDraw() {
    Board board(3, 3);
    CellState pattern[3][3] = {{CellState::Cross, CellState::Nought, CellState::Cross}, {CellState::Cross, CellState::Nought, CellState::Nought}, {CellState::Nought, CellState::Cross, CellState::Cross}};
    for(int r = 0; r < 3; r++) {
        for(int c = 0; c < 3; c++) { board.Place(r, c, pattern[r][c]); }
    }
    assert_func(board.IsFull() && board.Winner() == CellState::Empty);
}

void TestBoardKey() {
    Board a(3, 3);
    Board b(3, 3);
    assert_func(a.GetKey(CellState::Nought) == b.GetKey(CellState::Nought));
    a.Place(0, 0, CellState::Cross);
    assert_func(a.GetKey(CellState::Nought) != b.GetKey(CellState::Nought));
    b.Place(0, 0, CellState::Cross);
    assert_func(a.GetKey(CellState::Nought) == b.GetKey(CellState::Nought));
    assert_func(a.GetKey(CellState::Nought) != a.GetKey(CellState::Cross));
    b.Erase(0, 0);
    b.Place(0, 1, CellState::Cross);
    assert_func(a.GetKey(CellState::Nought) != b.GetKey(CellState::Nought));

    Board big(6, 4);
    for(int i = 0; i < 6; i++) { big.Place(i, i, CellState::Nought); }
    assert_func(big.GetKey(CellState::Nought) > 0);
}

void TestBoardMoves() {
    Board board(5, 4);
    ArraySequence<Move> moves = board.GetMoves(1);
    assert_func(moves.GetLength() == 1 && moves[0].row == 2 && moves[0].col == 2);

    board.Place(2, 2, CellState::Cross);
    moves = board.GetMoves(1);
    assert_func(moves.GetLength() == 8);
    moves = board.GetMoves(2);
    assert_func(moves.GetLength() == 24);

    Board corner(5, 4);
    corner.Place(0, 0, CellState::Cross);
    moves = corner.GetMoves(1);
    assert_func(moves.GetLength() == 3);
    assert_func(Throws<std::invalid_argument>([&] { corner.GetMoves(0); }));

    Board full(1, 1);
    full.Place(0, 0, CellState::Cross);
    assert_func(full.GetMoves(1).empty());
}

void TestEvaluate() {
    Board board(3, 3);
    assert_func(Evaluate(board) == 0);
    board.Place(1, 1, CellState::Nought);
    assert_func(Evaluate(board) > 0);
    Board crossBoard(3, 3);
    crossBoard.Place(1, 1, CellState::Cross);
    assert_func(Evaluate(crossBoard) == -Evaluate(board));

    Board mixed(3, 3);
    mixed.Place(0, 0, CellState::Nought);
    mixed.Place(0, 1, CellState::Cross);
    Board single(3, 3);
    single.Place(0, 0, CellState::Nought);
    assert_func(Evaluate(mixed) != Evaluate(single));

    Board two(3, 3);
    two.Place(0, 0, CellState::Nought);
    two.Place(0, 1, CellState::Nought);
    assert_func(Evaluate(two) > Evaluate(single));
}

void TestPositionGenerator() {
    Board a = RandomBoard(4, 4, 5, 123);
    Board b = RandomBoard(4, 4, 5, 123);
    assert_func(a.CountPieces() == 5 && a.Winner() == CellState::Empty);
    assert_func(a.GetKey(CellState::Nought) == b.GetKey(CellState::Nought));
    Board empty = RandomBoard(3, 3, 0, 1);
    assert_func(empty.CountPieces() == 0);
}

void TestSearchWinInOne() {
    Board board(3, 3);
    board.Place(0, 0, CellState::Nought);
    board.Place(0, 1, CellState::Nought);
    board.Place(1, 0, CellState::Cross);
    board.Place(1, 1, CellState::Cross);
    board.Place(2, 2, CellState::Cross);
    Searcher searcher(Algorithm::ALPHABETA, TableType::HASH, 3, 1);
    SearchResult result = searcher.Recommend(board, CellState::Nought);
    assert_func(result.found && result.move.row == 0 && result.move.col == 2);
    assert_func(result.score > WIN);
}

void TestSearchBlock() {
    Board board(3, 3);
    board.Place(0, 0, CellState::Cross);
    board.Place(0, 1, CellState::Cross);
    board.Place(1, 1, CellState::Nought);
    Searcher searcher(Algorithm::MINIMAX, TableType::NONE, 4, 1);
    SearchResult result = searcher.Recommend(board, CellState::Nought);
    assert_func(result.found && result.move.row == 0 && result.move.col == 2);
}

void TestSearchGameOver() {
    Board won(3, 3);
    for(int c = 0; c < 3; c++) { won.Place(0, c, CellState::Cross); }
    Searcher searcher(Algorithm::ALPHABETA, TableType::NONE, 3, 1);
    assert_func(!searcher.Recommend(won, CellState::Nought).found);

    Board full(1, 1);
    full.Place(0, 0, CellState::Cross);
    assert_func(!searcher.Recommend(full, CellState::Nought).found);
}

void TestSearchWrongParameters() {
    assert_func(Throws<std::invalid_argument>([] { Searcher s(Algorithm::MINIMAX, TableType::NONE, 0, 1); }));
    assert_func(Throws<std::invalid_argument>([] { Searcher s(Algorithm::MINIMAX, TableType::NONE, 3, 0); }));
}

void TestSearchEmptyBoardIsDraw() {
    Board board(3, 3);
    Searcher searcher(Algorithm::ALPHABETA, TableType::HASH, 9, 1);
    SearchResult result = searcher.Recommend(board, CellState::Nought);
    assert_func(result.found && result.score == 0);
    assert_func(result.move.row == 1 && result.move.col == 1);
    assert_func(searcher.GetTableSize() > 0);
}

bool AllModesAgree(const Board& board, int depth, int radius) {
    Searcher reference(Algorithm::MINIMAX, TableType::NONE, depth, radius);
    SearchResult expected = reference.Recommend(board, CellState::Nought);
    const Algorithm algorithms[2] = {Algorithm::MINIMAX, Algorithm::ALPHABETA};
    const TableType tables[3] = {TableType::NONE, TableType::HASH, TableType::TREE};
    for(Algorithm algorithm : algorithms) {
        for(TableType type : tables) {
            Searcher searcher(algorithm, type, depth, radius);
            SearchResult result = searcher.Recommend(board, CellState::Nought);
            if(result.found != expected.found || result.score != expected.score) {
                return false;
            }
            if(result.found &&
               (result.move.row != expected.move.row || result.move.col != expected.move.col)) {
                return false;
            }
        }
    }
    return true;
}

void TestSearchModesAgree3x3() {
    bool agree = true;
    for(unsigned int seed = 1; seed <= 15; seed++) {
        Board board = RandomBoard(3, 3, 3 + seed % 3, seed);
        if(!AllModesAgree(board, 9, 1)) {
            agree = false;
        }
    }
    assert_func(agree);
}

void TestSearchModesAgree4x4() {
    bool agree = true;
    for(unsigned int seed = 1; seed <= 6; seed++) {
        Board board = RandomBoard(4, 3, 4, seed);
        if(!AllModesAgree(board, 4, 1)) {
            agree = false;
        }
    }
    assert_func(agree);

    Board board = RandomBoard(5, 4, 6, 77);
    assert_func(AllModesAgree(board, 3, 2));
}

void TestSearchPruningAndTable() {
    Board board = RandomBoard(3, 3, 2, 5);
    Searcher plain(Algorithm::MINIMAX, TableType::NONE, 9, 1);
    Searcher pruned(Algorithm::ALPHABETA, TableType::NONE, 9, 1);
    Searcher hash(Algorithm::MINIMAX, TableType::HASH, 9, 1);
    Searcher tree(Algorithm::MINIMAX, TableType::TREE, 9, 1);
    SearchResult a = plain.Recommend(board, CellState::Nought);
    SearchResult b = pruned.Recommend(board, CellState::Nought);
    SearchResult c = hash.Recommend(board, CellState::Nought);
    SearchResult d = tree.Recommend(board, CellState::Nought);
    assert_func(b.nodes < a.nodes);
    assert_func(c.nodes < a.nodes && c.hits > 0);
    assert_func(c.nodes == d.nodes && c.hits == d.hits);
    assert_func(a.hits == 0 && b.hits == 0);
}

void TestBenchmark() {
    Board board = RandomBoard(3, 3, 3, 9);
    ArraySequence<BenchRow> rows = RunSearchBenchmark(board, 9, 1);
    assert_func(rows.GetLength() == 6);
    bool sameScore = true;
    for(const BenchRow& row : rows) {
        if(row.score != rows[0].score || row.ms < 0) {
            sameScore = false;
        }
    }
    assert_func(sameScore);

    ArraySequence<DictBenchRow> dictRows = RunDictionaryBenchmark(200);
    assert_func(dictRows.GetLength() == 6);
    assert_func(dictRows[0].count == 200 && dictRows[0].ms >= 0);

    WriteSearchCsv(rows, "bench_test_search.csv");
    WriteDictionaryCsv(dictRows, "bench_test_dict.csv");
    std::FILE* file = std::fopen("bench_test_search.csv", "r");
    assert_func(file != nullptr);
    if(file != nullptr) {
        std::fclose(file);
    }
    std::remove("bench_test_search.csv");
    std::remove("bench_test_dict.csv");
    assert_func(Throws<std::runtime_error>([&] { WriteSearchCsv(rows, "no_such_dir/file.csv"); }));
}

int main() {
    TestArraySequence();
    TestListSequence();
    TestTreeDictionary();
    TestHashFunctions();
    TestHashDictionary();
    TestHashCollisions();
    TestHashResize();
    TestHashWrongParameters();
    TestHashStringKeys();
    TestBoardCreate();
    TestBoardPlace();
    TestBoardWinner();
    TestBoardDraw();
    TestBoardKey();
    TestBoardMoves();
    TestEvaluate();
    TestPositionGenerator();
    TestSearchWinInOne();
    TestSearchBlock();
    TestSearchGameOver();
    TestSearchWrongParameters();
    TestSearchEmptyBoardIsDraw();
    TestSearchModesAgree3x3();
    TestSearchModesAgree4x4();
    TestSearchPruningAndTable();
    TestBenchmark();

    printf("Passed: %zu, failed: %zu\n", get_true_tests(), get_failed_tests());
    return get_failed_tests() == 0 ? 0 : 1;
}
