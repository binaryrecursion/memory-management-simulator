#include <iostream>
#include <string>
#include "../include/memory.h"

using namespace std;

int main() {
    string cmd;
    while (true) {
        cin >> cmd;
        if (cmd == "init") {
            int size; cin >> size;
            init_memory(size);
        } else if (cmd == "dump") {
            dump_memory();
        } 
        else if (cmd == "alloc") {
    int size; cin >> size;
    cout << malloc_first_fit(size) << endl;
}
else if (cmd == "exit") break;
    }
}
