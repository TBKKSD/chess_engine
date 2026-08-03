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
        if (c == 'K') pos.castlingRights |= 1; // White kingside
        if (c == 'Q') pos.castlingRights |= 2; // White queenside
        if (c == 'k') pos.castlingRights |= 4; // Black kingside
        if (c == 'q') pos.castlingRights |= 8; // Black queenside
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
    int kingSquare = __builtin_ctzll(pos.pieces[pos.whiteToMove ? WK : BK]);
    return isSquareAttacked(pos, kingSquare, !pos.whiteToMove);
}