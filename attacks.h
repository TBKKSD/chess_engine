#pragma once
#include "type.h"
#include "position.h"

extern U64 PAWN_ATTACKS[2][64];
extern U64 KNIGHT_ATTACKS[64];
extern U64 KING_ATTACKS[64];
extern U64 ROOK_ATTACKS[64];
extern U64 BISHOP_ATTACKS[64];
extern U64 QUEEN_ATTACKS[64];
extern U64 getRookAttacks(int square, U64 blockers);
extern U64 getBishopAttacks(int square, U64 blockers);
extern U64 getQueenAttacks(int square, U64 blockers);

void initAttacksTables();