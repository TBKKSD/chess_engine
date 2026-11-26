#include "search.h"
#include <chrono>
using Clock = std::chrono::steady_clock;

static Clock::time_point searchStart;
static int      searchTimeLimit; // ms
static bool     stopSearch;
static uint64_t nodeCount;

static inline bool timeUp() {
    if (searchTimeLimit <= 0) return false;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  Clock::now() - searchStart).count();
    return ms >= searchTimeLimit;
}

int toTT(int score, int ply) {
    return score > MATE_SCORE - 1000 ? score + ply : score < -MATE_SCORE + 1000 ? score - ply : score;
}

int search(Position &pos, int  depth, int alpha, int beta, int ply) {
    if (stopSearch) return 0;
    if ((++nodeCount & 2047) == 0 && timeUp()) { stopSearch = true; return 0; }

    int alphaOrig = alpha;
    Move ttMove = 0;
    TTEntry entry;
    if (ttProbe(pos.key, entry)) {
        ttMove = entry.bestMove;
        if (entry.depth >= depth) {
            int score = entry.score;
            if (score > MATE_SCORE - 1000) score -= ply; // Adjust score for mate distance
            if (score < -MATE_SCORE + 1000) score += ply; // Adjust score for mate distance
            if (entry.flag == TT_EXACT) return score;
            else if (entry.flag == TT_LOWER && score >= beta) return score;
            else if (entry.flag == TT_UPPER && score <= alpha) return score;

        }
    }
    if (depth == 0) return quiescence(pos, alpha, beta);

    MoveList list;
    genLegalMoves(pos, list);
    if (list.count == 0)
        return inCheck(pos) ? -MATE_SCORE + ply : 0;

    orderMoves(pos, list, ttMove);

    Move bestMove = 0;
    for (int i = 0; i < list.count; i++) {
        Undo undo;
        doMove(pos, list.moves[i],undo);
        int score = -search(pos, depth - 1, -beta, -alpha, ply + 1);
        undoMove(pos,list.moves[i],undo);

        if (stopSearch) return 0;
        if (score >= beta)  {
            ttStore(pos.key, depth, toTT(score, ply), TT_LOWER, list.moves[i]);
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            bestMove = list.moves[i];
        }
    }
    ttStore(pos.key, depth, toTT(alpha, ply), alpha > alphaOrig ? TT_EXACT : TT_UPPER, bestMove);
    return alpha;
}

int quiescence(Position &pos, int alpha, int beta) {
    if (stopSearch) return 0;
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

void orderMoves(const Position &pos, MoveList &list, Move ttMove) {
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
        if (m == ttMove) scores[i] += 1000000; // TT move gets highest priority
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

Move searchPosition(Position &pos, int maxDepth, int timeLimitMs) {
    searchStart = Clock::now() ;
    stopSearch = false ;
    nodeCount = 0 ;
    searchTimeLimit = timeLimitMs;

    Move bestMove = 0;
    for (int depth = 1; depth <= maxDepth; ++depth) {
        int iterBest = 0;
        int alpha = -INF, beta = INF;
        MoveList list;
        genLegalMoves(pos, list);
        orderMoves(pos, list, bestMove);

        for (int i = 0; i < list.count; ++i) {
            Undo undo;
            doMove(pos, list.moves[i], undo);
            int score = -search(pos, depth - 1, -beta, -alpha, 1);
            undoMove(pos, list.moves[i],undo);
            if (score > alpha) { alpha = score; iterBest = list.moves[i]; }
        }
        if (stopSearch) break;
        bestMove = iterBest;
        ttStore(pos.key, depth, alpha, TT_EXACT, bestMove);
    }
    return bestMove;
}
