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

  template <typename T>
  static T*
  allocate(uint size)
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


	static void free(void*);
	static void printMemoryMap();
	static void exit();

private:
 	static Allocator* _instance;
	// insert your code here
  uint totalSize; // total size of the heap
  char *memory, *end_of_block; // pointer to the beggining of the memory block
  BlockInfo *freeBlocks; // pointer to the first free block in the circular list

  static BlockInfo* get_prev_footer(BlockInfo* h);
  static BlockInfo* get_next_header(BlockInfo* h);
  static BlockInfo* get_header(BlockInfo* h);

  Allocator(uint size):
    totalSize(size),
    memory(nullptr),
    freeBlocks(nullptr)
  {
    memory = new char[totalSize];
    end_of_block = memory + totalSize;
  }

  static BlockInfo*
  split_block(BlockInfo *h, uint size_required)
  {

    auto block_size = size_required - 2*sizeof(BlockInfo);
   
    h->size -= size_required;
    auto f = _instance->get_footer(h); // f is now the new footer of the splitten block h
    f->flag = false;
    f->prev = h;
    
    auto new_header = reinterpret_cast<BlockInfo*>(
        reinterpret_cast<char*>(f) + sizeof(BlockInfo) //calculate the new header position from the footer of the block fom the left
    );

    new_header->flag = true;
    new_header->size = block_size;

    f = _instance->get_footer(new_header);
    f->prev = new_header;
    f->flag = true;
    
    return reinterpret_cast<BlockInfo*>(new_header + 1); // ==> new_header + sizeof(BlockInfo) ...
  }

  static void
  fill_block(BlockInfo *h)
  {
    //removing h from free blocks array
    h->next->prev = h->prev;
    h->prev->next = h->next;

    h->flag = true;
    auto footer = _instance->get_footer(h);

    footer->flag = true;
    footer->prev = h;
  }

  static BlockInfo* get_footer(BlockInfo *h);

}; // Allocator
}
#endif // __GC_h
