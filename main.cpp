#include "position.h"

#include <string>

int main() {
    Position pos;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    parseFEN(pos, fen);
    printBoard(pos);
    return 0;
}
