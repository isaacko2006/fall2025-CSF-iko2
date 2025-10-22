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
  uint32_t tag = 0;
  bool valid = false;
  uint32_t last_used = 0;
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

//accesses cache and returns boolean of whether it was a hit or not (is_store not used for milestone2)
bool accessCache(Cache &cache, uint32_t address, bool is_store, uint32_t &timestamp)
{
  //increment timestamp for LRU
  timestamp++;

  //break down address into index and tag
  //don't need offset because data can't span multiple blocks
  uint32_t index = (address >> cache.offset_bits) & ((1 << cache.index_bits) - 1);
  uint32_t tag = address >> (cache.index_bits + cache.offset_bits);

  //obtain specific set for index
  Set &set = cache.sets[index];

  //search for matching tag (hit)
  for (size_t i = 0; i < set.slots.size(); ++i)
  {
    Slot &slot = set.slots[i];
    if (slot.valid && slot.tag == tag)
    {
      //hit, update LRU
      slot.last_used = timestamp;
      return true;
    }
  }

  //pointer to potential replacement slot for miss
  Slot *replace_slot = nullptr;

  //look for empty slot
  for (size_t i = 0; i < set.slots.size(); ++i)
  {
    if (!set.slots[i].valid)
    {
      replace_slot = &set.slots[i];
      break;
    }
  }

  //if not empty, replace least recently used (LRU)
  if (!replace_slot)
  {
    replace_slot = &set.slots[0];
    for (size_t i = 1; i < set.slots.size(); ++i)
    {
      if (set.slots[i].last_used < replace_slot->last_used)
      {
        replace_slot = &set.slots[i];
      }
    }
  }

  //fill slot chosen 
  replace_slot->valid = true;
  replace_slot->tag = tag;
  replace_slot->last_used = timestamp;

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

  uint32_t num_sets = stoi(argv[1]);
  uint32_t num_blocks = stoi(argv[2]);
  uint32_t num_bytes = stoi(argv[3]);
  string write_alloc = argv[4];
  string write_mode = argv[5];
  string remove_method = argv[6];

  //error checking based on instructions of assignment
  if (!isPowerOfTwo(num_sets) || !isPowerOfTwo(num_blocks) || !isPowerOfTwo(num_bytes))
  {
    cerr << "Error: All numeric parameters must be powers of 2.\n";
    return 1;
  }

  if (num_bytes < 4)
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

  //variables to store statistics
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
    uint32_t address;

    //stringstream to interpret string as number
    stringstream ss;
    //convert hex address into and int and store into address
    ss << hex << hex_address;
    ss >> address;

    bool hit = false;

    //simulates load instruction
    if (operation == "l")
    {
      total_loads++;
      hit = accessCache(cache, address, false, timestamp);
      if (hit)
      {
        //1 cycle for hit
        load_hits++;
        total_cycles += 1;
      }
      else
      {
        //100 extra cycles for miss
        load_misses++;
        total_cycles += 1 + 100;
      }
    }

    //simulates store instruction
    else if (operation == "s")
    {
      total_stores++;
      hit = accessCache(cache, address, true, timestamp);
      if (hit)
      {
        //1 cycle for hit
        store_hits++;
        total_cycles += 1;
      }
      else
      {
        //100 extra cycles for miss
        store_misses++;
        total_cycles += 1 + 100;
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
