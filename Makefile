CXX = g++
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra
SRCS = main.cpp position.cpp attacks.cpp
# TODO: เพิ่ม movegen.cpp uci.cpp เมื่อเขียนเสร็จ
OBJS = $(SRCS:.cpp=.o)

engine: $(OBJS)
	$(CXX) $(CXXFLAGS) -flto -o $@ $(OBJS)

debug: CXXFLAGS = -g -O0 -std=c++17 -Wall -Wextra -fsanitize=address,undefined
debug: clean engine

clean:
	rm -f $(OBJS) engine