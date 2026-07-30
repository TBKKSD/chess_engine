g++ -g -O0 -std=c++17 -Wall -Wextra -o tests.exe tests.cpp position.cpp attacks.cpp movegen.cpp
if ($?) { .\tests.exe }
