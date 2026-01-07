# Memory Management Simulator — Design Document

## 1. Memory Layout & Assumptions
Physical memory is modeled as a contiguous array divided into fixed‑size frames.  
Virtual memory provides each  process with its own address space, and
page tables map virtual pages to physical frames.

**Assumptions**

- fixed page/frame size   
- demand‑paging: pages are created on first access (page fault)  
- statistics are simulated (not hardware‑measured)

Virtual Addr → Page Table → Physical Addr → Cache → Main Memor

---

## 2. Allocation Strategies (First_fit, Best_Fit, Worst_Fit)

A free‑list tracks blocks inside a simulated heap.Free contiguous memmory are grouped together as required.

| Strategy   | Description                         | 
|------------|-------------------------------------|
| First Fit  | first block large enough            |
| Best Fit   | smallest block that fits           |
| Worst Fit  | largest available block            |

Fragmentation and utilization statistics are present in comparision table.



## 3. Buddy System Design
Memory is divided into **power‑of‑two** sized blocks.Free contiguous memmory are grouped together as required.

**Algorithm**

1. round request to nearest power‑of‑two  
2. split larger blocks recursively  
3. on free, merge buddies whenever both are free  

 fast split/merge, zero external fragmentation  
 possible internal fragmentation

![Diagram](images/buddy.png)


---

## 4. Cache Hierarchy & Replacement
Two‑level cache model:

- **L1** — small & fast  
- **L2** — larger & slower  

Lookup order:

L1 → L2 → Main Memory

Replacement policy: **FIFO**.  

We track:

- cache hits / misses  
- main‑memory accesses  
- symbolic cycle cost (illustrative, not hardware‑accurate)

Spatial locality appears naturally because entire blocks are fetched per access.

---

## 5. Virtual Memory Model
Paging maps virtual pages to physical frames.

**Access flow**

1. compute page number + offset  
2. page‑table lookup  
3. if missing → page fault  
4. allocate a frame and install mapping  

Page replacement uses **LRU**.  
We track page hits, faults, and per‑process frame usage.

---

## 6. Address Translation Flow

       ┌───────────────┐
       │ Virtual Addr  │
       └───────┬───────┘
               ↓
       ┌───────────────┐
       │ Page | Offset │
       └───────┬───────┘
               ↓
       ┌───────────────┐
       │ Page Table    │
       └───────┬───────┘
        page present?
        ┌──────┴──────┐
       YES           NO
        |            |
        |      ┌───────────────┐
        |      │ Page Fault   │
        |      │ (allocate or │
        |      │  replace)    │
        |      └───────┬──────┘
        |              ↓
        └────────► Physical Frame ◄────────┘
                      ↓
              ┌───────────────┐
              │ Cache Lookup  │
              └───────┬───────┘
                      │
                    hit?
            ┌─────────┴─────────┐
           YES                 NO
            │                   │
    ┌───────────────┐   ┌───────────────┐
    │   Return      │   │   Main Mem    │
    │    (L1/L2)    │   │  (load data)  │
    └───────┬───────┘   └───────┬───────┘
            └──────────────► Data Returned




## 7. Frame Reuse & Multi‑Process Behavior
Frames belong to a **global frame pool**.

- when a frame is evicted from Process‑A  
- its page‑table entry is **invalidated**  
- the frame may be reassigned to Process‑B  

> This mirrors real OS behavior: frames move between processes, while
> page‑table entries remain process‑specific and are updated safely.

Advanced sharing (copy‑on‑write, shared libraries) is **not modeled**.

---

## 8. Compare Mode (Allocation Strategy Comparison)

The simulator can replay the same workload under multiple strategies  
(**FF, BF, WF, Buddy**) and report statistics side‑by‑side:

- allocation successes / failures  
- memory utilization  
- internal / external fragmentation  
- total allocations and frees  

This mode does not change allocator behavior — it only **evaluates** it.

---


## 9. Limitations & Simplifications (why we chose them)

- **Implicit demand paging** — focus on paging instead of crash behavior  
- **Heap & paging independent** — simpler, easier to reason about  
- **LRU (pages) / FIFO (cache)** — clear and explainable policies  
- **Symbolic timing** — shows trends without hardware modeling  
- **No TLB, interrupts, permissions, or real disk I/O** — reduced complexity  
---

## 10. Testing & Validation

Workloads exercise:

- allocation & fragmentation  
- buddy split/merge behavior  
- page faults vs. hits  
- cache hit/miss patterns  
- strategy comparison  

Outputs go to `output/`, with combined results in:


Screenshots and the demo video illustrate expected behavior.

---

## 11. Conclusion
The simulator integrates allocation, paging, and caching into one framework,
highlighting performance and fragmentation trade‑offs while staying
transparent and educational.


