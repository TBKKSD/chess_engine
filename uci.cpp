#include "uci.h"

Move stringToMove(Position &pos, const std::string &str) {
    MoveList list;
    genLegalMoves(pos, list);
    for (int i = 0; i < list.count; ++i)
        if (moveToString(list.moves[i]) == str)
            return list.moves[i];
    return 0;
}

void uciLoop() {
    Position pos;
    parseFEN(pos, START_FEN);
    std::string line;

    while (std::getline(std::cin, line)) {
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "uci") {
            std::cout << "id name MyEngine 0.1\n"
                      << "id author " << "..." << "\n"
                      << "uciok" << std::endl;
        }
        else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (token == "ucinewgame") {
            parseFEN(pos, START_FEN);
        }
        else if (token == "position") {
            handlePosition(pos, ss);
        }
        else if (token == "go") {
            handleGo(pos, ss);
        }
        else if (token == "quit") {
            break;
        }
        else {
            std::cerr << "unknown command :" << token << "\n" ;
        }
    }
}

void handlePosition(Position &pos, std::istringstream &ss) {
    std::string token;
    ss >> token;

    if (token == "startpos") {
        parseFEN(pos, START_FEN);
        ss >> token;
    } else if (token == "fen") {
        std::string fen, part;
        for (int i = 0; i < 6 && ss >> part && part != "moves"; ++i)
            fen += part + " ";
        parseFEN(pos, fen);
        if (part == "moves") token = part;
        else ss >> token ;
    }

    if (token == "moves") {
        std::string moveStr;
        while (ss >> moveStr) {
            Move m = stringToMove(pos, moveStr);
            if (m) {
                Undo undo;
                doMove(pos, m, undo);
            }
        }
    }
}

void handleGo(Position &pos, std::istringstream &ss) {
    int depth = 6;
    int wtime = 0, btime = 0, movetime = 0;
    std::string token;

    while (ss >> token) {
        if (token == "depth")         ss >> depth;
        else if (token == "movetime") ss >> movetime;
        else if (token == "wtime")    ss >> wtime;
        else if (token == "btime")    ss >> btime;
    }

    
    int timeLimit = movetime;
    if (timeLimit == 0) {
        int myTime = pos.whiteToMove ? wtime : btime;
        if (myTime) timeLimit = myTime / 30;
    }
    if (timeLimit == 0) timeLimit = 5000;

    Move best = searchPosition(pos, depth, timeLimit);
    if (best == 0) std::cout << "bestmove (none)" << std::endl;
    else std::cout << "bestmove " << moveToString(best) << std::endl;
     
}

