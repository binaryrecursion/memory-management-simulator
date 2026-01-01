#ifndef VM_H
#define VM_H

#include <vector>
using namespace std;

struct PageTableEntry {
    bool valid;     
    int frame;      
    int last_used;  
};

void init_vm(int virtual_size, int physical_size, int page_size);
int vm_access(int vaddr);        
void dump_page_table();


int get_page_hits();
int get_page_faults();

#endif
