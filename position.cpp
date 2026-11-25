#include "position.h"

#include <iostream>
#include <sstream>
#include <cctype>

#include <random>

U64 pieceKeys[12][64];
U64 sideKey;
U64 castleKeys[16];
U64 epKeys[8];

void initZobrist() {
    std::mt19937_64 rng(1070372);
    for (int p=0;p<12;p++){
        for(int sq=0;sq<64;sq++){
            pieceKeys[p][sq] = rng();
        }
    }
    sideKey = rng();
    for (int i=0;i<16;++i) castleKeys[i] = rng();
    for (int f=0;f<8;++f) epKeys[f] = rng();
}

const std::string pieceToChar = "PNBRQKpnbrqk";

int charToPiece(char c){
    size_t i = pieceToChar.find(c);
    return (i == std::string::npos) ? NO_PIECE : (int)i;
}

void parseFEN(Position &pos, const std::string &fen) {
    pos = Position{};

    std::istringstream ss(fen);
    std::string board, side, castle, ep;
    ss >> board >> side >> castle >> ep;
    ss >> pos.halfmoveClock;

    int rank = 7, file = 0;
    for (char c : board) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (std::isdigit((unsigned char)c)) {
            file += c - '0';
        } else {
            int p = charToPiece(c);
            if (p != NO_PIECE) setBit(pos.pieces[p], rank * 8 + file);
            file++;
        }
    }

    // move side
    pos.whiteToMove = (side == "w");

    // castling rights
    for (char c : castle) {
        if (c == 'K') pos.castlingRights |= WK_CASTLE;
        if (c == 'Q') pos.castlingRights |= WQ_CASTLE;
        if (c == 'k') pos.castlingRights |= BK_CASTLE;
        if (c == 'q') pos.castlingRights |= BQ_CASTLE; 
    }

    // en passant square
    if (ep != "-" && ep.size() >= 2)
        pos.epSquare = (ep[0] - 'a') + (ep[1] - '1') * 8;

    // occupied squares
    for (int p = WP; p <= WK; ++p) pos.occupied[0] |= pos.pieces[p];
    for (int p = BP; p <= BK; ++p) pos.occupied[1] |= pos.pieces[p];
    pos.occupied[2] = pos.occupied[0] | pos.occupied[1];

    // compute Zobrist key
    pos.key = computeKey(pos);
}

int pieceAt(const Position &pos, int square) {
    for (int p = 0; p < 12; ++p) {
        if (getBit(pos.pieces[p], square)) return p;
    }
    return NO_PIECE;
}

void printBoard(const Position &pos) {
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank + 1) << "  ";
        for (int file = 0; file < 8; ++file) {
            int p = pieceAt(pos, rank * 8 + file);
            std::cout << (p == NO_PIECE ? '.' : pieceToChar[p]) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "\n   a b c d e f g h\n\n";
    std::cout << "Move: " << (pos.whiteToMove ? "White" : "Black")
              << " | castling: " << pos.castlingRights
              << " | ep: "       << pos.epSquare << "\n";
}

bool isSquareAttacked(const Position &pos, int  sq, bool byWhite) {
    U64 occ = pos.occupied[2];

    // Knight attacks
    U64 knights = pos.pieces[byWhite ? WN : BN];
    if (KNIGHT_ATTACKS[sq] & knights) return true;

    // King attacks
    U64 king = pos.pieces[byWhite ? WK : BK];
    if (KING_ATTACKS[sq] & king) return true;

    // Rook and Queen attacks
    U64 rq = pos.pieces[byWhite ? WR : BR] | pos.pieces[byWhite ? WQ : BQ];
    if (getRookAttacks(sq, occ) & rq) return true;

    // Bishop and Queen attacks
    U64 bq = pos.pieces[byWhite ? WB : BB] | pos.pieces[byWhite ? WQ : BQ];
    if (getBishopAttacks(sq, occ) & bq) return true;

    // Pawn attacks
    U64 pawns = pos.pieces[byWhite ? WP : BP];
    if (byWhite) {
        if ((((1ULL << sq) & ~FILE_A) >> 9) & pawns) return true; // capture from left
        if ((((1ULL << sq) & ~FILE_H) >> 7) & pawns) return true; // capture from right
    } else {
        if ((((1ULL << sq) & ~FILE_A) << 7) & pawns) return true; // capture from left
        if ((((1ULL << sq) & ~FILE_H) << 9) & pawns) return true; // capture from right
    }
    return false;
}

bool inCheck(const Position &pos) {
    int kingSquare = lsb(pos.pieces[pos.whiteToMove ? WK : BK]);
    return isSquareAttacked(pos, kingSquare, !pos.whiteToMove);
}

static constexpr auto castlingMask = [] {
    std::array<int, 64> m{};
    for (auto &v : m) v = 15;
    m[E1] = 12; m[A1] = 13; m[H1] = 14;
    m[E8] = 3;  m[A8] = 7;  m[H8] = 11;
    return m;
}();


void doMove(Position &pos, Move move, Undo &undo) {
    undo.key = pos.key; // Save the current key in the undo structure
    int from = fromMove(move), to = toMove(move), flags = flagsMove(move);
    bool white = pos.whiteToMove;
    int us = white ? 0 : 1, them = us ^ 1;
    pos.key ^= sideKey; // Update key for side to move

    int piece = pieceAt(pos, from);
    int capturedPiece = pieceAt(pos, to);
    undo.capturedPiece = capturedPiece;

    // Clear Captured Piece
    if (capturedPiece != NO_PIECE) {
        clearBit(pos.pieces[capturedPiece], to);
        clearBit(pos.occupied[them], to);
        pos.key ^= pieceKeys[capturedPiece][to]; // Update key for captured piece
    }

    // Move Piece
    clearBit(pos.pieces[piece], from);
    pos.key ^= pieceKeys[piece][from]; // Update key for moved piece
    setBit(pos.pieces[piece], to);
    pos.key ^= pieceKeys[piece][to]; // Update key for moved piece
    clearBit(pos.occupied[us], from);
    setBit(pos.occupied[us], to);
    undo.castlingRights = pos.castlingRights;
    undo.epSquare = pos.epSquare;

    //en passant capture
    if (flags == EP_CAPTURE) {
        int capSquare = white ? to - 8 : to + 8;
        int capPiece = white ? BP : WP;
        clearBit(pos.pieces[capPiece], capSquare);
        clearBit(pos.occupied[them], capSquare);
        pos.key ^= pieceKeys[capPiece][capSquare];
        undo.epSquare = pos.epSquare; 
    }

    // Castling
    if (flags == KING_CASTLE) {
        int rookfrom = (white ? H1 : H8);
        int rookto = (white ? F1 : F8);
        undo.castlingRights = pos.castlingRights;

        int rookPiece = white ? WR : BR;
        clearBit(pos.pieces[rookPiece], rookfrom);
        pos.key ^= pieceKeys[rookPiece][rookfrom]; // Update key for rook move
        setBit(pos.pieces[rookPiece], rookto);
        pos.key ^= pieceKeys[rookPiece][rookto]; // Update key for rook move
        clearBit(pos.occupied[us], rookfrom);
        setBit(pos.occupied[us], rookto);
    }
    if (flags == QUEEN_CASTLE) {
        int rookfrom = (white ? A1 : A8);
        int rookto = (white ? D1 : D8);
        undo.castlingRights = pos.castlingRights;

        int rookPiece = white ? WR : BR;
        clearBit(pos.pieces[rookPiece], rookfrom);
        pos.key ^= pieceKeys[rookPiece][rookfrom]; // Update key for rook move
        setBit(pos.pieces[rookPiece], rookto);
        pos.key ^= pieceKeys[rookPiece][rookto]; // Update key for rook move
        clearBit(pos.occupied[us], rookfrom);
        setBit(pos.occupied[us], rookto);
    }

    // Promotion
    if (flags >= PROMO_N) {
        int promoted = us * 6 + WN + (flags & 3);
        clearBit(pos.pieces[piece], to); // Remove pawn
        setBit(pos.pieces[promoted], to); // Add promoted piece
        pos.key ^= pieceKeys[piece][to]; // Update key for pawn removal
        pos.key ^= pieceKeys[promoted][to]; // Update key for promoted piece
    }

    //Update castling rights
    pos.key ^= castleKeys[pos.castlingRights]; // Remove old castling rights from key
    pos.castlingRights &= castlingMask[from];
    pos.castlingRights &= castlingMask[to];
    pos.key ^= castleKeys[pos.castlingRights]; // Add new castling rights to key

    // Update en passant square
    if (pos.epSquare != -1) pos.key ^= epKeys[pos.epSquare % 8]; // Remove old ep square from key
    pos.epSquare = (flags == DOUBLE_PUSH) ? (white ? to - 8 : to + 8) : -1;
    if (pos.epSquare != -1) pos.key ^= epKeys[pos.epSquare % 8]; // Add new ep square to key

    // Update halfmove clock
    undo.halfmoveClock = pos.halfmoveClock;
    bool isPawn = (piece == WP || piece == BP);
    pos.halfmoveClock = (isPawn || capturedPiece != NO_PIECE) ? 0 : pos.halfmoveClock + 1;

    // Update occupied squares and side to move
    pos.occupied[2] = pos.occupied[0] | pos.occupied[1];
    pos.whiteToMove = !pos.whiteToMove;
}

void undoMove(Position &pos, Move move, const Undo &undo) {
    pos.key = undo.key; // Restore the Zobrist key from the undo structure
    int from = fromMove(move), to = toMove(move), flags = flagsMove(move);
    bool white = !pos.whiteToMove; 
    int us = white ? 0 : 1, them = us ^ 1;

    int piece = pieceAt(pos, to);

    // Move Piece back
    clearBit(pos.pieces[piece], to);
    setBit(pos.pieces[piece], from);
    clearBit(pos.occupied[us], to);
    setBit(pos.occupied[us], from);

    // Restore captured piece
    if (undo.capturedPiece != NO_PIECE) {
        setBit(pos.pieces[undo.capturedPiece], to);
        setBit(pos.occupied[them], to);
    }

    // Undo en passant capture
    if (flags == EP_CAPTURE) {
        int capSquare = white ? to - 8 : to + 8;
        int capPiece = white ? BP : WP;
        setBit(pos.pieces[capPiece], capSquare);
        setBit(pos.occupied[them], capSquare);
    }

    // Undo castling
    if (flags == KING_CASTLE) {
        int rookfrom = (white ? H1 : H8);
        int rookto = (white ? F1 : F8);

        int rookPiece = white ? WR : BR;
        clearBit(pos.pieces[rookPiece], rookto);
        setBit(pos.pieces[rookPiece], rookfrom);
        clearBit(pos.occupied[us], rookto);
        setBit(pos.occupied[us], rookfrom);
    } else if (flags == QUEEN_CASTLE) {
        int rookfrom = (white ? A1 : A8);
        int rookto = (white ? D1 : D8);

        int rookPiece = white ? WR : BR;
        clearBit(pos.pieces[rookPiece], rookto);
        setBit(pos.pieces[rookPiece], rookfrom);
        clearBit(pos.occupied[us], rookto);
        setBit(pos.occupied[us], rookfrom);
    }

    // Undo promotion
    if (flags >= PROMO_N) {
        int promoted = us * 6 + WN + (flags & 3);
        clearBit(pos.pieces[promoted], from);
        setBit(pos.pieces[white ? WP : BP], from);
    }

    // Restore castling rights, en passant square, and halfmove clock
    pos.castlingRights = undo.castlingRights;
    pos.epSquare = undo.epSquare;
    pos.halfmoveClock = undo.halfmoveClock;
    
    // Update occupied squares and side to move
    pos.occupied[2] = pos.occupied[0] | pos.occupied[1];
    pos.whiteToMove = !pos.whiteToMove;
}

U64 computeKey(const Position &pos) {
    U64 key = 0;
    for (int p=0; p<12; p++){
        U64 pieces = pos.pieces[p];
        while (pieces) {
            int sq = popLsb(pieces);
            key ^= pieceKeys[p][sq];
        }
    }

    if (!pos.whiteToMove) key ^= sideKey;
    key ^= castleKeys[pos.castlingRights];
    if (pos.epSquare != -1) key ^= epKeys[pos.epSquare % 8];

    return key;
}