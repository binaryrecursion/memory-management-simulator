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
int PHYSICAL_MEM_SIZE=0;
int PAGE_SIZE=0;
int NUM_FRAMES=0;

enum AllocatorMode {
    NONE,
    LINEAR,
    BUDDY
};

AllocatorMode alloc_mode = NONE;
bool compare_mode = false;

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
    cout << "Memory Initialization\n";
    cout << "    init <bytes> <page_size>                    Initialize physical memory for allocation and\n";
    cout << "                                                page size for virtual memory simulations.\n\n";
    cout << "Dynamic Allocation - choose b/w [ff/bf/wf] or [buddy] per init\n";
    cout << "    alloc ff <bytes>                            First Fit allocation\n";
    cout << "    alloc bf <bytes>                            Best Fit allocation\n";
    cout << "    alloc wf <bytes>                            Worst Fit allocation\n";
    cout << "    alloc buddy <bytes>                         Buddy system allocation\n";
    cout << "    free <block_id>                             Free allocated block\n\n";

    cout << "Virtual Memory (Paging)\n";
    cout << "    vm_init <pid> <vsize>                       Initialize paging for a process\n";
    cout << "    access <pid> <vaddr>                        Translate virtual address and access memory\n";
    cout << "    vm_table <pid>                              Dump page table for process\n\n";

    cout << "Inspection & Statistics\n";
    cout << "    dump                                        Dump heap memory layout\n";
    cout << "    stats                                       Show memory, VM, and cache stats\n";
    cout << "    compare                                     Compare allocation strategies\n\n";

    cout << "Utility\n";
    cout << "    clear_workload                              Clear recorded workload\n";
    cout << "    help                                        Show this help\n";
    cout << "    exit                                        Quit simulator\n\n";
    cout << "\nNote: Heap allocation and virtual memory are simulated independently.\n";
    cout << "VM accesses do not validate heap allocation state.\n";

    cout << "---------------------------------------------------------------------\n";
}

int main() {
    string cmd;

    cout << "- Memory Management Simulator - \n\n";
    cout << "Type 'help' to see commands\n";

    while (true) {
        cout << ">> ";
        //cin >> cmd;
if (!(cin >> cmd))  {
        break;
}
 
        if (compare_mode && cmd != "init" && cmd != "help" && cmd != "exit") {
            cout << "Error: Simulator must be reinitialized after compare.\n";
            continue;
        }

        if (cmd == "init") {
            int size, page;
            cin >> size >> page;

            if (size % page != 0) {
                cout << "Error: Physical memory must be divisible by page size.\n";
                continue;
            }

            PHYSICAL_MEM_SIZE = size;
            PAGE_SIZE = page;
            NUM_FRAMES = size / page;
reset_allocation_stats();
            init_memory(size);
            reset_vm_system(size, page);   // clears page tables, frame bitmap
             buddy_ids.clear();
            workload.clear();
            compare_mode = false;
            delete buddy;
            delete L1;
            delete L2;

            buddy = new BuddyAllocator(size, 128);
            L1 = nullptr;
            L2 = nullptr;

            cout << "System memory and page size initialize\n";
            cout << "Configure - cache :\n";
            int c1, b1, a1, c2, b2, a2;
            cout << "Enter L1 cache size, L1_block size, L1 associativity: [ <L1_size> <L1_block> <L1_assoc> ]\n>>";
            cin >> c1 >> b1 >> a1;
            cout << "Enter L2 cache size, L2_block size, L2 associativity: [ <L2_size> <L2_block> <L2_assoc> ]\n>>";
            cin >> c2 >> b2 >> a2;

            L1 = new Cache(c1, b1, a1);
            L2 = new Cache(c2, b2, a2);

            alloc_mode = NONE;
            total_cycles = 0;
            buddy_ids.clear();
            workload.clear();
            compare_mode = false;
            cout << "System initialized\n";
            cout << "Physical Memory : " << size << " bytes\n";
            cout << "Page Size (for Virtual Memory simulations)  : " << page << " bytes\n";
            cout << "Total Frames (for Virtual Memory simulations)  : " << NUM_FRAMES << "\n";
            cout << "L1 Size: "<< c1 << "B | Block Size:"<<b1<< "B | Assoc: " << a1 << "-way\n";
            cout << "L2 Size: "<< c2 << "B | Block Size:"<<b2<< "B | Assoc: " << a2 << "-way\n";
        }

        else if (cmd == "vm_init") {
            if (PAGE_SIZE == 0 || PHYSICAL_MEM_SIZE==0) {
                cout << "Error: Initialize system first using init.\n";
                continue;
            }
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

        else if (cmd == "dump"){
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

                int internal = internal_fragmentation();   // currently 0
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

                cout << "External Fragmentation: "
                    << "0 bytes (0%)\n";

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
                cout <<"\n";
            compare_strategies();
            compare_mode = true;
            cout << "\nSimulation state invalidated.\n";
            cout << "Please reinitialize memory using 'init <size>'.\n";
            cout << "Note: You may clear the workload using 'workload_clear', to analyze new patterns if required.\n";
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