#include "position.h"
#include "uci.h"
#include <string>

int main() {
    std::cout << std::unitbuf;
    initZobrist();
    ttInit(64);
    initAttacksTables();
    uciLoop();
    return 0;
}
