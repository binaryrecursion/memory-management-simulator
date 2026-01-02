#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <queue>
#include <string>
struct CacheLine {
    bool valid;
    int tag;
};
extern int l1_penalty;
extern int l2_penalty;
extern int memory_penalty;

extern int total_cycles;

class Cache {
private:
    int cache_size;    
    int block_size;    
    int associativity; 
    int num_sets;     

    std::vector<std::vector<CacheLine>> sets;
    std::vector<std::queue<int>> fifo;

    int accesses = 0;
    int hits = 0;
    int misses = 0;

public:
    Cache(int C, int b, int N);

    bool access(int address);   
    void insert(int address);

    void print_stats(const std::string &name) const;
};

#endif