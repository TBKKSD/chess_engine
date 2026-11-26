#pragma once

#include "type.h"
#include <stdlib.h>
#include <vector>

enum TTFlag : uint8_t {
    TT_EXACT,
    TT_LOWER,
    TT_UPPER
};

struct TTEntry {
    U64 key;
    int16_t score;
    Move bestMove;
    int8_t depth;
    uint8_t flag;
};

void ttInit(int sizeMB);
void ttClear();
bool ttProbe(U64 key, TTEntry &out);
void ttStore(U64 key, int depth,int score, int flag, Move bestMove);