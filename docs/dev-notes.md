# Development Notes and Lessons Learned

Building a lock-free skip list turned out to be harder than I expected, mostly because of subtle interactions between memory reclamation and pointer marking. Here are a few technical notes from working through the implementation.

## 1. Failed Approach: Simple Reference Counting
I initially tried using shared pointers with atomic control blocks to manage node lifecycles. That idea failed quickly. Standard atomic shared pointer operations introduced noticeable atomic reference count overhead on every traversal step, completely tanking performance. It also didn't play nicely with the bit-marking trick needed for two-phase logical deletion.

## 2. A Hard Concurrency Bug
The most frustrating bug I hit was a segmentation fault under 8-thread stress testing during node removal. Reads would occasionally dereference a node right after another thread physically unlinked it. I realized I was marking level 0 first, which caused a race condition where helper threads unlinked the node before level 1..N pointers were marked. Flipping the order to mark higher level pointers first and level 0 last fixed the race, because level 0 is what readers check to determine if a node is live.

## 3. Why I Chose Hazard Pointers
I considered epoch-based reclamation (EBR), but EBR can defer memory freeing indefinitely if a single thread gets stalled or preempted inside a critical section. Hazard pointers give a tighter memory bound. Each reader thread explicitly publishes up to two active pointers it is visiting (the predecessor and current node). Once a node is unlinked and no thread's hazard slot points to it, it gets deleted promptly.

## 4. What Is Still Not Perfect
The fixed hazard pointer domain uses a static thread-local record pool capped at 128 threads. If more threads join or threads get created and destroyed frequently, record allocation isn't dynamic. Also, the random level generator uses a uniform real distribution instead of a faster bit-counting PRNG, which adds a tiny overhead during insertion.
