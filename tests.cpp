// tests.cpp
#include <cassert>
#include <iostream>
#include "attacks.h"
#include "movegen.h"
#include "type.h"
#include "position.h"

void testAttackTables() {
    // 1. ผลรวมทั้งกระดาน — ตัวเลขนี้คงที่ พิสูจน์ได้
    int knightTotal = 0, kingTotal = 0;
    for (int sq = 0; sq < 64; ++sq) {
        knightTotal += popcount(KNIGHT_ATTACKS[sq]);
        kingTotal   += popcount(KING_ATTACKS[sq]);
    }
    assert(knightTotal == 336);
    assert(kingTotal   == 420);

    // 2. มุมกระดาน — เคสที่ wrap พังบ่อยที่สุด
    assert(popcount(KNIGHT_ATTACKS[A1]) == 2);
    assert(popcount(KNIGHT_ATTACKS[H1]) == 2);
    assert(popcount(KNIGHT_ATTACKS[A8]) == 2);
    assert(popcount(KNIGHT_ATTACKS[H8]) == 2);
    assert(popcount(KING_ATTACKS[A1])   == 3);
    assert(popcount(KING_ATTACKS[H8])   == 3);
    assert(popcount(KNIGHT_ATTACKS[D4]) == 8);

    // 3. สมมาตร — ถ้า a โจมตี b ได้ b ก็ต้องโจมตี a ได้
    for (int a = 0; a < 64; ++a)
        for (int b = 0; b < 64; ++b) {
            assert(getBit(KNIGHT_ATTACKS[a], b) == getBit(KNIGHT_ATTACKS[b], a));
            assert(getBit(KING_ATTACKS[a], b)   == getBit(KING_ATTACKS[b], a));
        }

    // 4. ไม่มีช่องไหนโจมตีตัวเอง
    for (int sq = 0; sq < 64; ++sq) {
        assert(!getBit(KNIGHT_ATTACKS[sq], sq));
        assert(!getBit(KING_ATTACKS[sq], sq));
    }
    std::cout << "attack tables OK\n";
}

int countMoves(const std::string &fen) {
    Position pos;
    parseFEN(pos, fen);
    MoveList list;
    genKnightMoves(pos, list);
    genKingMoves(pos, list);
    return list.count;
}

void testMoveGen() {
    // ม้าโล่ง ๆ กลางกระดาน: ม้า d5 = 8 ทาง, คิง a1 = 3 ทาง
    assert(countMoves("8/8/8/3N4/8/8/8/K6k w - - 0 1") == 11);

    // เบี้ยขาว (พวกเดียวกัน) ยืนขวางที่ c7,e7 → ม้าเหลือ 6
    assert(countMoves("8/2P1P3/8/3N4/8/8/8/K6k w - - 0 1") == 9);

    // เปลี่ยนเป็นเบี้ยดำ → กินได้ กลับมา 8 เท่าเดิม
    assert(countMoves("8/2p1p3/8/3N4/8/8/8/K6k w - - 0 1") == 11);

    // ม้า a1 (มุม) = 2 ทาง, ม้า g1 = 3 ทาง (e2,f3,h3), ไม่มีคิงขาว = 0
    assert(countMoves("8/8/8/8/8/8/8/N5Nk w - - 0 1") == 2 + 3 + 0);

    std::cout << "movegen OK\n";
}

int main() {
    initAttacksTables();
    testAttackTables();
    testMoveGen();
    std::cout << "all tests passed\n";
    return 0;
}