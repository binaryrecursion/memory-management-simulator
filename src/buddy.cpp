#include "../include/buddy.h"
#include "../include/memory.h"   
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

bool BuddyAllocator::is_power_of_two(int x) {
    return x && !(x & (x - 1));
}

int BuddyAllocator::size_to_order(int size) {
    int order = 0;
    int val = 1;
    while (val < size) {
        val <<= 1;
        order++;
    }
    return order;
}

BuddyAllocator::BuddyAllocator(int memory_size, int min_block_size) {
    if (!is_power_of_two(memory_size) || !is_power_of_two(min_block_size)) {
        throw runtime_error("Sizes must be power of two");
    }

    total_size = memory_size;
    min_order = size_to_order(min_block_size);
    max_order = size_to_order(memory_size);

    free_list.resize(max_order + 1);
    free_list[max_order].push_back(0);
}

int BuddyAllocator::buddy_malloc(int size) {
    int req_order = size_to_order(size);
    if (req_order < min_order)
        req_order = min_order;

    int cur_order = req_order;
    while (cur_order <= max_order && free_list[cur_order].empty())
        cur_order++;

    if (cur_order > max_order)
        return -1;

    int addr = free_list[cur_order].front();
    free_list[cur_order].pop_front();

    while (cur_order > req_order) {
        cur_order--;
        int buddy = addr + (1 << cur_order);
        free_list[cur_order].push_back(buddy);
    }

    allocated_order[addr] = req_order;
    requested_size[addr] = size;
    used_memory += (1 << req_order);

    buddy_ids[addr] = next_block_id++;

    return addr;
}

void BuddyAllocator::buddy_free(int addr) {
    if (allocated_order.find(addr) == allocated_order.end())
        return;

    int order = allocated_order[addr];
    used_memory -= (1 << order);
    allocated_order.erase(addr);
    requested_size.erase(addr);
    buddy_ids.erase(addr);

    while (order < max_order) {
        int block_size = 1 << order;
        int buddy = addr ^ block_size;

        auto &lst = free_list[order];
        auto it = find(lst.begin(), lst.end(), buddy);

        if (it == lst.end())
            break;

        lst.erase(it);
        addr = min(addr, buddy);
        order++;
    }

    free_list[order].push_back(addr);
}

int BuddyAllocator::get_used_memory() const {
    return used_memory;
}

void BuddyAllocator::dump_free_lists() {
    cout << "---- Buddy Free Lists ----\n";
    for (int i = min_order; i <= max_order; i++) {
        cout << "Order " << i << " (size " << (1 << i) << "): ";
        for (int addr : free_list[i])
            cout << addr << " ";
        cout << "\n";
    }
}

int BuddyAllocator::get_internal_fragmentation() const {
    int total = 0;

    for (auto &p : allocated_order) {
        int addr = p.first;
        int order = p.second;

        int allocated = 1 << order;
        int requested = requested_size.at(addr);

        total += (allocated - requested);
    }

    return total;
}

void BuddyAllocator::dump_allocations() const {
    cout << "---- Buddy Allocations ----\n";
    cout << "Addr\tReq\tAlloc\tInternalFrag\n";

    for (auto &p : allocated_order) {
        int addr = p.first;
        int order = p.second;

        int allocated = 1 << order;
        int requested = requested_size.at(addr);

        cout << "0x" << hex << addr << dec << "\t"
             << requested << "\t"
             << allocated << "\t"
             << (allocated - requested) << "\n";
    }
}
