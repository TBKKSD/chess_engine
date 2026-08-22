#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include "type.h"
#include "movegen.h"
#include "position.h"
#include "perft.h"
#include "search.h"

Move stringToMove(Position &pos, const std::string &str);

const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void uciLoop();
void handlePosition(Position &pos, std::istringstream &ss);
void handleGo(Position &pos, std::istringstream &ss);