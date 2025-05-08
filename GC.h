#ifndef __GC_h
#define __GC_h

#include <new>

namespace tcii::ex
{ // begin namespace tcii::ex

using uint = unsigned int;

//////////////////////////////////////////////////////////
//
// BlockInfo class
// =========
struct BlockInfo
{
	bool flag;
	uint size;
	BlockInfo* prev;
	BlockInfo* next;

}; // BlockInfo


//////////////////////////////////////////////////////////
//
// Allocator class
// =========
class Allocator
{
public:
	static constexpr auto minBlockSize = 16u;
	static constexpr auto heapSize = 1048576u;

	static void initialize(uint size = heapSize);
  template <typename T> static T* allocate(uint size);
	static void free(void*);
	static void printMemoryMap();
	static void exit();

private:
 	static Allocator* _instance;
	// insert your code here

  uint totalSize; // total size of the heap
  char *memory, *end_of_block; // pointers to the beggining and end of the memory block
  BlockInfo *freeBlocks; // pointer to the first free block in the circular list

  Allocator(uint size):
    totalSize(size),
    memory(nullptr),
    freeBlocks(nullptr)
  {
    memory = new char[totalSize];
    end_of_block = memory + totalSize;
  }

  static BlockInfo* get_prev_footer(BlockInfo* h);
  static BlockInfo* get_next_header(BlockInfo* h);
  static BlockInfo* get_header(BlockInfo* h);
  static BlockInfo* split_block(BlockInfo *h, uint size_required);
  static void fill_block(BlockInfo *h);
  static BlockInfo* get_footer(BlockInfo *h);

}; // Allocator

template<typename T>
T*
Allocator::allocate(uint size)
{
  auto h = _instance->freeBlocks;
  auto it = h; // cannot reference
  uint fill_size = size*sizeof(T);
  uint split_size = 2*sizeof(BlockInfo) + size*sizeof(T); // if we split, we'll need 2 new BlockInfo's
  do{
    
    if(size > it->size){
      it = it->next;
      continue;
    }

    uint remaining_size = it->size - fill_size;
    if(it->size >= size){
      if(remaining_size <= _instance->minBlockSize || it->size == fill_size)
      {
        _instance->fill_block(it);
        return reinterpret_cast<T*>(reinterpret_cast<char*>(reinterpret_cast<char*>(it) + sizeof(BlockInfo)));
      } else if(split_size <= it->size){
        return reinterpret_cast<T*>(_instance->split_block(it, split_size));
      }
    }

    it = it->next;

  }while(it != h);

  throw std::bad_alloc();
}
}
#endif // __GC_h
