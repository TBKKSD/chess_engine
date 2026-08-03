#include "position.h"

#include <string>

int main() {
    Position pos;
    std::string fen = "8/8/8/3R4/8/8/8/K6k w - - 0 1";
    parseFEN(pos, fen);
    printBoard(pos);
    return 0;
}
