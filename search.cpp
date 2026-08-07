#include "search.h"

int search(Position &pos, int  depth, int alpha, int beta, int ply) {
    if (depth == 0) return quiescence(pos, alpha, beta);

    MoveList list;
    genLegalMoves(pos, list);
    if (list.count == 0)
        return inCheck(pos) ? -MATE_SCORE + ply : 0;

    orderMoves(pos, list);

    for (int i = 0; i < list.count; i++) {
        Undo undo;
        doMove(pos, list.moves[i],undo);
        int score = -search(pos, depth - 1, -beta, -alpha, ply + 1);
        undoMove(pos,list.moves[i],undo);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

int quiescence(Position &pos, int alpha, int beta) {
    int standPat = evaluate(pos);        // "ถ้าไม่ทำอะไรเลยได้เท่านี้"
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    MoveList list ;
    genLegalMoves(pos,list);
    MoveList CaptureList ;
    for (int i = 0; i < list.count; ++i) {
        if (!(flagsMove(list.moves[i]) & CAPTURE)) continue;
        CaptureList.add(list.moves[i]);
    }
    orderMoves(pos, CaptureList);

    for (int i = 0; i < CaptureList.count; ++i) {
        Undo undo;
        doMove(pos,CaptureList.moves[i],undo);
        int score = -quiescence(pos, -beta, -alpha);
        undoMove(pos,CaptureList.moves[i],undo);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

void orderMoves(const Position &pos, MoveList &list) {
    int scores[256];
    for (int i = 0; i < list.count; ++i) {
        Move m = list.moves[i];
        int victim = pieceAt(pos, toMove(m));
        int attacker = pieceAt(pos, fromMove(m));
        scores[i] = (victim != NO_PIECE)
            ? MVV_LVA[victim % 6][attacker % 6] + 10000
            : 0;
        if (flagsMove(m) == EP_CAPTURE)
            scores[i] = MVV_LVA[0][0] + 10000;
    }
    // insertion sort
    for (int i = 1; i < list.count; ++i) {
        Move m = list.moves[i]; int s = scores[i], j = i - 1;
        while (j >= 0 && scores[j] < s) {
            list.moves[j+1] = list.moves[j]; scores[j+1] = scores[j]; j--;
        }
        list.moves[j+1] = m; scores[j+1] = s;
    }
}

Move searchPosition(Position &pos, int maxDepth) {
    Move bestMove = 0;
    for (int depth = 1; depth <= maxDepth; ++depth) {
        int alpha = -INF, beta = INF;
        MoveList list;
        genLegalMoves(pos, list);
        orderMoves(pos, list);

        for (int i = 0; i < list.count; ++i) {
            Undo undo;
            doMove(pos, list.moves[i], undo);
            int score = -search(pos, depth - 1, -beta, -alpha, 1);
            undoMove(pos, list.moves[i],undo);
            if (score > alpha) { alpha = score; bestMove = list.moves[i]; }
        }
    }
    return bestMove;
}