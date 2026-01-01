#ifndef MEMORY_H
#define MEMORY_H

#include <list>

#include <vector>
#include <string>
#include <iostream>

using namespace std;
struct Block {
    int start;
    int size;
    bool free;
};

void init_memory(int total_size);
void dump_memory();

#endif
