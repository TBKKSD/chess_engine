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

int countPawnMoves(const std::string &fen) {
    Position pos;
    parseFEN(pos, fen);
    MoveList list;
    genPawnMoves(pos, list);
    return list.count;
}

int countSliding(const std::string &fen) {
    Position pos;
    parseFEN(pos, fen);
    MoveList list;
    genSlidingMoves(pos, list);
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

void testPawnMoves() {
    assert(countPawnMoves("8/8/8/8/8/8/4P3/8 w - - 0 1")        == 2);  // e3, e4
    assert(countPawnMoves("8/8/8/8/8/4p3/4P3/8 w - - 0 1")      == 0);  // ถูกบล็อก ห้ามข้าม
    assert(countPawnMoves("8/8/8/3p1p2/4P3/8/8/8 w - - 0 1")    == 3);  // e5, exd5, exf5
    assert(countPawnMoves("8/4P3/8/8/8/8/8/8 w - - 0 1")        == 4);  // โปรโมท 4 แบบ
    assert(countPawnMoves("3q4/4P3/8/8/8/8/8/8 w - - 0 1")      == 8);  // เดิน 4 + กิน 4
    assert(countPawnMoves("8/8/8/3pP3/8/8/8/8 w - d6 0 1")      == 2);  // e6, exd6 e.p.
    assert(countPawnMoves("8/8/8/8/P6p/8/8/8 w - - 0 1")        == 1);  // a5 เท่านั้น
    assert(countPawnMoves("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == 16);

    std::cout << "pawn movegen OK\n";
}

void testRookBishopQueenAttacks() {
    // เรือกลางกระดานโล่ง = 14 ช่องเสมอ (แนวนอน 7 + แนวตั้ง 7)
    assert(popcount(getRookAttacks(D4, 0)) == 14);
    assert(popcount(getRookAttacks(A1, 0)) == 14);   // มุมก็ 14 เหมือนกัน

    // บิชอปขึ้นกับสี่ช่อง: กลาง 13, มุม 7
    assert(popcount(getBishopAttacks(D4, 0)) == 13);
    assert(popcount(getBishopAttacks(A1, 0)) == 7);
    assert(popcount(getQueenAttacks(D4, 0)) == 27);

    // มีตัวบล็อก
    U64 b = (1ULL << D6);
    U64 a = getRookAttacks(D4, b);
    assert(getBit(a, D5) && getBit(a, D6));    // ถึง D6 ได้ (กินได้)
    assert(!getBit(a, D7));                    // เลย D6 ไม่ได้

    // นับหมากเดินจาก FEN
    assert(countSliding("8/8/8/3R4/8/8/8/K6k w - - 0 1") == 14);
    // เรือสองตัวบล็อกกันเองในแนว d — ไม่ใช่ตัวละ 12
    // เรือ d5: ขึ้น 3 (d6-d8) + ลง 3 (d4-d2, ติดเรือตัวเองที่ d1) + แนวนอน 7 = 13
    // เรือ d1: ขึ้น 3 (d2-d4, ติดเรือตัวเองที่ d5) + ซ้าย 2 (c1,b1 ติดคิงตัวเองที่ a1) + ขวา 4 (e1-h1 กินคิงดำได้) = 9
    assert(countSliding("8/8/8/3R4/8/8/8/K2R3k w - - 0 1") == 13 + 9);
    std::cout << "rook, bishop, queen attacks OK\n";
}

int main() {
    initAttacksTables();
    testAttackTables();
    testMoveGen();
    testPawnMoves();
    testRookBishopQueenAttacks();
    std::cout << "all tests passed\n";
    return 0;
}