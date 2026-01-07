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

int PHYSICAL_MEM_SIZE = 0;
int PAGE_SIZE = 0;
int NUM_FRAMES = 0;

enum AllocatorMode { NONE, LINEAR, BUDDY };
enum LinearStrategy { LNONE, FIRST, BEST, WORST };

AllocatorMode alloc_mode = NONE;
LinearStrategy lin_strategy = LNONE;

bool compare_mode = false;
bool system_initialized = false;

void cache_access(int addr) {
    if (addr < 0) return;

    if (L1->access(addr)) {
        total_cycles += l1_penalty;
        cout << "L1 hit!\n";
        return;
    }

    total_cycles += l1_penalty;

    if (L2->access(addr)) {
        total_cycles += l2_penalty;
        cout << "L1 miss. L2 hit.\n";
        return;
    }

    total_cycles += l2_penalty + memory_penalty;
    cout << "L1 miss. L2 miss. Accessing main memory.\n";
}

void print_help() {
    cout << "\nCOMMANDS\n";
    cout << "----------------------------------------------------------------------\n";
    cout << "init                                        Initialize system\n";
    cout << "alloc                                       Configure allocator\n";
    cout << "malloc <size>                               Allocate memory\n";
    cout << "free <block_id>                             Free block\n";
    cout << "vm_init <pid> <vsize>                       Init paging\n";
    cout << "access <pid> <vaddr>                        Access virtual address\n";
    cout << "vm_table <pid>                              Print page table\n";
    cout << "dump                                        Dump heap\n";
    cout << "stats                                       Show statistics\n";
    cout << "compare                                     Compare strategies\n";
    cout << "clear_workload                              Clear workload\n";
    cout << "help                                        Show help\n";
    cout << "exit                                        Quit\n";
}

int main() {
    string cmd;

    cout << "- Memory Management Simulator - \n\n";
    cout << "Type 'help' to see commands\n";

    while (true) {
        cout << ">> ";
        if (!(cin >> cmd)) break;

    
        if (!system_initialized &&
            cmd != "init" && cmd != "help" && cmd != "exit") {
            cout << "System not initialized. Run: init <bytes> <page_size>\n";
            continue;
        }

        if (compare_mode &&
            cmd != "init" && cmd != "help" && cmd != "exit") {
            cout << "Error: Simulator must be reinitialized after compare.\n";
            continue;
        }

        if (cmd == "init") {

            int size, page;

            cout << "Enter physical memory size: ";
            cin >> size;

            cout << "Enter page size: ";
            cin >> page;

            if (size % page != 0) {
                cout << "Error: Physical memory must be divisible by page size.\n";
                continue;
            }

            system_initialized = true;

            PHYSICAL_MEM_SIZE = size;
            PAGE_SIZE = page;
            NUM_FRAMES = size / page;

            reset_allocation_stats();
            init_memory(size);
            reset_vm_system(size, page);
            buddy_ids.clear();
            workload.clear();

            delete buddy;
            delete L1;
            delete L2;

            buddy = new BuddyAllocator(size, 128);
            L1 = nullptr;
            L2 = nullptr;

            cout << "System memory and page size initialize\n";
            cout << "Configure - cache :\n";

            int c1, b1, a1, c2, b2, a2;

            cout << "Enter L1 cache size, L1_block size, L1 associativity:\n>>";
            cin >> c1 >> b1 >> a1;

            cout << "Enter L2 cache size, L2_block size, L2 associativity:\n>>";
            cin >> c2 >> b2 >> a2;

            L1 = new Cache(c1, b1, a1);
            L2 = new Cache(c2, b2, a2);

            alloc_mode = NONE;
            lin_strategy = LNONE;
            total_cycles = 0;
            compare_mode = false;

            cout << "System initialized\n";
            cout << "Physical Memory : " << size << " bytes\n";
            cout << "Page Size       : " << page << " bytes\n";
            cout << "Total Frames    : " << NUM_FRAMES << "\n";
        }

        else if (cmd == "vm_init") {
            int pid, vsize;
            cin >> pid >> vsize;
            init_vm(pid, vsize);
        }

        
        else if (cmd == "access") {
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

            if (alloc_mode == NONE) {
                cout << "Choose allocator mode (buddy / linear): ";
                string t; cin >> t;

                if (t == "buddy") {
                    alloc_mode = BUDDY;
                    cout << "Buddy allocator activated.\n";
                }
                else if (t == "linear") {
                    alloc_mode = LINEAR;
                    cout << "Linear allocator activated.\n";
                }
                else {
                    cout << "Invalid allocator type.\n";
                    continue;
                }
            }

            if (alloc_mode == BUDDY) {
                cout << "Buddy allocator configured. Use: malloc <size>\n";
            }

            else if (alloc_mode == LINEAR) {

                if (lin_strategy != LNONE) {
                    cout << "Linear allocator already configured. Use: malloc <size>\n";
                }
                else {
                    string strat;
                    cout << "Choose linear strategy (first_fit / best_fit / worst_fit): ";
                    cin >> strat;

                    if (strat == "first_fit")      lin_strategy = FIRST;
                    else if (strat == "best_fit")  lin_strategy = BEST;
                    else if (strat == "worst_fit") lin_strategy = WORST;
                    else {
                        cout << "Invalid linear strategy.\n";
                        continue;
                    }

                    cout << "Strategy selected. Use: malloc <size>\n";
                }
            }
        }

        else if (cmd == "malloc") {

            int size;
            cin >> size;

            if (alloc_mode == NONE) {
                cout << "Allocator not configured. Use 'alloc' first.\n";
                continue;
            }

            int addr = -1;

            if (alloc_mode == BUDDY) {
                addr = buddy->buddy_malloc(size);
            }
            else {

                if (lin_strategy == FIRST)
                    addr = malloc_first_fit(size);
                else if (lin_strategy == BEST)
                    addr = malloc_best_fit(size);
                else if (lin_strategy == WORST)
                    addr = malloc_worst_fit(size);
                else {
                    cout << "No strategy selected. Run 'alloc' first.\n";
                    continue;
                }
            }

            workload.push_back({ALLOC_EVENT, size});

            if (addr == -1)
                cout << "Allocation failed\n";
            else {
                int id = (alloc_mode == BUDDY)
                        ? buddy_ids[addr]
                        : get_block_id(addr);

                cout << "Allocated block id=" << id
                     << " at address=0x" << hex << addr << dec << "\n";
            }
        }

        else if (cmd == "free") {
            int id;
            cin >> id;

            int addr = get_block_start_by_id(id);

            if (addr == -1) {
                for (auto &p : buddy_ids)
                    if (p.second == id) addr = p.first;
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
    if (alloc_mode == BUDDY && buddy) {
        cout << " Buddy allocator in use \n";
        buddy->dump_allocations();
        buddy->dump_free_lists();
    }
    else {
        cout << "Linear allocator in use \n";
        dump_memory();
    }
}

        else if (cmd == "stats") {
            cout << "=======STATISTICS=======\n";

            cout << "\n----- Memory -----\n";

            if (alloc_mode == LINEAR) {
                cout << "Allocator Type: Linear (FF/BF/WF)\n";

                int internal = internal_fragmentation();
                int external = external_fragmentation();

                int total = total_memory_size;
                int free_mem = 0;

                for (auto &b : memory_blocks)
                    if (b.free) free_mem += b.size;

                double internal_pct = total ? (internal * 100.0 / total) : 0.0;
                double external_pct = free_mem ? (external * 100.0 / free_mem) : 0.0;

                cout << "Internal Fragmentation: "
                     << internal << " bytes ("
                     << internal_pct << "% of total memory)\n";

                cout << "External Fragmentation: "
                     << external << " bytes ("
                     << external_pct << "% of free memory)\n";

                cout << "Memory Utilization: "
                     << memory_utilization() << "%\n";

                allocation_stats();
            }

            else if (alloc_mode == BUDDY) {
                cout << "Allocator Type: Buddy System\n";

                int used = buddy->get_used_memory();
                int total = total_memory_size;
                int internal = buddy->get_internal_fragmentation();
                int free_mem = total - used;

                double internal_pct = used
                    ? (internal * 100.0 / used)
                    : 0.0;

                cout << "Total Memory: " << total << " bytes\n";
                cout << "Used Memory: " << used << " bytes\n";
                cout << "Free Memory: " << free_mem << " bytes\n";

                cout << "Internal Fragmentation: "
                    << internal << " bytes ("
                    << internal_pct << "% of allocated memory)\n";

                cout << "External Fragmentation: 0 bytes (0%)\n";

                cout << "Memory Utilization: "
                    << (total ? (used * 100.0 / total) : 0.0)
                    << "%\n";

                cout << "\nPer Allocation Fragmentation:\n";
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

            cout << "\nPer-Process Frame Usage:\n";

            if (!any_vm_initialized()) {
                cout << "No virtual memory initialized for any process.\n";
            }
            else {
                for (int pid : get_initialized_pids()) {
                    int used = get_used_frames(pid);
                    if (used == 0)
                        cout << "PID " << pid << ": NIL\n";
                    else
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
            compare_mode = true;
        }

        else if (cmd == "clear_workload") {
            workload.clear();
            cout << "Workload cleared\n";
        }

        else if (cmd == "help") print_help();
        else if (cmd == "exit") break;
        else cout << "Unknown command\n";
    }

    return 0;
}
