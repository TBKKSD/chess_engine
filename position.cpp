#include "position.h"

#include <iostream>
#include <sstream>
#include <cctype>

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
    int from = fromMove(move), to = toMove(move), flags = flagsMove(move);
    bool white = pos.whiteToMove;
    int us = white ? 0 : 1, them = us ^ 1;

    int piece = pieceAt(pos, from);
    int capturedPiece = pieceAt(pos, to);
    undo.capturedPiece = capturedPiece;

    // Clear Captured Piece
    if (capturedPiece != NO_PIECE) {
        clearBit(pos.pieces[capturedPiece], to);
        clearBit(pos.occupied[them], to);
    }

    // Move Piece
    clearBit(pos.pieces[piece], from);
    setBit(pos.pieces[piece], to);
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
        undo.epSquare = pos.epSquare; 
    }

    // Castling
    if (flags == KING_CASTLE) {
        int rookfrom = (white ? H1 : H8);
        int rookto = (white ? F1 : F8);
        undo.castlingRights = pos.castlingRights;

        int rookPiece = white ? WR : BR;
        clearBit(pos.pieces[rookPiece], rookfrom);
        setBit(pos.pieces[rookPiece], rookto);
        clearBit(pos.occupied[us], rookfrom);
        setBit(pos.occupied[us], rookto);
    }
    if (flags == QUEEN_CASTLE) {
        int rookfrom = (white ? A1 : A8);
        int rookto = (white ? D1 : D8);
        undo.castlingRights = pos.castlingRights;

        int rookPiece = white ? WR : BR;
        clearBit(pos.pieces[rookPiece], rookfrom);
        setBit(pos.pieces[rookPiece], rookto);
        clearBit(pos.occupied[us], rookfrom);
        setBit(pos.occupied[us], rookto);
    }

    // Promotion
    if (flags >= PROMO_N) {
        int promoted = us * 6 + WN + (flags & 3);
        clearBit(pos.pieces[piece], to); // Remove pawn
        setBit(pos.pieces[promoted], to); // Add promoted piece
    }

    //Update castling rights
    pos.castlingRights &= castlingMask[from];
    pos.castlingRights &= castlingMask[to];

    // Update en passant square
    pos.epSquare = (flags == DOUBLE_PUSH) ? (white ? to - 8 : to + 8) : -1;

    // Update halfmove clock
    undo.halfmoveClock = pos.halfmoveClock;
    bool isPawn = (piece == WP || piece == BP);
    pos.halfmoveClock = (isPawn || capturedPiece != NO_PIECE) ? 0 : pos.halfmoveClock + 1;

    // Update occupied squares and side to move
    pos.occupied[2] = pos.occupied[0] | pos.occupied[1];
    pos.whiteToMove = !pos.whiteToMove;
}

void undoMove(Position &pos, Move move, const Undo &undo) {
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