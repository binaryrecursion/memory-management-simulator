# Memory Management Simulator
This project is a terminal‑based **Memory Management Simulator** that demonstrates how an operating system manages memory at a conceptual level.More details about this project is in design document.md file present in docs folder in root directory.Test cases are present in test folder.

It simulates:

- First Fit, Best Fit, Worst Fit allocation
- Buddy allocation (power‑of‑two)
- Virtual Memory with paging and demand loading
- Page faults and page replacement
- Two‑level cache hierarchy (L1 / L2)
- Runtime statistics and strategy comparison

---

##  Project Structure
* `src/` :      implementation files (.cpp) including main.cpp for CLI
* `include/` :    header files (.h)
* `test/` :     input workloads
* `output/` :   generated logs (created when tests run)
* `docs/` :   design document + images
* `run_tests.sh` :      Linux/Mac automated test runner
* `run_tests.bat` :      Windows automated test runner
* `Makefile`


---

##  How to Build and Run

### Requirements
- C++17 compiler (g++ / clang++)
- `make`

### Build
Run the following command in the root directory:
```bash
make
```
If make doesnt work ,Run
```bash
mingw32-make
```
If even this doesnt work then ,Run
```bash
g++ -std=c++17 -O2 -Wall src/main.cpp src/memory.cpp src/buddy.cpp src/cache.cpp src/vm.cpp -Iinclude -o memsim
```
### Run the simulator
Linux / Mac
```bash
./memsim
```

Windows
```bash
memsim.exe
```
Type commands directly in the terminal.

### Help
Type help to see all the functions avaiable after running the simulator.

---
## Running the tests
### Manually
Linux / Mac
```bash
./memsim < test/example.txt
```
Windows
```bash
memsim.exe < test\example.txt
```
output will be in terminal
### Automated 
Linux / Mac
```bash
./run_tests.sh
```
Windows
```bash
run_tests.bat
```
 this will run all test workloads and save logs into all_tests_output.txt (combined output) and output folder for individual test output like buddy_log.txt.

---
 ## Features Implemented 
1. **Memory Allocation Strategies**: First Fit, Best Fit, Worst Fit, and Buddy System.
2. **Virtual Memory**: Per-process Page Tables mapping Virtual Pages to Physical Frames(user configurable)
3. **Demand Paging**: Lazy loading of pages (Page Fault handling).
4. **Page Replacement**: FIFO eviction policy in cache and LRU eviction policy in virtual memory.
5. **Cache Hierarchy**:Associativity is user configurable  
*   L1 Cache
*   L2 Cache
6. **Deallocation**: Proper cleanup of Virtual Blocks and Physical Frames.
7. **Allocator comparision** :Comparison table running on same set of operations comparing fragmentation ,hits , utilization among diffenret allocators.
8. **Statistics** :Dump and stats functions to tell page hits ,frame used ,L1 ,L2 hits and misses and ,memory utilization ,fragmentation,allocation hits and misses.

---
## Demo Video

https://drive.google.com/file/d/11ZG9KcEeEBjVAhyUlZ9K2osP86sQktJB/view?usp=sharing
increase the quality to 1080 p and initially few seconds the quality is pretty poor but after 5-10 seconds it becomes good

---
## Assumptions & Simplifications
* Implicit demand paging: unmapped pages trigger a page fault and are automatically mapped (no segmentation faults).
* Heap & paging are independent: allocators manage heap; paging manages frames/page tables separately.
* No protection bits: R/W/X permissions are not simulated.
* Abstracted CPU behavior: we model translation flow, not full instruction execution or traps.
* Simplified replacement: LRU for pages, FIFO for cache (no dirty‑bit/disk writes).


---
## Note
* output/ is created automatically when tests run
* Logs are generated rather than stored in Git
* All tests are reproducible using the provided scripts

