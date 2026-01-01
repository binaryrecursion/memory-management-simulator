#include <list>
#include <iostream>
#include "../include/memory.h"
#include "../include/buddy.h"

using namespace std;
int total_alloc_requests = 0;
int successful_allocs = 0;
int failed_allocs = 0;
int total_memory_size = 0;


list<Block> memory_blocks;
vector<Event> workload;

void init_memory(int total_size) {
    memory_blocks.clear();
 total_memory_size = total_size;
    Block initial;
    initial.start = 0;
    initial.size = total_size;
    initial.free = true;

    memory_blocks.push_back(initial);
}

void dump_memory() {
    cout << "----- Memory Dump -----\n";
    for (auto &b : memory_blocks) {
        int end = b.start + b.size - 1;
        cout << "[" << b.start << " - " << end << "] ";
        cout << (b.free ? "FREE\n" : "ALLOCATED\n");
    }
    cout << "-----------------------\n";
}

int malloc_first_fit(int size) {
    total_alloc_requests++;

    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {

        if (it->free && it->size >= size) {

            int alloc_start = it->start;


            if (it->size == size) {
                it->free = false;
            }

            else {
                Block allocated;
                allocated.start = it->start;
                allocated.size = size;
                allocated.free = false;

                Block remaining;
                remaining.start = it->start + size;
                remaining.size = it->size - size;
                remaining.free = true;

                it = memory_blocks.erase(it);


                it = memory_blocks.insert(it, allocated);
                ++it;
                memory_blocks.insert(it, remaining);
            }
            successful_allocs++;
            return alloc_start;
        }
    }
failed_allocs++;
    return -1; 
}

void free_block(int start_address) {
    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {

        if (it->start == start_address && !it->free) {

            it->free = true;


            if (it != memory_blocks.begin()) {
                auto prev = it;
                --prev;

                if (prev->free) {
                    prev->size += it->size;
                    it = memory_blocks.erase(it);
                    it = prev;
                }
            }


            auto next = it;
            ++next;

            if (next != memory_blocks.end() && next->free) {
                it->size += next->size;
                memory_blocks.erase(next);
            }

            return;
        }
    }
}
int malloc_best_fit(int size) {
    total_alloc_requests++;

    auto best = memory_blocks.end();

    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {
        if (it->free && it->size >= size) {
            if (best == memory_blocks.end() || it->size < best->size) {
                best = it;
            }
        }
    }

    if (best == memory_blocks.end()){
    failed_allocs++;
        return -1;
    }

    int alloc_start = best->start;

    if (best->size == size) {
        best->free = false;
    } else {
        Block allocated{best->start, size, false};
        Block remaining{best->start + size, best->size - size, true};

        best = memory_blocks.erase(best);
        best = memory_blocks.insert(best, allocated);
        ++best;
        memory_blocks.insert(best, remaining);
    }
   successful_allocs++;
    return alloc_start;
}
int malloc_worst_fit(int size) {
    total_alloc_requests++;

    auto worst = memory_blocks.end();

    for (auto it = memory_blocks.begin(); it != memory_blocks.end(); ++it) {
        if (it->free && it->size >= size) {
            if (worst == memory_blocks.end() || it->size > worst->size) {
                worst = it;
            }
        }
    }

    if (worst == memory_blocks.end()){
        failed_allocs++;
        return -1;
    }

    int alloc_start = worst->start;

    if (worst->size == size) {
        worst->free = false;
    } else {
        Block allocated{worst->start, size, false};
        Block remaining{worst->start + size, worst->size - size, true};

        worst = memory_blocks.erase(worst);
        worst = memory_blocks.insert(worst, allocated);
        ++worst;
        memory_blocks.insert(worst, remaining);
    }
successful_allocs++;
    return alloc_start;
}

int internal_fragmentation() {
    int internal = 0;

    for (auto &b : memory_blocks) {
        if (!b.free) {
            internal += 0;
        }
    }

    return internal;
}
int external_fragmentation() {
    int total_free = 0;
    int max_free = 0;

    for (auto &b : memory_blocks) {
        if (b.free) {
            total_free += b.size;
            if (b.size > max_free)
                max_free = b.size;
        }
    }

    return total_free - max_free;
}
double memory_utilization() {
    int used = 0;
    int total = 0;

    for (auto &b : memory_blocks) {
        total += b.size;
        if (!b.free)
            used += b.size;
    }

    if (total == 0) return 0.0;

    return (used * 100.0) / total;
}
void allocation_stats() {
    cout << "Allocation Requests: " << total_alloc_requests << "\n";
    cout << "Successful Allocations: " << successful_allocs << "\n";
    cout << "Failed Allocations: " << failed_allocs << "\n";

    if (total_alloc_requests > 0) {
        double rate = (successful_allocs * 100.0) / total_alloc_requests;
        cout << "Allocation Success Rate: " << rate << "%\n";
    } else {
        cout << "Allocation Success Rate: 0%\n";
    }
}
static void reset_simulation(int mem_size) {
    init_memory(mem_size);
    total_alloc_requests = 0;
    successful_allocs = 0;
    failed_allocs = 0;
}


struct Result {
    int ext_frag;
    double util;
    double success;
};

static Result replay(const string &type) {
   reset_simulation(total_memory_size);
    for (auto &e : workload) {
        if (e.type == ALLOC_EVENT) {
            if (type == "ff") malloc_first_fit(e.value);
            else if (type == "bf") malloc_best_fit(e.value);
            else if (type == "wf") malloc_worst_fit(e.value);
        } else {
            free_block(e.value);
        }
    }

    Result r;
    r.ext_frag = external_fragmentation();
    r.util = memory_utilization();
    r.success = total_alloc_requests
        ? (successful_allocs * 100.0 / total_alloc_requests)
        : 0.0;

    return r;
}

static Result replay_buddy() {
    BuddyAllocator buddy_test(total_memory_size, 128);

    int total_allocs = 0;
    int successful = 0;

    for (auto &e : workload) {
        if (e.type == ALLOC_EVENT) {
            total_allocs++;
            int addr = buddy_test.buddy_malloc(e.value);
            if (addr != -1)
                successful++;
        } else {
            buddy_test.buddy_free(e.value);
        }
    }

    Result r;
    r.ext_frag = 0; 
    r.util = (buddy_test.get_used_memory() * 100.0) / total_memory_size;
    r.success = total_allocs
        ? (successful * 100.0 / total_allocs)
        : 0.0;

    return r;
}

void compare_strategies() {
    if (workload.empty()) {
        cout << "No workload recorded.\n";
        return;
    }

    Result ff = replay("ff");    
    Result bf = replay("bf");
    Result wf = replay("wf");
    Result buddy = replay_buddy();


    cout << "\nStrategy   ExtFrag   Utilization   SuccessRate\n";
    cout << "FF         " << ff.ext_frag << "        "
         << ff.util << "%        " << ff.success << "%\n";
    cout << "BF         " << bf.ext_frag << "        "
         << bf.util << "%        " << bf.success << "%\n";
    cout << "WF         " << wf.ext_frag << "        "
         << wf.util << "%        " << wf.success << "%\n";
         cout << "Buddy      " << buddy.ext_frag << "        "
     << buddy.util << "%        " << buddy.success << "%\n";

}