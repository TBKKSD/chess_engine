#pragma once

#include "position.h"
#include "type.h"
#include "movegen.h"
#include "eval.h"

constexpr int MATE_SCORE = 30000;
constexpr int INF        = 31000;

constexpr int MVV_LVA[6][6] = {
    {105,104,103,102,101,100},
    {205,204,203,202,201,200},
    {305,304,303,302,301,300},
    {405,404,403,402,401,400},
    {505,504,503,502,501,500},
    {0,0,0,0,0,0}
};

int  search(Position &pos, int depth, int alpha, int beta, int ply);
int  quiescence(Position &pos, int alpha, int beta);
void orderMoves(const Position &pos, MoveList &list);
Move searchPosition(Position &pos, int maxDepth, int timeLimitMs = 0);
