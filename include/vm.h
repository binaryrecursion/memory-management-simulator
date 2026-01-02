#ifndef VM_H
#define VM_H

#include <vector>
#include <unordered_map>
using namespace std;

extern int disk_penalty;

struct PageTableEntry {
    bool valid;
    int frame;
    int last_used;
};

void reset_vm_system(int physical_size, int page_size);


void init_vm(int pid, int virtual_size);
int vm_access(int pid, int vaddr);
void dump_page_table(int pid);

bool any_vm_initialized();
vector<int> get_initialized_pids();

int get_page_hits();
int get_page_faults();

int get_used_frames(int pid);
int get_total_frames();

#endif