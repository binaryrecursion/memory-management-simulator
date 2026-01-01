#ifndef BUDDY_H
#define BUDDY_H

#include <vector>
#include <unordered_map>
#include <list>

using namespace std;

class BuddyAllocator {
private:
    int total_size;
    int min_order;
    int max_order;

    int used_memory = 0; 

    vector<list<int>> free_list;             
    unordered_map<int, int> allocated_order;   

    int size_to_order(int size);
    bool is_power_of_two(int x);

public:
    BuddyAllocator(int memory_size, int min_block_size);

    int buddy_malloc(int size);
    void buddy_free(int addr);

    int get_used_memory() const;  

    void dump_free_lists();
};

#endif