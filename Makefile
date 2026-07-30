CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra
SRCS = main.cpp position.cpp attacks.cpp movegen.cpp
OBJS = $(SRCS:.cpp=.o)

engine: $(OBJS)
	$(CXX) $(CXXFLAGS) -flto -o $@ $(OBJS)

tests: tests.cpp position.cpp attacks.cpp movegen.cpp
	$(CXX) -g -O0 -std=c++17 -Wall -Wextra -fsanitize=address,undefined -o $@ $^
	./tests

debug: CXXFLAGS = -g -O0 -std=c++17 -Wall -Wextra -fsanitize=address,undefined
debug: clean engine

clean:
	rm -f $(OBJS) engine