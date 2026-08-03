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

// รวม flag ของทุกตาที่เดินจาก from ไป to เป็น bitmask (bit n ติด = เจอ flag n)
// ใช้ mask เพราะโปรโมชั่นมีหลายตาที่ from/to ซ้ำกันแต่ flag ต่างกัน
int collectPawnFlags(const std::string &fen, int from, int to) {
    Position pos;
    parseFEN(pos, fen);
    MoveList list;
    genPawnMoves(pos, list);

    int mask = 0;
    for (int i = 0; i < list.count; ++i)
        if (fromMove(list.moves[i]) == from && toMove(list.moves[i]) == to)
            mask |= 1 << flagsMove(list.moves[i]);
    return mask;   // 0 = ไม่มีตานี้เลย
}

int countCastling(const std::string &fen) {
    Position pos;
    parseFEN(pos, fen);
    MoveList list;
    genCastling(pos, list);
    return list.count;
}

// มีตาเข้าป้อม from→to ที่ flag ตรงตามที่ขอไหม
bool hasCastle(const std::string &fen, int from, int to, int flag) {
    Position pos;
    parseFEN(pos, fen);
    MoveList list;
    genCastling(pos, list);

    for (int i = 0; i < list.count; ++i)
        if (fromMove(list.moves[i]) == from && toMove(list.moves[i]) == to &&
            flagsMove(list.moves[i]) == flag)
            return true;
    return false;
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

void testPawnMoveFlags() {
    // invariant ของ encoding เอง — เช็คตั้งแต่ตอน compile
    static_assert((EP_CAPTURE  & CAPTURE) != 0, "ep ต้องมีบิต CAPTURE ติด");
    static_assert((PROMO_Q_CAP & CAPTURE) != 0, "capture-promo ต้องมีบิต CAPTURE ติด");
    static_assert((PROMO_Q_CAP & 3) == (PROMO_Q & 3), "บิตบอกตัวที่โปรโมทต้องตรงกันทั้งสองฝั่ง");

    // เดินธรรมดา / เดินสองช่อง
    assert(collectPawnFlags("8/8/8/8/8/8/4P3/8 w - - 0 1", E2, E3) == (1 << QUIET));
    assert(collectPawnFlags("8/8/8/8/8/8/4P3/8 w - - 0 1", E2, E4) == (1 << DOUBLE_PUSH));
    assert(collectPawnFlags("8/3p4/8/8/8/8/8/8 b - - 0 1", D7, D5) == (1 << DOUBLE_PUSH));

    // กินธรรมดา
    assert(collectPawnFlags("8/8/8/3p4/4P3/8/8/8 w - - 0 1", E4, D5) == (1 << CAPTURE));

    // โปรโมท: ต้องออกครบ 4 ตัว ไม่ใช่แค่ควีน
    assert(collectPawnFlags("8/4P3/8/8/8/8/8/8 w - - 0 1", E7, E8) ==
           ((1 << PROMO_N) | (1 << PROMO_B) | (1 << PROMO_R) | (1 << PROMO_Q)));

    // โปรโมทพร้อมกิน: ต้องเป็นฝั่ง _CAP ทั้ง 4
    assert(collectPawnFlags("3q4/4P3/8/8/8/8/8/8 w - - 0 1", E7, D8) ==
           ((1 << PROMO_N_CAP) | (1 << PROMO_B_CAP) | (1 << PROMO_R_CAP) | (1 << PROMO_Q_CAP)));

    // en passant — ช่องปลายทางว่าง ถ้าไม่ใส่ flag ตานี้จะกลายเป็น QUIET
    // แล้ว makeMove() จะไม่รู้ว่าต้องลบเบี้ยที่ to-8 / to+8
    assert(collectPawnFlags("8/8/8/3pP3/8/8/8/8 w - d6 0 1", E5, D6) == (1 << EP_CAPTURE));
    assert(collectPawnFlags("8/8/8/8/3Pp3/8/8/8 b - d3 0 1", E4, D3) == (1 << EP_CAPTURE));

    std::cout << "pawn move flags OK\n";
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

void testIsSquareAttacked() {
    // เบี้ยดำที่ d5 โจมตี c4 กับ e4 (มันเดินลงล่าง)
    Position p; parseFEN(p, "8/8/8/3p4/8/8/8/8 w - - 0 1");
    assert(isSquareAttacked(p, C4, false));
    assert(isSquareAttacked(p, E4, false));
    assert(!isSquareAttacked(p, D4, false));   // เบี้ยไม่โจมตีตรงหน้าตัวเอง
    assert(!isSquareAttacked(p, C6, false));   // ไม่ใช่ทิศทางที่ถูก

    // เบี้ยขาวที่ d4 โจมตีขึ้นบน
    parseFEN(p, "8/8/8/8/3P4/8/8/8 w - - 0 1");
    assert(isSquareAttacked(p, C5, true) && isSquareAttacked(p, E5, true));
    assert(!isSquareAttacked(p, C3, true));

    // เบี้ยริมกระดาน — เคสที่ shift แล้ว wrap ข้ามไฟล์ พังบ่อยที่สุด
    parseFEN(p, "8/8/8/8/8/8/P7/8 w - - 0 1");   // เบี้ยขาว a2
    assert(isSquareAttacked(p, B3, true));
    assert(!isSquareAttacked(p, H3, true));      // ห้าม wrap ไปอีกฝั่งกระดาน
    parseFEN(p, "8/7p/8/8/8/8/8/8 w - - 0 1");   // เบี้ยดำ h7
    assert(isSquareAttacked(p, G6, false));
    assert(!isSquareAttacked(p, A6, false));

    // ม้า d4
    parseFEN(p, "8/8/8/8/3N4/8/8/8 w - - 0 1");
    assert(isSquareAttacked(p, E6, true) && isSquareAttacked(p, B5, true));
    assert(!isSquareAttacked(p, D5, true));

    // คิง e1 คุมช่องรอบตัวเท่านั้น
    parseFEN(p, "8/8/8/8/8/8/8/4K3 w - - 0 1");
    assert(isSquareAttacked(p, D2, true) && isSquareAttacked(p, F1, true));
    assert(!isSquareAttacked(p, E3, true));

    // เรือถูกบล็อก
    parseFEN(p, "8/8/8/8/8/8/8/R2P3r w - - 0 1");
    assert(isSquareAttacked(p, C1, true));     // เรือขาวถึง C1
    assert(!isSquareAttacked(p, E1, true));    // เบี้ย D1 บล็อกอยู่

    // ควีนดำ d5 ทแยงลงขวา ชนเบี้ยขาว f3 → ถึง f3 แต่ไม่เลยไป g2
    parseFEN(p, "8/8/8/3q4/8/5P2/8/8 w - - 0 1");
    assert(isSquareAttacked(p, E4, false) && isSquareAttacked(p, F3, false));
    assert(!isSquareAttacked(p, G2, false));

    // inCheck ต้องดูคิงของฝ่ายที่ถึงตาเดินเท่านั้น
    parseFEN(p, "4k3/8/8/8/8/8/8/4K2r w - - 0 1");
    assert(inCheck(p));                        // เรือดำ h1 รุกคิงขาว e1
    parseFEN(p, "4k3/8/8/8/8/8/8/4KB1r w - - 0 1");
    assert(!inCheck(p));                       // บิชอป f1 บังอยู่

    std::cout << "isSquareAttacked OK\n";
}

void testCastling() {
    // สิทธิ์ครบทั้งสองฝั่ง
    const char *wBoth = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
    assert(countCastling(wBoth) == 2);
    assert(hasCastle(wBoth, E1, G1, KING_CASTLE));
    assert(hasCastle(wBoth, E1, C1, QUEEN_CASTLE));

    // สิทธิ์ฝั่งเดียว — ต้องอ่านบิตของ castlingRights ให้ตรงฝั่ง
    // parseFEN เก็บบิตเป็น K=1 Q=2 k=4 q=8 (position.cpp:41-44)
    const char *wK = "r3k2r/8/8/8/8/8/8/R3K2R w K - 0 1";
    assert(countCastling(wK) == 1);
    assert(hasCastle(wK, E1, G1, KING_CASTLE));

    const char *wQ = "r3k2r/8/8/8/8/8/8/R3K2R w Q - 0 1";
    assert(countCastling(wQ) == 1);
    assert(hasCastle(wQ, E1, C1, QUEEN_CASTLE));

    // ฝั่งดำ
    const char *bBoth = "r3k2r/8/8/8/8/8/8/R3K2R b kq - 0 1";
    assert(countCastling(bBoth) == 2);
    assert(hasCastle(bBoth, E8, G8, KING_CASTLE));
    assert(hasCastle(bBoth, E8, C8, QUEEN_CASTLE));

    const char *bK = "r3k2r/8/8/8/8/8/8/R3K2R b k - 0 1";
    assert(countCastling(bK) == 1);
    assert(hasCastle(bK, E8, G8, KING_CASTLE));

    // ไม่มีสิทธิ์เลย
    assert(countCastling("r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1") == 0);

    // มีหมากขวางทาง
    assert(countCastling("r3k2r/8/8/8/8/8/8/R3KB1R w KQ - 0 1") == 1);  // f1 ตัน → เหลือฝั่งควีน
    assert(countCastling("r3k2r/8/8/8/8/8/8/RN2K2R w KQ - 0 1") == 1);  // b1 ตัน → เหลือฝั่งคิง

    // ห้ามเข้าป้อมออกจากการถูกรุก
    assert(countCastling("4r3/8/8/8/8/8/8/R3K2R w KQ - 0 1") == 0);

    // ช่องที่คิงเดินผ่านถูกโจมตี → ห้าม
    // บิชอปดำ f3 คุม d1 → ฝั่งควีนห้าม แต่ฝั่งคิงยังได้
    assert(countCastling("8/8/8/8/8/5b2/8/R3K2R w KQ - 0 1") == 1);
    assert(hasCastle("8/8/8/8/8/5b2/8/R3K2R w KQ - 0 1", E1, G1, KING_CASTLE));

    // b1 ถูกโจมตี "ไม่" ห้ามเข้าป้อมฝั่งควีน เพราะคิงไม่ได้เดินผ่าน b1
    // (b1 แค่ต้องว่างให้เรือผ่าน) — กติกาข้อที่พลาดกันบ่อย
    assert(countCastling("1r6/8/8/8/8/8/8/R3K2R w KQ - 0 1") == 2);

    std::cout << "castling OK\n";
}

int main() {
    initAttacksTables();
    testAttackTables();
    testMoveGen();
    testPawnMoves();
    testPawnMoveFlags();
    testRookBishopQueenAttacks();
    testIsSquareAttacked();
    testCastling();
    std::cout << "all tests passed\n";
    return 0;
}