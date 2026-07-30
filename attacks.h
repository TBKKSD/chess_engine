#pragma once
#include "type.h"
#include "position.h"

extern U64 PAWN_ATTACKS[2][64];
extern U64 KNIGHT_ATTACKS[64];
extern U64 KING_ATTACKS[64];

void initAttacksTables();