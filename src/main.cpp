#include <iostream>
#include <string>

#include "../include/memory.h"


using namespace std;



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


            cout << "Memory initialized\n";
        }

        else if (cmd == "alloc") {
            string type;
            int size;
            cin >> type >> size;

            int addr = -1;

            if (type == "ff") {

                addr = malloc_first_fit(size);
            }
            else if (type == "bf") {

                addr = malloc_best_fit(size);
            }
            else if (type == "wf") {

                addr = malloc_worst_fit(size);
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

            }
        }

        else if (cmd == "free") {
            int addr;
            cin >> addr;

                free_block(addr);

            workload.push_back({FREE_EVENT, addr});

            cout << "Block freed\n";
        }


     


        else if (cmd == "dump") {
            dump_memory();
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