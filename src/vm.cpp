#include "../include/vm.h"
#include <iostream>

using namespace std;

static int PAGE_SIZE;
static int NUM_PAGES;
static int NUM_FRAMES;

static vector<PageTableEntry> page_table;
static vector<int> frame_owner;      

static int time_counter = 0;
static int page_hits = 0;
static int page_faults = 0;


void init_vm(int virtual_size, int physical_size, int page_size) {

    PAGE_SIZE = page_size;
    NUM_PAGES = virtual_size / page_size;
    NUM_FRAMES = physical_size / page_size;

    page_table.assign(NUM_PAGES, {false, -1, 0});
    frame_owner.assign(NUM_FRAMES, -1);

    time_counter = 0;
    page_hits = 0;
    page_faults = 0;

    cout << "Virtual memory initialized\n";
}


static int choose_victim_frame() {
    
    int oldest_time = 1e9;
    int victim = 0;

    for (int f = 0; f < NUM_FRAMES; f++) {
        int p = frame_owner[f];
        if (page_table[p].last_used < oldest_time) {
            oldest_time = page_table[p].last_used;
            victim = f;
        }
    }
    return victim;
}


int vm_access(int vaddr) {

    time_counter++;

    int page   = vaddr / PAGE_SIZE;
    int offset = vaddr % PAGE_SIZE;

    PageTableEntry &pte = page_table[page];

    if (pte.valid) {
        page_hits++;
        pte.last_used = time_counter;
        return pte.frame * PAGE_SIZE + offset;
    }

    page_faults++;
    cout << "PAGE FAULT on page " << page << "\n";
    for (int f = 0; f < NUM_FRAMES; f++) {
        if (frame_owner[f] == -1) {
            frame_owner[f] = page;

            pte.valid = true;
            pte.frame = f;
            pte.last_used = time_counter;

            cout << "Loaded page " << page << " into frame " << f << "\n";
            return f * PAGE_SIZE + offset;
        }
    }

    int victim = choose_victim_frame();
    int victim_page = frame_owner[victim];

    cout << "Evicting page " << victim_page
         << " from frame " << victim << "\n";

    page_table[victim_page].valid = false;

    frame_owner[victim] = page;

    pte.valid = true;
    pte.frame = victim;
    pte.last_used = time_counter;

    return victim * PAGE_SIZE + offset;
}


void dump_page_table() {
    cout << "Page\tValid\tFrame\n";
    for (int i = 0; i < NUM_PAGES; i++) {
        cout << i << "\t"
             << page_table[i].valid << "\t"
             << page_table[i].frame << "\n";
    }
}

int get_page_hits()    { return page_hits; }
int get_page_faults()  { return page_faults; }
