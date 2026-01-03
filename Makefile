CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDE = -Iinclude

SRC = src/main.cpp src/memory.cpp src/buddy.cpp src/cache.cpp src/vm.cpp
OUT = memsim

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(INCLUDE) -o $(OUT)

clean:
	rm -f $(OUT)
