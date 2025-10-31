TODO: names of team members and their contributions to the project
Milestone 1:
Isaac Ko: Set up Github repository, downloaded and starter code, put in #include statements and a brief framework for main.cpp
Gavin Simurdiak: Adjusted Isaac's original main.cpp, added error checking for parameter arguments and file not opening, stores contents of file to variables and output to cout

Milestone 2:
Isaac Ko: Implemented sets in the cache, blocks in each set, bytes in each block, and helped with LRU. Created write-through or write-back.
Gavin Simurdiak: Completed implementation of LRU and performed error checking on all functions. Created write-allocate or no-write-allocate.

Milestone 3:
Isaac Ko: Implemented cycle counting logic, fixed write-back eviction costs, and conducted experiments on various cache configurations.
Gavin Simurdiak: Implemented FIFO policy and assisted with cycle counting fixes and testing.

We conducted experiments on the data sets of gcc.trace.

In our experiments we took into account:
1. Cache size:  1K, 4K, 8K, 16K
2. Associativity: direct mapped, 2-way, 4-way, 8-way, 16-way
3. Block size: 4, 8, 16, 32, 64 bytes
4. All write policies (write-allocate and no-write-allocate, write-back and write-through)
5. Both of the replacement policies (LRU and FIFO)

Experiment Results: 
1. Cache Size (gcc.trace):

Command: ./csim 64 2 8 write-allocate write-back lru < gcc.trace
Result: 16,929,594 cycles (1K cache)

Command: ./csim 256 2 8 write-allocate write-back lru < gcc.trace
Result: 10,349,134 cycles (4K cache)

Command: ./csim 512 2 8 write-allocate write-back lru < gcc.trace
Result: 9,318,494 (8K cache)

Command: ./csim 256 4 16 write-allocate write-back lru < gcc.trace
Result: 9,353,719 cycles (16K cache)

Reason for commands:
We tested 1k, 4k, and 16k caches in order to represent differently sized caches.
By pairing 2-way associativity and 8 or 16 bytes sizes, we could test the effect 
of cache sizes on different associativities and byte sizes for varied results.

Findings:
1K and 4K caches showed poor performance with cycle counts exceeding 10M. 4K and 8k
provided cycle counts of around 9-10M. 16K cache showed optimal performance with cycle 
counts around 9-9.5M, making 16K caches the most optimal.

2. Associativity Impact (gcc.trace):

Command: ./csim 1024 1 16 write-allocate write-back lru < gcc.trace
Result: 11,137,267 cycles (1-way)

Command: ./csim 512 2 16 write-allocate write-back lru < gcc.trace
Result: 9,598,245 cycles (2-way)

Command: ./csim 256 4 16 write-allocate write-back lru < gcc.trace
Result: 9,353,719 cycles (4-way)

Command: ./csim 128 8 16 write-allocate write-back lru < gcc.trace
Result: 9,286,891 cycles (8-way)

Command: ./csim 64 16 16 write-allocate write-back lru < gcc.trace
Result: 9,276,093 cycles (16-way)

Reason for commands:
For associativity testing, we used a fixed 16KB cache and fixed write policy 
to eliminate capacity and write policy as a variable. We tested 1-way through 16-way 
associativity, which allowed us to observe any existing patterns.

Findings: 
The higher the associativity, the better the results but resulted in worse and worse
improvements after 4-way and 8-way. 

3. Block Size Impact (gcc.trace):

Command: ./csim 512 2 4 write-allocate write-back lru < gcc.trace
Result: 7,494,514 cycles (4 bytes)

Command: ./csim 256 2 8 write-allocate write-back lru < gcc.trace
Result: 10,349,134 cycles (8 bytes)

Command: ./csim 128 2 16 write-allocate write-back lru < gcc.trace
Result: 13,435,670 cycles (16 bytes)

Command: ./csim 64 2 32 write-allocate write-back lru < gcc.trace
Result: 20,731,329 cycles (32 bytes)

Command: ./csim 32 2 64 write-allocate write-back lru < gcc.trace
Result: 38,227,145 cycles (64 bytes)

Reason for commands:
We kept the total block size, write polciies, replacement policies, and associativity 
constant while testing the different block sizes to isolate the impact of the different
block sizes.

Findings:
The smaller block sizes (4 bytes) provide better performance than the larger blocks.

4. Write Policy Impact:

Command: ./csim 256 4 16 write-allocate write-back lru < gcc.trace
Result: 9,353,719 cycles

Command: ./csim 256 4 16 write-allocate write-through lru < gcc.trace
Result: 25,318,283 cycles

Command: ./csim 256 4 16 no-write-allocate write-through lru < gcc.trace
Result: 22,865,216 cycles

Reason for commands:
We tested the three valid combinations of write policies (write-allocate+write-back, 
write-allocate+write-through, no-write-allocate+write-through) using the same cache
config in order to isolate the impact of the write policy. 

Findings:
Write-back outperforms write-through by avoiding repetitive memory writes. Also, 
write-allocate is needed for caching stores efficiently.

5. Replacement Policy Impact (gcc.trace):

Command: ./csim 256 4 16 write-allocate write-back lru < gcc.trace
Result: 9,353,719 cycles (LRU)

Command: ./csim 256 4 16 write-allocate write-back fifo < gcc.trace
Result: 9,854,722 cycles (FIFO)

Command: ./csim 256 1 16 write-allocate write-back lru  < gcc.trace
Result: 20,324,767 (LRU)

Command: ./csim 256 1 16 write-allocate write-back fifo < gcc.trace
Result: 20,324,767 (FIFO)

Command: ./csim 32 8 16 write-allocate write-back lru  < gcc.trace
Result: 11,078,352 (LRU)

Command: ./csim 32 8 16 write-allocate write-back fifo < gcc.trace
Result: 12,618,535 (FIFO)

Reason for commands:
By testing LRU vs FIFO with identical configurations, we could measure the pure impact 
of replacement strategy. We tested several different configurations.

Findings:
LRU performs slightly better than FIFO but the performance difference is negligible.

Recommendations:
Based on our experiments, we recommend the following cache configuration:
16KB cache, write-allocate write-back, LRU, 4-way set-associative, 16 bytes

