#include "../include/vm.h"
#include <iostream>
#include <unordered_map>

using namespace std;

static int PAGE_SIZE;
static int NUM_FRAMES;
static int PHYSICAL_MEM_SIZE;

struct ProcessVM {
    int num_pages;
    vector<PageTableEntry> table;
};

static unordered_map<int, ProcessVM> page_tables;

static vector<int> frame_owner;   

static int time_counter = 0;
static int page_hits = 0;
static int page_faults = 0;

int disk_penalty = 200;

void reset_vm_system(int physical_size, int page_size) {
    PHYSICAL_MEM_SIZE = physical_size;
    PAGE_SIZE = page_size;
    NUM_FRAMES = PHYSICAL_MEM_SIZE / PAGE_SIZE;

    frame_owner.assign(NUM_FRAMES, -1);
    page_tables.clear();

    page_hits = 0;
    page_faults = 0;

}


void init_vm(int pid, int virtual_size) {

    int num_pages = virtual_size / PAGE_SIZE;

    ProcessVM vm;
    vm.num_pages = num_pages;
    vm.table.assign(num_pages, {false, -1, 0});

    page_tables[pid] = vm;

    cout << "Virtual memory initialized for PID " << pid << " of size: " << virtual_size <<"B\n";
}




static int choose_victim_frame() {
    int oldest = 1e9;
    int victim = 0;

    for (int f = 0; f < NUM_FRAMES; f++) {

        if (frame_owner[f] == -1)
            return f;

        int owner_pid = frame_owner[f];
        auto &proc = page_tables[owner_pid];

        for (auto &pte : proc.table) {
            if (pte.valid && pte.frame == f && pte.last_used < oldest) {
                oldest = pte.last_used;
                victim = f;
            }
        }
    }
    return victim;
}


int vm_access(int pid, int vaddr) {

    time_counter++;

    int page = vaddr / PAGE_SIZE;
    int offset = vaddr % PAGE_SIZE;

    auto &proc = page_tables[pid];

    if (page < 0 || page >= proc.num_pages) {
        cout << "Invalid virtual address: " << vaddr << "\n";
        return -1;
    }

    PageTableEntry &pte = proc.table[page];


    if (pte.valid) {
        page_hits++;
        pte.last_used = time_counter;
        cout << "PAGE HIT (PID " << pid
         << ", page " << page
         << ", frame " << pte.frame << ")\n";
        return pte.frame * PAGE_SIZE + offset;
    }

    page_faults++;
    cout << "PAGE FAULT (PID " << pid << ", page " << page << ")\n";

    extern int total_cycles;
    total_cycles += disk_penalty;

    int frame = choose_victim_frame();

    if (frame_owner[frame] != -1) {
        int old_pid = frame_owner[frame];

        auto &old_proc = page_tables[old_pid];

        for (auto &old_pte : old_proc.table) {
            if (old_pte.valid && old_pte.frame == frame) {

                cout << "PAGE EVICTION: PID "
                    << old_pid << ", frame " << frame << "\n";

                old_pte.valid = false;
                old_pte.frame = -1;
                break;
            }
        }

    }


    frame_owner[frame] = pid;
    pte.valid = true;
    pte.frame = frame;
    pte.last_used = time_counter;
cout << "Mapped (PID " << pid
     << ", page " << page
     << ") -> frame " << frame << "\n";
    return frame * PAGE_SIZE + offset;
}



void dump_page_table(int pid) {
    auto &proc = page_tables[pid];

    cout << "PID " << pid << " Page Table\n";
    cout << "Page\tValid\tFrame\n";

    for (int i = 0; i < proc.num_pages; i++) {
        cout << i << "\t"
            << proc.table[i].valid << "\t"
            << proc.table[i].frame << "\n";
    }

}

bool any_vm_initialized() {
    return !page_tables.empty();
}

vector<int> get_initialized_pids() {
    vector<int> pids;
    for (auto &pt : page_tables) {
        pids.push_back(pt.first);
    }
    return pids;
}

int get_used_frames(int pid) {
    int used = 0;
    for (auto &pte : page_tables[pid].table)
        if (pte.valid) used++;
    return used;
}


int get_total_frames() {
    return NUM_FRAMES;
}

int get_page_hits() { return page_hits; }
int get_page_faults() { return page_faults; }