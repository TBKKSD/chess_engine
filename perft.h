#pragma once
#include "position.h"
#include "movegen.h"
#include <string>

// จำนวน node ที่ระดับลึก depth — นับเฉพาะตา legal
U64 perft(Position &pos, int depth);

// แจกแจงจำนวน node ต่อตาระดับบนสุด รูปแบบเดียวกับ "go perft N" ของ Stockfish
// เอาผลไป diff กันได้ตรงๆ เพื่อหาว่าสาขาไหนผิด
U64 perftDivide(Position &pos, int depth);

// รูปแบบ UCI: e2e4, e7e8q
std::string moveToString(Move move);
