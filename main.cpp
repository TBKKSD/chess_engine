#include "position.h"
#include "uci.h"
#include <string>

int main() {
    std::cout << std::unitbuf;
    initAttacksTables();
    uciLoop();
    return 0;
}
