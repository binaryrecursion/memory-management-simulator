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


enum AllocatorMode {
    NONE,
    LINEAR,
    BUDDY
};

AllocatorMode alloc_mode = NONE;


void cache_access(int addr) {
    if (addr < 0) return;

    if (L1->access(addr)) {
        total_cycles += 1;   
        cout << "L1 hit!\n";
        return;
    }

    total_cycles += 1;

    if (L2->access(addr)) {
        total_cycles += 5; 
        cout << "L1 miss. L2 hit!";
        return;
    }

    total_cycles += 5 + 50;
    cout << "L1 miss. L2 miss. Accessing from main memory....\n";
    return;
}


void print_help() {
    cout << "\nCOMMANDS\n";
    cout << "----------------------------------------------------------------------\n";
    cout << "Memory Initialization\n";
    cout << "    init <bytes>                               Initialize physical memory for allocation\n\n";
    cout << "Dynamic Allocation - choose b/w [ff/bf/wf] or [buddy] per init\n";
    cout << "    alloc ff <bytes>                            First Fit allocation\n";
    cout << "    alloc bf <bytes>                            Best Fit allocation\n";
    cout << "    alloc wf <bytes>                            Worst Fit allocation\n";
    cout << "    alloc buddy <bytes>                         Buddy system allocation\n";
    cout << "    free <block_id>                             Free allocated block\n\n";

    cout << "Virtual Memory (Paging)\n";
    cout << "    vm_init <pid> <vsize> <psize> <page>        Initialize paging for a process\n";
    cout << "    vm_access <pid> <vaddr>                     Translate virtual address and access memory\n";
    cout << "    vm_table <pid>                              Dump page table for process\n\n";

    cout << "Inspection & Statistics\n";
    cout << "    dump                                        Dump heap memory layout\n";
    cout << "    dump_buddy                                  Dump buddy free lists\n";
    cout << "    stats                                       Show memory, VM, and cache stats\n";
    cout << "    compare                                     Compare allocation strategies\n\n";

    cout << "Utility\n";
    cout << "    clear_workload                              Clear recorded workload\n";
    cout << "    help                                         Show this help\n";
    cout << "    exit                                          Quit simulator\n";
    cout << "---------------------------------------------------------------------\n";
}


int main() {
    string cmd;

    cout << "- Memory Management Simulator - \n\n";
    cout << "Type 'help' to see commands\n";

    while (true) {
        cout << ">> ";
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

            alloc_mode = NONE;
            total_cycles = 0;
            buddy_ids.clear();

            cout << " Memory Re-initialized (" << size << " bytes)\n";
            cout << " Allocator mode reset\n";
        }


        else if (cmd == "vm_init") {
            int pid, vsize, psize, page;
            cin >> pid >> vsize >> psize >> page;

            init_vm(pid, vsize, psize, page);
        }


        else if (cmd == "vm_access") {
            int pid, vaddr;
            cin >> pid >> vaddr;

            int paddr = vm_access(pid, vaddr);
            cout << "Physical address = " << paddr << "\n";

            if (paddr != -1)
                cache_access(paddr);
        }


        else if (cmd == "vm_table") {
            int pid;
            cin >> pid;
            dump_page_table(pid);
        }


        else if (cmd == "alloc") {
            string type;
            int size;
            cin >> type >> size;

            int addr = -1;

            if (alloc_mode == NONE) {
                alloc_mode = (type == "buddy") ? BUDDY : LINEAR;
                cout << "Allocator mode set to "
                     << (alloc_mode == BUDDY ? "BUDDY\n" : "LINEAR\n");
            }

            if (alloc_mode == LINEAR && type == "buddy") {
                cout << "Cannot use buddy allocator.\n";
                cout << "Memory is in LINEAR mode. Reinitialize to switch.\n";
                continue;
            }

            if (alloc_mode == BUDDY && type != "buddy") {
                cout << "Cannot use linear allocator.\n";
                cout << "Memory is in BUDDY mode. Reinitialize to switch.\n";
                continue;
            }

            if (alloc_mode == LINEAR) {
                if (type == "ff") addr = malloc_first_fit(size);
                else if (type == "bf") addr = malloc_best_fit(size);
                else if (type == "wf") addr = malloc_worst_fit(size);
                else {
                    cout << "Unknown allocator\n";
                    continue;
                }
            } else {
                addr = buddy->buddy_malloc(size);
            }

            workload.push_back({ALLOC_EVENT, size});

            if (addr == -1) {
                cout << "Allocation failed\n";
            } else {
                int id = get_block_id(addr);
                if (alloc_mode == BUDDY)
                    id = buddy_ids[addr];

                cout << "Allocated block id=" << id
                     << " at address=0x"
                     << hex << addr << dec << "\n";

                cache_access(addr);
            }
        }


        else if (cmd == "free") {
            int id;
            cin >> id;

            int addr = -1;

            addr = get_block_start_by_id(id);

            if (addr == -1) {
                for (auto &p : buddy_ids) {
                    if (p.second == id) {
                        addr = p.first;
                        break;
                    }
                }
            }

            if (addr == -1) {
                cout << "No block with id=" << id << "\n";
                continue;
            }

            if (buddy_ids.count(addr))
                buddy->buddy_free(addr);
            else
                free_block(addr);

            workload.push_back({FREE_EVENT, addr});
            cout << "Block " << id << " freed\n";
        }


        else if (cmd == "dump") {
            dump_memory();
        }

        else if (cmd == "dump_buddy") {
            buddy->dump_free_lists();
        }

       else if (cmd == "stats") {
            cout << "----- Memory -----\n";

            if (alloc_mode == LINEAR) {
                cout << "Allocator Type: Linear (FF/BF/WF)\n";
                cout << "Internal Fragmentation: " << internal_fragmentation() << "\n";
                cout << "External Fragmentation: " << external_fragmentation() << "\n";
                cout << "Memory Utilization: " << memory_utilization() << "%\n";
                allocation_stats();
            }
            else if (alloc_mode == BUDDY) {
                cout << "Allocator Type: Buddy System\n";

                int used = buddy->get_used_memory();
                int total = total_memory_size;
                int internal = buddy->get_internal_fragmentation();

                cout << "Total Memory: " << total << " bytes\n";
                cout << "Used Memory: " << used << " bytes\n";
                cout << "Free Memory: " << (total - used) << " bytes\n";

                cout << "Internal Fragmentation: "
                    << internal << " bytes\n";

                cout << "External Fragmentation: 0 bytes\n";

                cout << "Memory Utilization: "
                    << (total ? (used * 100.0 / total) : 0.0)
                    << "%\n";

                cout << "\nPer‑Allocation Fragmentation:\n";
                buddy->dump_allocations();
            }

            else {
                cout << "No allocator active\n";
            }

            
            cout << "\n----- Virtual Memory -----\n";

            int hits = get_page_hits();
            int faults = get_page_faults();
            int total = hits + faults;

            cout << "Total Page Hits: " << hits << "\n";
            cout << "Total Page Faults: " << faults << "\n";

            if (total > 0)
                cout << "Fault Rate: " << (faults * 100.0 / total) << "%\n";
            else
                cout << "Fault Rate: 0%\n";


            cout << "\nPer‑Process Frame Usage:\n";

            for (int pid = 1; pid <= 10; pid++) {  
                int used = get_used_frames(pid);
                if (used > 0) {
                    cout << "PID " << pid << ": "
                        << used << "/" << get_total_frames()
                        << " frames used\n";
                }
            }

            cout << "\n----- Cache -----\n";
            L1->print_stats("L1");
            L2->print_stats("L2");
            cout << "Total Memory Access Cycles: " << total_cycles << "\n";
            cout << "Disk Penalty per fault: " << disk_penalty << "\n";
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
        else if (cmd == "help") print_help();
        else {
            cout << "Unknown command\n";
        }
    }

    return 0;
}