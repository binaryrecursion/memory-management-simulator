#ifndef MEMORY_H
#define MEMORY_H

#include <list>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
extern int total_memory_size;

struct Block {
    int start;
    int size;
    bool free;
};


extern list<Block> memory_blocks;


void init_memory(int total_size);
void dump_memory();


int malloc_first_fit(int size);
int malloc_best_fit(int size);
int malloc_worst_fit(int size);
void free_block(int start_address);


int internal_fragmentation();
int external_fragmentation();
double memory_utilization();


extern int total_alloc_requests;
extern int successful_allocs;
extern int failed_allocs;

void allocation_stats();

enum EventType {
    ALLOC_EVENT,
    FREE_EVENT
};

struct Event {
    EventType type;
    int value;   
};

extern vector<Event> workload;

void compare_strategies();

#endif