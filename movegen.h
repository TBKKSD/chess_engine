#pragma once
#include "type.h"
#include "position.h"
#include "attacks.h"

void genKnightMoves(Position &pos, MoveList &moves);
void genKingMoves(Position &pos, MoveList &moves);
void genPawnMoves(Position &pos, MoveList &moves);
void genSlidingMoves(const Position &pos, MoveList &moves);
void genCastling(const Position &pos, MoveList &moves);

void genAllMoves(Position &pos, MoveList &moves);
void genLegalMoves(Position &pos, MoveList &moves);