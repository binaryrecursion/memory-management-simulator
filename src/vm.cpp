#include "../include/vm.h"
#include <iostream>
#include <unordered_map>

using namespace std;

static int PAGE_SIZE;
static int NUM_PAGES;
static int NUM_FRAMES;

static unordered_map<int, vector<PageTableEntry>> page_tables;

static vector<int> frame_owner;   

static int time_counter = 0;
static int page_hits = 0;
static int page_faults = 0;

int disk_penalty = 200;


void init_vm(int pid, int virtual_size, int physical_size, int page_size) {

    PAGE_SIZE = page_size;
    NUM_PAGES = virtual_size / page_size;
    NUM_FRAMES = physical_size / page_size;

    page_tables[pid] = vector<PageTableEntry>(
        NUM_PAGES, {false, -1, 0}
    );

    if (frame_owner.empty())
        frame_owner.assign(NUM_FRAMES, -1);

    cout << "Virtual memory initialized for PID " << pid << "\n";
}



static int choose_victim_frame() {
    int oldest = 1e9;
    int victim = 0;

    for (int f = 0; f < NUM_FRAMES; f++) {
        if (frame_owner[f] == -1) return f;

        int owner = frame_owner[f];
        for (auto &pt : page_tables) {
            if (pt.first == owner) {
                for (auto &pte : pt.second) {
                    if (pte.valid && pte.frame == f &&
                        pte.last_used < oldest) {
                        oldest = pte.last_used;
                        victim = f;
                    }
                }
            }
        }
    }
    return victim;
}

int vm_access(int pid, int vaddr) {

    time_counter++;

    int page = vaddr / PAGE_SIZE;
    int offset = vaddr % PAGE_SIZE;

    auto &ptable = page_tables[pid];
    if (page < 0 || page >= NUM_PAGES) {
        cout << "Invalid virtual address: " << vaddr << "\n";
        return -1;
    }

    PageTableEntry &pte = ptable[page];   

    if (pte.valid) {
        page_hits++;
        pte.last_used = time_counter;
        return pte.frame * PAGE_SIZE + offset;
    }

    page_faults++;
    cout << "PAGE FAULT (PID " << pid << ", page " << page << ")\n";

    extern int total_cycles;
    total_cycles += disk_penalty;

    int frame = choose_victim_frame();

    if (frame_owner[frame] != -1) {
        int old_pid = frame_owner[frame];
        for (auto &old_pte : page_tables[old_pid]) {
            if (old_pte.valid && old_pte.frame == frame) {
                old_pte.valid = false;
                break;
            }
        }
    }

    frame_owner[frame] = pid;
    pte.valid = true;
    pte.frame = frame;
    pte.last_used = time_counter;

    return frame * PAGE_SIZE + offset;
}



void dump_page_table(int pid) {
    auto &ptable = page_tables[pid];

    cout << "PID " << pid << " Page Table\n";
    cout << "Page\tValid\tFrame\n";

    for (int i = 0; i < ptable.size(); i++) {
        cout << i << "\t"
             << ptable[i].valid << "\t"
             << ptable[i].frame << "\n";
    }
}

int get_used_frames(int pid) {
    int used = 0;
    for (auto &pte : page_tables[pid])
        if (pte.valid) used++;
    return used;
}

int get_total_frames() {
    return NUM_FRAMES;
}

int get_page_hits() { return page_hits; }
int get_page_faults() { return page_faults; }
