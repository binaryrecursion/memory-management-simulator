#include <iostream>
#include <list>
#include "../include/memory.h"

using namespace std;

list<Block> memory_blocks;

void init_memory(int total_size) {
    memory_blocks.clear();
    Block initial{0, total_size, true};
    memory_blocks.push_back(initial);
}

void dump_memory() {
    cout << "----- Memory Dump -----\n";
    for (auto &b : memory_blocks) {
        cout << "[" << b.start << " - "
             << b.start + b.size - 1 << "] "
             << (b.free ? "FREE\n" : "ALLOCATED\n");
    }
}
