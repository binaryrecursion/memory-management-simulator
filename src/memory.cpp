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
int malloc_first_fit(int size) {
    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {
        if (it->free && it->size >= size) {
            int addr = it->start;
            if (it->size == size) {
                it->free = false;
            } else {
                Block alloc{it->start, size, false};
                Block rem{it->start + size, it->size - size, true};
                it = memory_blocks.erase(it);
                it = memory_blocks.insert(it, alloc);
                memory_blocks.insert(++it, rem);
            }
            return addr;
        }
    }
    return -1;
}

