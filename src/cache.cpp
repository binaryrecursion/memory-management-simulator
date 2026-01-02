#include "../include/cache.h"
#include <iostream>

using namespace std;
int l1_penalty   = 1;
int l2_penalty   = 5;
int memory_penalty = 50;

int total_cycles = 0;

Cache::Cache(int C, int b, int N)
    : cache_size(C), block_size(b), associativity(N) {

    int blocks = cache_size / block_size;
    num_sets = blocks / associativity;

    sets.resize(num_sets, vector<CacheLine>(associativity, {false, -1}));
    fifo.resize(num_sets);
}

bool Cache::access(int address) {
    accesses++;

    int block_number = address / block_size;
    int set_index = block_number % num_sets;
    int tag = block_number / num_sets;

    for (auto &line : sets[set_index]) {
        if (line.valid && line.tag == tag) {
            hits++;
            return true; 
        }
    }

    misses++;
    insert(address);
    return false; 
}

void Cache::insert(int address) {
    int block_number = address / block_size;
    int set_index = block_number % num_sets;
    int tag = block_number / num_sets;


    for (int i = 0; i < associativity; i++) {
        if (!sets[set_index][i].valid) {
            sets[set_index][i] = {true, tag};
            fifo[set_index].push(i);
            return;
        }
    }

    int victim = fifo[set_index].front();
    fifo[set_index].pop();

    sets[set_index][victim] = {true, tag};
    fifo[set_index].push(victim);
}

void Cache::print_stats(const string &name) const {
    cout << name << " Accesses: " << accesses << "\n";
    cout << name << " Hits: " << hits << "\n";
    cout << name << " Misses: " << misses << "\n";

    if (accesses > 0)
        cout << name << " Hit Ratio: "
             << (hits * 100.0 / accesses) << "%\n";
}