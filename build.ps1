g++ -O2 -std=c++17 -Wall -Wextra -flto -o engine.exe main.cpp position.cpp attacks.cpp movegen.cpp
if ($?) { .\engine.exe }
