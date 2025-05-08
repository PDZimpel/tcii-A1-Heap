#include "GC.h"
#include<iostream>

namespace tcii::ex
{ // begin namespace tcii::ex


//////////////////////////////////////////////////////////
//
// Allocator implementation
// =========
Allocator* Allocator::_instance;

// insert your code here

inline BlockInfo*
Allocator::get_footer(BlockInfo *h)
{
  return reinterpret_cast<BlockInfo*>( (reinterpret_cast<char*>(h)) + h->size + sizeof(BlockInfo));
}

void
Allocator::initialize(uint size)
{
  uint total_size = size + 2*sizeof(BlockInfo);
  _instance = new Allocator(total_size);

  auto head = reinterpret_cast<BlockInfo*>(_instance->memory);
  head->flag = false;
  head->size = size;
  head->prev = head;
  head->next = head;

  BlockInfo* footer = _instance->get_footer(head);

  footer->flag = false;
  footer->prev = head;

  _instance->freeBlocks = head;
}


void
Allocator::fill_block(BlockInfo *h)
{
  //removing h from free blocks array
  h->next->prev = h->prev;
  h->prev->next = h->next;

  h->flag = true;
  auto footer = _instance->get_footer(h);

  footer->flag = true;
  footer->prev = h;
}

BlockInfo*
Allocator::split_block(BlockInfo *h, uint size_required)
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

BlockInfo*
Allocator::get_header(BlockInfo* f)
{
  return f->prev;
}

BlockInfo*
Allocator::get_prev_footer(BlockInfo* f)
  // calculates footer from the block on the left, basically position - sizeof(BlockInfo) and checking if inside the
  // memory area, nullptr means no more blocks to look for...
{
  if(reinterpret_cast<char*>(f) - sizeof(BlockInfo) >= _instance->memory)
    return reinterpret_cast<BlockInfo*>(reinterpret_cast<char*>(f)-sizeof(BlockInfo));
  else
    return nullptr;
}

BlockInfo*
Allocator::get_next_header(BlockInfo* h)
  // calculate next header position, same as prev_footer, nullptr means nothing to look for.
{
  auto footer = _instance->get_footer(h);
    if(reinterpret_cast<char*>(footer) + sizeof(BlockInfo) < _instance->end_of_block)
    return reinterpret_cast<BlockInfo*>(reinterpret_cast<char*>(footer)+sizeof(BlockInfo));
  else
    return nullptr;

}

void
Allocator::free(void *mem_block)
{
  BlockInfo *h = reinterpret_cast<BlockInfo*>(reinterpret_cast<char*>(mem_block) - sizeof(BlockInfo)); // get the header
  h->flag = false; // free block

  BlockInfo *prev_footer = _instance->get_prev_footer(h);
  bool prev_free = prev_footer && !prev_footer->flag; // condition: do we have free blocks on the left?

  BlockInfo *next_header = _instance->get_next_header(h);
  bool next_free = next_header && !next_header->flag; // condition: do we have free blocks on the right?

  if(prev_free && next_free) // ==> is there two free blocks around the current one? If so, merge the three of them into one
  { 
    auto left_h = _instance->get_header(prev_footer); // leftmost header, will be the new header

    //removing rightmost header from freeBlocks circular list
    next_header->prev->next = next_header->next; 
    next_header->next->prev = next_header->prev;

    // the new size will be the sum of sizes and the BlockInfos from the two merged blocks
    auto new_size = left_h->size + h->size + next_header->size + 4 * sizeof(BlockInfo);

    left_h->size = new_size;
    auto new_footer = _instance->get_footer(left_h);
    new_footer->flag = false;
    new_footer->prev = left_h;

  } else if(prev_free) // ==> if only one block around is free, is the leftmost the one? if so, merge
  {
    // again, leftmost header will be the new one, we calculate the size, no need to update freeBlocks, the current one isn't inside it...
    auto left_header = _instance->get_header(prev_footer);
    auto new_size = left_header->size +  h->size + 2*sizeof(BlockInfo);
    left_header->size = new_size;
    auto new_footer = _instance->get_footer(left_header);
    new_footer->prev = left_header;
    new_footer->flag = false;

  } else if(next_free) // ==> if only one block around is free, is the rightmost the one? merge...
  {
    //remove rightmost header from freeBlocks
    next_header->prev->next = next_header->next;
    next_header->next->prev = next_header->prev;
    // sums the two sizes and adds the rightmost header's BlockInfos
    auto new_size = h->size + next_header->size + 2*sizeof(BlockInfo);

    h->size = new_size, h->flag = false; // updates current block's BlockInfos
    auto new_f = _instance->get_footer(h);
    new_f->prev = h, new_f->flag = false;

    //inserts the new Block into freeBlocks
    h->next = _instance->freeBlocks->next;
    h->prev = _instance->freeBlocks;
    _instance->freeBlocks->next->prev = h;
    _instance->freeBlocks->next = h;

  } else // No free block around the target one, just add it to freeBloks and update the BlockInfos
  {
    h->flag = false;
    h->next = _instance->freeBlocks->next;
    h->prev = _instance->freeBlocks;

    _instance->freeBlocks->next->prev = h;
    _instance->freeBlocks->next = h;

    auto footer = _instance->get_footer(h);
    footer->flag = false;
  }
}

void
Allocator::printMemoryMap()
{
  using namespace std;
  cout << "\n" << endl;
  auto it = reinterpret_cast<BlockInfo*>(_instance->memory);
  uint total_size = 0;
  cout << "Block Address  | Flag  | Size  "<< endl;
  do
  {
    total_size += it->flag ? 0 : it->size;
    cout << it << " | " << it->flag << "     | " << it->size << endl;
    it = get_next_header(it);
  }while(it);

  cout << "\nFree Size: \n" << total_size << endl;
}

void
Allocator::exit()
{
  delete[] static_cast<char*>(_instance->memory);
  delete _instance;
}

} // begin namespace tcii::ex
