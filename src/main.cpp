#include <iostream>
#include <string>

#include "../include/memory.h"
#include "../include/buddy.h"
#include "../include/cache.h"
#include "../include/vm.h"

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

        else if (cmd == "vm_init") {
    int vsize, psize, page;
    cin >> vsize >> psize >> page;

    init_vm(vsize, psize, page);
}
else if (cmd == "vm_access") {
    int vaddr;
    cin >> vaddr;

    int paddr = vm_access(vaddr);
    cout << "Physical address = " << paddr << "\n";

    if (paddr != -1)
        cache_access(paddr);
}
else if (cmd == "vm_table") {
    dump_page_table();
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
                int id = get_block_id(addr);

cout << "Allocated block id=" << id
     << " at address=0x"
     << hex << addr << dec << "\n";

                cache_access(addr);
            }
        }


        else if (cmd == "free") {
    int id;
    cin >> id;

    int addr = get_block_start_by_id(id);

    if (addr == -1) {
        cout << "No block with id=" << id << "\n";
        continue;
    }

    if (buddy_mode)
        buddy->buddy_free(addr);
    else
        free_block(addr);

    workload.push_back({FREE_EVENT, addr});

    cout << "Block " << id << " freed \n";
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

    cout << "----- Memory -----\n";
    cout << "Internal Fragmentation: " << internal_fragmentation() << "\n";
    cout << "External Fragmentation: " << external_fragmentation() << "\n";
    cout << "Memory Utilization: " << memory_utilization() << "%\n";
    allocation_stats();

    cout << "\n----- Virtual Memory -----\n";
    int hits = get_page_hits();
    int faults = get_page_faults();
    int total = hits + faults;

    cout << "Page Hits: " << hits << "\n";
    cout << "Page Faults: " << faults << "\n";

    if (total > 0)
        cout << "Fault Rate: " << (faults * 100.0 / total) << "%\n";
    else
        cout << "Fault Rate: 0%\n";

    cout << "\n----- Cache -----\n";
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