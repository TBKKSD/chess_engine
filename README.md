# Chess Engine

A small UCI chess engine written from scratch in C++17, built step by step: bitboards, move generation, evaluation, search, and finally the UCI protocol so it can talk to chess GUIs.

## Features

- **Board:** bitboard representation (12 piece bitboards + occupancy), FEN parsing, board printing
- **Move generation:** all piece types, castling, en passant, promotion; pseudo-legal moves filtered to legal ones; `doMove` / `undoMove` with an `Undo` record
- **Verification:** perft against known node counts
- **Evaluation:** material plus piece-square tables, tapered between middlegame and endgame by game phase
- **Search:** iterative deepening negamax with alpha-beta, quiescence search on captures, MVV-LVA move ordering, time limit
- **Protocol:** UCI (`uci`, `isready`, `ucinewgame`, `position`, `go`, `quit`)
- **In progress:** Zobrist hashing (`initZobrist`, `computeKey`) as groundwork for a transposition table

## Building

You need `g++` with C++17 support (MinGW on Windows works). On Windows/PowerShell:

```powershell
.\build.ps1         # release build -> engine.exe, then starts it
.\build_tests.ps1   # debug build with asserts -> tests.exe, then runs it
```

Or compile by hand:

```sh
g++ -O2 -std=c++17 -Wall -Wextra -flto -DNDEBUG -o engine.exe \
    main.cpp position.cpp attacks.cpp movegen.cpp perft.cpp eval.cpp search.cpp uci.cpp
```

Compile the release build with `-DNDEBUG` on every file at once. `lsb()` is an inline function in a header and contains an `assert`, so mixing files built with and without it breaks the one-definition rule.

> The `Makefile` is out of date. It doesn't list `perft.cpp`, `eval.cpp`, `search.cpp` or `uci.cpp`, so use the scripts above.

## Usage

The engine reads UCI commands from stdin. You can load it into a GUI such as Cute Chess or Arena, or type commands yourself:

```
uci
isready
position startpos moves e2e4 e7e5
go movetime 1000
bestmove ...
```

`go` accepts `depth`, `movetime`, `wtime` and `btime`. With no time control it searches up to depth 6 or for 5 seconds. With `wtime`/`btime` it uses 1/30 of the remaining clock.

### Self-play

`selfplay.ps1` makes the engine play itself and prints the moves:

```powershell
.\selfplay.ps1                    # 200 ms per move, up to 120 plies
.\selfplay.ps1 -MoveTime 1000     # 1 second per move
.\selfplay.ps1 -MaxPlies 40       # stop after 40 half-moves
.\selfplay.ps1 -Pgn               # print a PGN instead (needs topgn.py)
```

`topgn.py` turns UCI moves into PGN you can paste into [lichess.org/analysis](https://lichess.org/analysis). It needs `pip install chess`:

```powershell
.\selfplay.ps1 | python topgn.py
python topgn.py e2e4 e7e5 g1f3
```

## Project layout

| File | Purpose |
|------|---------|
| `type.h` | Core types, squares, pieces, move encoding |
| `position.h/.cpp` | `Position` struct, FEN, make/unmake move, check detection, Zobrist keys |
| `attacks.h/.cpp` | Precomputed attack tables (knight, king, pawn) and sliding attacks |
| `movegen.h/.cpp` | Pseudo-legal and legal move generation |
| `perft.h/.cpp` | Perft node counting for move generator testing |
| `eval.h/.cpp` | Piece values, piece-square tables, tapered evaluation |
| `search.h/.cpp` | Alpha-beta, quiescence, move ordering, iterative deepening |
| `uci.h/.cpp` | UCI command loop |
| `main.cpp` | Entry point: initializes tables, starts the UCI loop |
| `tests.cpp` | Unit tests: attacks, move generation, perft, make/unmake, eval, search, UCI |
| `selfplay.ps1`, `topgn.py` | Self-play and PGN conversion tools |

## Roadmap

- [ ] Finish Zobrist hashing and keep the key updated incrementally in `doMove`
- [ ] Transposition table
- [ ] Repetition and 50-move rule detection
- [ ] Better move ordering (killer moves, history heuristic)
- [ ] `stop` command and `info` output during search
