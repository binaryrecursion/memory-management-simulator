#include <iostream>
#include <string>

#include "../include/memory.h"
#include "../include/buddy.h"
#include "../include/cache.h"

using namespace std;

BuddyAllocator* buddy = nullptr;
Cache* L1 = nullptr;
Cache* L2 = nullptr;

bool buddy_mode = false;


void cache_access(int addr) {
    if (addr < 0) return;

    if (!L1->access(addr)) {
        L2->access(addr);
    }
}


int main() {
    string cmd;

    cout << "Memory Management Simulator\n";
    cout << "Type 'exit' to quit\n";

    while (true) {
        cout << "> ";
        cin >> cmd;


        if (cmd == "init") {
            int size;
            cin >> size;

            init_memory(size);

            delete buddy;
            delete L1;
            delete L2;

            buddy = new BuddyAllocator(size, 128);


            L1 = new Cache(256, 16, 2);   
            L2 = new Cache(1024, 16, 4); 

            buddy_mode = false;

            cout << "Memory initialized\n";
        }


        else if (cmd == "alloc") {
            string type;
            int size;
            cin >> type >> size;

            int addr = -1;

            if (type == "ff") {
                buddy_mode = false;
                addr = malloc_first_fit(size);
            }
            else if (type == "bf") {
                buddy_mode = false;
                addr = malloc_best_fit(size);
            }
            else if (type == "wf") {
                buddy_mode = false;
                addr = malloc_worst_fit(size);
            }
            else if (type == "buddy") {
                if (!buddy) {
                    cout << "Initialize memory first\n";
                    continue;
                }
                buddy_mode = true;
                addr = buddy->buddy_malloc(size);
            }
            else {
                cout << "Unknown allocator\n";
                continue;
            }

            if (addr == -1) {
                workload.push_back({ALLOC_EVENT, size});
                cout << "Allocation failed\n";
            } else {
                workload.push_back({ALLOC_EVENT, size});
                cout << "Allocated at address " << addr << "\n";
                cache_access(addr);
            }
        }


        else if (cmd == "free") {
            int addr;
            cin >> addr;

            if (buddy_mode) {
                buddy->buddy_free(addr);
            } else {
                free_block(addr);
            }

            workload.push_back({FREE_EVENT, addr});
            cache_access(addr);

            cout << "Block freed\n";
        }


        else if (cmd == "access") {
            int addr;
            cin >> addr;

            cache_access(addr);
            cout << "Accessed address " << addr << "\n";
        }

        else if (cmd == "dump") {
            dump_memory();
        }

        else if (cmd == "dump_buddy") {
            buddy->dump_free_lists();
        }

        else if (cmd == "stats") {
            cout << "Internal Fragmentation: "
                 << internal_fragmentation() << "\n";

            cout << "External Fragmentation: "
                 << external_fragmentation() << "\n";

            cout << "Memory Utilization: "
                 << memory_utilization() << "%\n";

            allocation_stats();
        }

        else if (cmd == "cache_stats") {
            L1->print_stats("L1");
            L2->print_stats("L2");
        }

        else if (cmd == "compare") {
            compare_strategies();
        }

        else if (cmd == "clear_workload") {
            workload.clear();
            cout << "Workload cleared\n";
        }

        else if (cmd == "exit") {
            break;
        }

        else {
            cout << "Unknown command\n";
        }
    }

    return 0;
}