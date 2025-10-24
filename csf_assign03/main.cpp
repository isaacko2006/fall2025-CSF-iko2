#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>
#include <sstream>

using namespace std;

//represents one cache line aka a slot
struct Slot
{
  uint32_t tag = 0;        // tags the portion of memory address for this cache line
  bool valid = false;      // boolean to check if this slot contains valid data
  uint32_t last_used = 0;  // timestamp for LRU 
  bool dirty = false;      // dirty bit for write-back policy (true if modified but not written to memory)
};

//represents set containing multiple slots
struct Set
{
  vector<Slot> slots;
};

//represents entire cache as a whole (with sets, and number of bits in index and offset)
struct Cache
{
  vector<Set> sets;
  uint32_t index_bits;
  uint32_t offset_bits;
};

//helper to check if value is a proper power of two (for error checking)
bool isPowerOfTwo(uint32_t n)
{
  //true if only one bit is set in n, uses bit arithmetic (& operator)
  return (n > 0) && ((n & (n - 1)) == 0);
}

//creates a cache with config provided in input
Cache createCache(uint32_t num_sets, uint32_t num_blocks, uint32_t num_bytes)
{
  Cache cache;

  cache.index_bits = log2(num_sets);
  cache.offset_bits = log2(num_bytes);
  cache.sets.resize(num_sets);

  //put correct number of slots in each set
  for (size_t i = 0; i < cache.sets.size(); ++i)
  {
    cache.sets[i].slots.resize(num_blocks);
  }

  return cache;
}

//accesses cache and returns boolean of whether it was a hit or not
//also returns additional cycles needed for write back
bool accessCache(Cache &cache, uint32_t address, bool is_store, uint32_t &timestamp, 
                string write_alloc, string write_mode, uint32_t &extra_cycles, uint32_t num_bytes)
{
  //increment timestamp for LRU
  timestamp++;  
  extra_cycles = 0;  // Initialize extra cycles counter for write-back evictions

  //break down address into index and tag
  //don't need offset because data can't span multiple blocks
  uint32_t index = (address >> cache.offset_bits) & ((1 << cache.index_bits) - 1);  // get index bits from address
  uint32_t tag = address >> (cache.index_bits + cache.offset_bits);  // get tag bits from address

  //obtain specific set for index
  Set &set = cache.sets[index];  

  //search for matching tag (hit)
  for (size_t i = 0; i < set.slots.size(); ++i)  //itereate through all slots in the set
  {
    Slot &slot = set.slots[i];  // get the reference to the current slot
    if (slot.valid && slot.tag == tag)  // check if the slot is valid and if the tag matches
    {
      //hit, update LRU
      slot.last_used = timestamp;  
      
      //begin store operations
      if (is_store) {
        if (write_mode == "write-back") {
          slot.dirty = true;  // set block to dirty for write-back 
        }
      }
      
      return true;  
    }
  }

  // check cache miss for write allocation 
  if (is_store && write_alloc == "no-write-allocate") {
    //no modification just return the miss
    return false;
  }

  //pointer to potential replacement slot for miss
  Slot *replace_slot = nullptr;  

  //look for empty slot
  for (size_t i = 0; i < set.slots.size(); ++i)  
  {
    if (!set.slots[i].valid)  // check if slot is invalid (empty)
    {
      replace_slot = &set.slots[i];  // set the pointer to the empty slot
      break;  
    }
  }

  //if not empty, replace least recently used (LRU)
  if (!replace_slot)  
  {
    replace_slot = &set.slots[0];  // check first slot as replacement
    for (size_t i = 1; i < set.slots.size(); ++i)  // check the remaining slots
    {
      if (set.slots[i].last_used < replace_slot->last_used)  // find the slot with oldest timestamp
      {
        replace_slot = &set.slots[i];  // update the replacement slot
      }
    }
    
    // check if we need to write back the dirty block for write back 
    if (write_mode == "write-back" && replace_slot->dirty) {
      extra_cycles += (num_bytes / 4) * 100;
    }
  }

  //fill slot chosen 
  replace_slot->valid = true;  
  replace_slot->tag = tag;  
  replace_slot->last_used = timestamp;  
  
  // check dirty bit for write back
  if (write_mode == "write-back" && is_store) {
    replace_slot->dirty = true;  // mark as dirty for store
  } else {
    replace_slot->dirty = false;  // mark as clean for write through
  }

  return false;  
}

int main(int argc, char **argv)
{
  //TODO: implement

  //check if correct number of command line arguments given (6 + 1 for file)
  if (argc < 7)  
  {
    cerr << "Not enough command line arguments";  
    return 1;  
  }

  uint32_t num_sets = stoi(argv[1]);    // get number of sets from arg.
  uint32_t num_blocks = stoi(argv[2]);  // get number of blocks per set from 2nd arg.
  uint32_t num_bytes = stoi(argv[3]);   // get block size in bytes from 3rd arg.
  string write_alloc = argv[4];         // get write allocation policy from 4th arg.
  string write_mode = argv[5];          // get write mode policy from 5th arg.
  string remove_method = argv[6];       // get eviction method from 6th arg.

  //error checking based on instructions of assignment
  if (!isPowerOfTwo(num_sets) || !isPowerOfTwo(num_blocks) || !isPowerOfTwo(num_bytes))  
  {
    cerr << "Error: All numeric parameters must be powers of 2.\n";  
    return 1;  
  }

  if (num_bytes < 4)  // check if the block size is at least 4 
  {
    cerr << "Error: Block size must be at least 4 bytes.\n";  
    return 1;  
  }

  if (write_alloc == "no-write-allocate" && write_mode == "write-back")  
  {
    cerr << "Error: Cannot use no-write-allocate with write-back.\n";  
    return 1;  
  }

  //create cache using helper function
  Cache cache = createCache(num_sets, num_blocks, num_bytes);  

  //create counter variables to store statistics
  uint64_t total_loads = 0;     
  uint64_t total_stores = 0;   
  uint64_t load_hits = 0;      
  uint64_t load_misses = 0;    
  uint64_t store_hits = 0;     
  uint64_t store_misses = 0;   
  uint64_t total_cycles = 0;   
  uint32_t timestamp = 0;

  //variables to read memory access trace from stdin
  string operation;     
  string hex_address;   
  //third field to ignore according to assignment
  int third_field;      

  //read in memory trace, continue while more info left
  while (cin >> operation >> hex_address >> third_field)  
  {
    uint32_t address;  // the address to store 

    //stringstream to interpret string as number
    stringstream ss; 
    //convert hex address into and int and store into address
    ss << hex << hex_address;  
    ss >> address;  // get the address in integer form

    bool hit = false;  // variable to store cache hit/miss status
    uint32_t extra_cycles = 0;  // variable to store extra cycles for write-back evictions

    //simulates load instruction
    if (operation == "l")  
    {
      total_loads++;  //increment load counter
      hit = accessCache(cache, address, false, timestamp, write_alloc, write_mode, extra_cycles, num_bytes);  
      if (hit)  
      {
        //1 cycle for hit
        load_hits++;  // increment load hits counter
        total_cycles += 1;  // add to total cycle
      }
      else  
      {
        //100 extra cycles for miss
        load_misses++;  //increment load miss
        total_cycles += 1 + (num_bytes / 4) * 100 + extra_cycles;
      }
    }

    //simulates store instruction
    else if (operation == "s")  
    {
      total_stores++;
      hit = accessCache(cache, address, true, timestamp, write_alloc, write_mode, extra_cycles, num_bytes);  
      if (hit) 
      {
        //1 cycle for hit
        store_hits++;
        total_cycles += 1;  
        
        // for write through add extra cycles
        if (write_mode == "write-through") {
          total_cycles += 100;
        }
      }
      else
      {
        store_misses++;
        
        if (write_alloc == "no-write-allocate") {
          // for no-write allocate, write directly to memory
          total_cycles += 100;  // Only memory write cycles
        } else {
          // for write allocate, allocate block and write
          if (write_mode == "write-through") {
            total_cycles += 1 + (num_bytes / 4) * 100 + 100 + extra_cycles;
          } else {
            total_cycles += 1 + (num_bytes / 4) * 100 + extra_cycles + 1;
          }
        }
      }
    }
  } 

  //printing out final statistics in format of instructions
  cout << "Total loads: " << total_loads << endl;
  cout << "Total stores: " << total_stores << endl;
  cout << "Load hits: " << load_hits << endl;
  cout << "Load misses: " << load_misses << endl;
  cout << "Store hits: " << store_hits << endl;
  cout << "Store misses: " << store_misses << endl;
  cout << "Total cycles: " << total_cycles << endl;

  return 0;
}
