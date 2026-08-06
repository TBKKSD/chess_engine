// tests.cpp
#include <cassert>
#include <iostream>
#include "attacks.h"
#include "movegen.h"
#include "type.h"
#include "position.h"
#include "perft.h"
#include "eval.h"
#include <sstream>
#include <vector>

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

// รวม pseudo-legal ทุกชนิดไว้ที่เดียว (ชั่วคราวสำหรับเทส — ของจริงควรอยู่ใน movegen.cpp)
void genAllForTest(Position &pos, MoveList &list) {
    genPawnMoves(pos, list);
    genKnightMoves(pos, list);
    genKingMoves(pos, list);
    genSlidingMoves(pos, list);
    genCastling(pos, list);
}

// ตาที่ตั้งบิต CAPTURE ต้องเท่ากับตาที่กินหมากจริงเป๊ะ ไม่ขาดไม่เกิน
// (ep นับเป็นการกิน ทั้งที่ช่องปลายทางว่าง — เป็นข้อยกเว้นเดียว)
void assertCaptureFlagsMatch(const std::string &fen) {
    Position pos;
    parseFEN(pos, fen);
    MoveList list;
    genAllForTest(pos, list);

    int flagged = 0, actual = 0;
    for (int i = 0; i < list.count; ++i) {
        Move m = list.moves[i];
        if (flagsMove(m) & CAPTURE) flagged++;
        if (flagsMove(m) == EP_CAPTURE || pieceAt(pos, toMove(m)) != NO_PIECE) actual++;
    }
    // cerr เพราะ abort() ของ assert ไม่ flush cout ให้
    if (flagged != actual)
        std::cerr << "  capture flag mismatch: " << fen
                  << "\n    flagged=" << flagged << " actual=" << actual << std::endl;
    assert(flagged == actual);
}

void testCaptureFlags() {
    // หมากที่กระโดด — flag ต่อช่องปลายทาง
    assertCaptureFlagsMatch("4k3/8/8/8/3N4/8/2p5/4K3 w - - 0 1");
    assertCaptureFlagsMatch("4k3/8/8/8/8/8/3p4/3K4 w - - 0 1");

    // หมากที่เลื่อน — ตัวเดียวมีทั้งตากินและตาเดินเปล่าปนกัน
    assertCaptureFlagsMatch("4k3/3p4/8/8/3R4/8/8/4K3 w - - 0 1");
    assertCaptureFlagsMatch("4k3/8/8/8/8/8/1p6/2B1K3 w - - 0 1");
    assertCaptureFlagsMatch("4k3/3p4/8/8/3Q4/8/8/4K3 w - - 0 1");

    // เบี้ย รวม en passant
    assertCaptureFlagsMatch("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
    assertCaptureFlagsMatch("4k3/4P3/8/8/8/8/8/4K3 w - - 0 1");

    // ตำแหน่งจริงที่มีทุกอย่างปนกัน (Kiwipete)
    assertCaptureFlagsMatch("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    assertCaptureFlagsMatch("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1");

    std::cout << "capture flags OK\n";
}

// เทียบทีละ field — ห้ามใช้ memcmp เพราะ Position มี padding หลัง bool ที่ไม่รับประกันค่า
bool samePosition(const Position &a, const Position &b) {
    for (int i = 0; i < 12; ++i) if (a.pieces[i]   != b.pieces[i])   return false;
    for (int i = 0; i < 3;  ++i) if (a.occupied[i] != b.occupied[i]) return false;
    return a.whiteToMove    == b.whiteToMove
        && a.castlingRights == b.castlingRights
        && a.epSquare       == b.epSquare
        && a.halfmoveClock  == b.halfmoveClock;
}

void reportDiff(const Position &before, const Position &after, Move m) {
    std::cerr << "    move from=" << fromMove(m) << " to=" << toMove(m)
              << " flags=" << flagsMove(m) << "\n";
    for (int i = 0; i < 12; ++i)
        if (before.pieces[i] != after.pieces[i])
            std::cerr << "      pieces[" << pieceToChar[i] << "] ต่างกัน\n";
    for (int i = 0; i < 3; ++i)
        if (before.occupied[i] != after.occupied[i])
            std::cerr << "      occupied[" << i << "] ต่างกัน\n";
    if (before.castlingRights != after.castlingRights)
        std::cerr << "      castlingRights " << before.castlingRights
                  << " -> " << after.castlingRights << "\n";
    if (before.epSquare != after.epSquare)
        std::cerr << "      epSquare " << before.epSquare << " -> " << after.epSquare << "\n";
    if (before.halfmoveClock != after.halfmoveClock)
        std::cerr << "      halfmoveClock " << before.halfmoveClock
                  << " -> " << after.halfmoveClock << "\n";
    if (before.whiteToMove != after.whiteToMove)
        std::cerr << "      whiteToMove ไม่กลับ\n";
}

// doMove แล้ว undoMove ต้องได้กระดานเดิมเป๊ะ ทุกตา ไม่มีข้อยกเว้น
void assertRoundTrip(const std::string &fen) {
    Position pos;
    parseFEN(pos, fen);
    MoveList list;
    genAllForTest(pos, list);

    for (int i = 0; i < list.count; ++i) {
        Position before = pos;
        Undo undo{};
        doMove(pos, list.moves[i], undo);
        undoMove(pos, list.moves[i], undo);

        if (!samePosition(before, pos)) {
            std::cerr << "  round-trip FAIL: " << fen << std::endl;
            reportDiff(before, pos, list.moves[i]);
            assert(false && "doMove/undoMove round-trip");
        }
        pos = before;   // กันไม่ให้ตาที่พังลามไปตาถัดไป
    }
}

void testMakeUnmake() {
    assertRoundTrip("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assertRoundTrip("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");      // เข้าป้อมได้ทุกทาง
    assertRoundTrip("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    assertRoundTrip("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");         // en passant ขาว
    assertRoundTrip("4k3/8/8/8/3Pp3/8/8/4K3 b - d3 0 1");         // en passant ดำ
    assertRoundTrip("3q4/4P3/8/8/8/8/8/4K2k w - - 0 1");          // โปรโมท + โปรโมทพร้อมกิน
    assertRoundTrip("4k2K/8/8/8/8/8/4p3/3Q4 b - - 0 1");          // ฝั่งดำโปรโมท
    assertRoundTrip("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    assertRoundTrip("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1");

    std::cout << "make/unmake round-trip OK\n";
}

void assertPerft(const std::string &fen, int depth, U64 expect) {
    Position pos;
    parseFEN(pos, fen);
    U64 got = perft(pos, depth);
    if (got != expect)
        std::cerr << "  perft ไม่ตรง: " << fen << "\n    depth=" << depth
                  << " got=" << got << " want=" << expect << std::endl;
    assert(got == expect);
}

// ค่าอ้างอิงจาก Chess Programming Wiki — ตัวเลขพวกนี้ยืนยันกันมาหลายสิบปีแล้ว
// ถ้าตรงทุกตำแหน่ง แปลว่า movegen + doMove/undoMove + ตัวกรอง legal ถูกทั้งระบบ
void testPerft() {
    const std::string start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    assertPerft(start, 1, 20);
    assertPerft(start, 2, 400);
    assertPerft(start, 3, 8902);
    assertPerft(start, 4, 197281);

    // Kiwipete — ออกแบบมาดักบั๊กเข้าป้อมกับ en passant โดยเฉพาะ
    const std::string kiwi = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    assertPerft(kiwi, 1, 48);
    assertPerft(kiwi, 2, 2039);
    assertPerft(kiwi, 3, 97862);

    // เบี้ยผ่านและ pin ตามแนวนอน
    const std::string pos3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    assertPerft(pos3, 1, 14);
    assertPerft(pos3, 2, 191);
    assertPerft(pos3, 3, 2812);
    assertPerft(pos3, 4, 43238);

    // โปรโมชั่นหนาแน่นทั้งสองฝั่ง (เบี้ยขาว a7, เบี้ยดำ b2)
    const std::string pos4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    assertPerft(pos4, 1, 6);
    assertPerft(pos4, 2, 264);
    assertPerft(pos4, 3, 9467);

    const std::string pos5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    assertPerft(pos5, 1, 44);
    assertPerft(pos5, 2, 1486);
    assertPerft(pos5, 3, 62379);

    const std::string pos6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
    assertPerft(pos6, 1, 46);
    assertPerft(pos6, 2, 2079);
    assertPerft(pos6, 3, 89890);

    std::cout << "perft OK\n";
}

// สลับสีหมากทุกตัว + พลิกกระดานบนล่าง + สลับฝ่ายที่เดิน
// ตำแหน่งที่ได้ต้องมีคะแนนเท่าเดิมเป๊ะ เพราะ evaluate() มองจากฝ่ายที่ถึงตาเดิน
std::string mirrorFEN(const std::string &fen) {
    std::istringstream ss(fen);
    std::string board, side, castle, ep;
    ss >> board >> side >> castle >> ep;

    std::vector<std::string> ranks;
    std::string cur;
    for (char c : board) {
        if (c == '/') { ranks.push_back(cur); cur.clear(); }
        else cur += c;
    }
    ranks.push_back(cur);

    std::string flipped;
    for (int i = (int)ranks.size() - 1; i >= 0; --i) {
        for (char c : ranks[i])
            flipped += std::isalpha((unsigned char)c)
                     ? (std::islower((unsigned char)c) ? std::toupper(c) : std::tolower(c))
                     : c;
        if (i > 0) flipped += '/';
    }

    std::string newCastle;
    for (char c : castle)
        newCastle += std::isalpha((unsigned char)c)
                   ? (std::islower((unsigned char)c) ? std::toupper(c) : std::tolower(c))
                   : c;

    std::string newEp = ep;
    if (ep != "-" && ep.size() >= 2) newEp = std::string(1, ep[0]) + char('0' + (9 - (ep[1] - '0')));

    return flipped + " " + (side == "w" ? "b" : "w") + " " + newCastle + " " + newEp + " 0 1";
}

int evalOf(const std::string &fen) {
    Position pos;
    parseFEN(pos, fen);
    return evaluate(pos);
}

void assertMirrorEqual(const std::string &fen) {
    std::string m = mirrorFEN(fen);
    int a = evalOf(fen), b = evalOf(m);
    if (a != b)
        std::cerr << "  eval ไม่สมมาตร:\n    " << fen << " -> " << a
                  << "\n    " << m << " -> " << b << std::endl;
    assert(a == b);
}

void testEvalSymmetry() {
    assertMirrorEqual("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assertMirrorEqual("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    assertMirrorEqual("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    assertMirrorEqual("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    assertMirrorEqual("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");

    // ตำแหน่งเริ่มเกมสมมาตรสมบูรณ์ ต้องได้ 0 พอดี
    assert(evalOf("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == 0);

    std::cout << "eval symmetry OK\n";
}

void testEvalMaterial() {
    // ขาวได้ควีนเปล่าๆ หนึ่งตัว — ต้องนำอยู่ในระดับเกือบหนึ่งควีน
    int up = evalOf("4k3/8/8/8/8/8/8/3QK3 w - - 0 1");
    assert(up > 700 && up < 1200);

    // ตำแหน่งเดียวกันแต่ดำถึงตาเดิน — คะแนนต้องกลับเครื่องหมายพอดี
    assert(evalOf("4k3/8/8/8/8/8/8/3QK3 b - - 0 1") == -up);

    // เรือ > ม้า > เบี้ย
    int rook   = evalOf("4k3/8/8/8/8/8/8/R3K3 w - - 0 1");
    int knight = evalOf("4k3/8/8/8/8/8/8/N3K3 w - - 0 1");
    int pawn   = evalOf("4k3/8/8/8/8/8/P7/4K3 w - - 0 1");
    assert(rook > knight && knight > pawn && pawn > 0);

    std::cout << "eval material OK\n";
}

void testEvalPieceSquare() {
    // เบี้ยยิ่งใกล้โปรโมทยิ่งดี
    int e2 = evalOf("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1");
    int e5 = evalOf("4k3/8/8/4P3/8/8/8/4K3 w - - 0 1");
    int e7 = evalOf("4k3/4P3/8/8/8/8/8/4K3 w - - 0 1");
    assert(e7 > e5 && e5 > e2);

    // ม้ากลางกระดานดีกว่ามุม
    assert(evalOf("4k3/8/8/8/3N4/8/8/4K3 w - - 0 1") >
           evalOf("4k3/8/8/8/8/8/8/N3K3 w - - 0 1"));

    // บิชอปกลางกระดานดีกว่ามุม
    assert(evalOf("4k3/8/8/8/3B4/8/8/4K3 w - - 0 1") >
           evalOf("4k3/8/8/8/8/8/8/B3K3 w - - 0 1"));

    // เรือบนแถว 7 คือช่องคลาสสิกของเรือ ต้องดีกว่าเรือบนแถว 3
    assert(evalOf("4k3/R7/8/8/8/8/8/4K3 w - - 0 1") >
           evalOf("4k3/8/8/8/8/R7/8/4K3 w - - 0 1"));

    std::cout << "eval piece-square OK\n";
}

void testEvalPhase() {
    // หัวใจของการผสม MG/EG: ช่องคิง "คู่เดิม" ต้องสลับความชอบเมื่อ phase เปลี่ยน
    // ทุกคู่ด้านล่างต่างกันแค่ช่องคิงขาวเท่านั้น หมากอื่นเหมือนกันเป๊ะ

    // กลางเกม (หมากครบ phase=24) — คิงหลังแนวเบี้ยดีกว่าคิงกลางกระดาน
    int mgHome = evalOf("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1");
    int mgOut  = evalOf("rnbqkbnr/pppppppp/8/8/3K4/8/PPPPPPPP/RNBQ1BNR w - - 0 1");
    assert(mgHome > mgOut);

    // ปลายเกม (เหลือแต่คิง phase=0) — ช่องคู่เดิมกลับด้าน คิงกลางกระดานดีกว่า
    int egHome = evalOf("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
    int egOut  = evalOf("4k3/8/8/8/3K4/8/8/8 w - - 0 1");
    assert(egOut > egHome);

    // ปลายเกม คิงกลางกระดานดีกว่าคิงซุกมุมด้วย
    assert(evalOf("4k3/8/8/3K4/8/8/8/7P w - - 0 1") >
           evalOf("4k3/8/8/8/8/8/8/K6P w - - 0 1"));

    std::cout << "eval phase OK\n";
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
    testCaptureFlags();
    testMakeUnmake();
    testPerft();
    testEvalSymmetry();
    testEvalMaterial();
    testEvalPieceSquare();
    testEvalPhase();
    std::cout << "all tests passed\n";
    return 0;
}