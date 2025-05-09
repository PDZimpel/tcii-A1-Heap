#include "GC.h"
#include <cassert>
#include <iostream>

inline auto findBlockInfo(void* address)
{
  using tcii::ex::BlockInfo;
  char* add = reinterpret_cast<char*>(address);
  BlockInfo* info = reinterpret_cast<BlockInfo*>(add - sizeof(BlockInfo));
  return reinterpret_cast<void*>(info);
}

inline void
allocatorTest()
{
	using namespace tcii::ex;
  using namespace std;
	// insert your code here
  Allocator::initialize();
  
  std::cout << "Test 1: testing the split function \n Shows before and after inserting a int array of size 10" << std::endl;

  Allocator::printMemoryMap();
  auto split_test = Allocator::allocate<int>(10);
  
  std::cout << "\nOP: inserted block of size 40 into the heap" << std::endl;

  split_test[0] = 1;
  split_test[1] = 2;
  split_test[3] = 4;
  assert(split_test[0] == 1);
  assert(split_test[1] == 2);
  assert(split_test[3] == 4);
  Allocator::printMemoryMap();
  Allocator::exit();

  std::cout << "End of Test 1. ====="<< std::endl;
  
  std::cout << "\nPress Enter to continue testing" << std::endl;
  std::cin.get();

  Allocator::initialize(sizeof(int)*10);

  std:: cout << "Test 2: Fill function\nTests if we can allocate an entire block with the same size and wheter bad_alloc is thrown if tryied to insert new value" << std::endl;

  Allocator::printMemoryMap();
  auto d = Allocator::allocate<int>(10);
  std::cout << "\nOP: inserted block of size 40 into the heap" << std::endl;
  Allocator::printMemoryMap();

  std::cout << "\nOP: inserted block of size 5 into the heap" << std::endl;
  try
  {
    auto aa = Allocator::allocate<char>(5);
  }
  catch(const std::bad_alloc& e)
  {
    std::cerr << e.what() << '\n';
    std::cout << "exception correctly thrown\n" << std::endl;
  }
  

  std::cout << "End of Test 2. =====" << std::endl;
  Allocator::exit();

  std::cout << "\nPress Enter to continue testing" << std::endl;
  std::cin.get();

  Allocator::initialize();
  int *arr[5];
  for(int i = 0; i < 5; i++)
  {
    arr[i] = Allocator::allocate<int>(40);
  }

  std::cout << "Test 3: Remove operation \nTests primary free function in block surrounded by allocated blocks" << std::endl;

  Allocator::printMemoryMap();
  Allocator::free(arr[3]);
  std::cout << "\nOP: removed block at " << findBlockInfo(arr[3]) << std::endl;
  Allocator::printMemoryMap();
  Allocator::free(arr[1]);
  std::cout << "\nOP: removed block at " << findBlockInfo(arr[1]) << std::endl;
  Allocator::printMemoryMap();

  std::cout << "End of Test 3. =====" << std::endl;

  std::cout << "\nPress Enter to continue testing" << std::endl;
  std::cin.get();

  std::cout << "Test 4: Remove and Merge\n Tests if the free function correctly manages other free blocks around the target one. First freeing block in the middle, then the one at the end and freeing the remaining one" << std::endl;

  Allocator::printMemoryMap();
  Allocator::free(arr[2]);
  std::cout << "\nOP: removed block at" << findBlockInfo(arr[2]) << std::endl;
  Allocator::printMemoryMap();
  Allocator::free(arr[0]);
  std::cout << "\nOP: removed block at" << findBlockInfo(arr[0]) << std::endl;
  Allocator::printMemoryMap();
  Allocator::free(arr[4]);
  std::cout << "\nOP: removed block at" << findBlockInfo(arr[4]) << std::endl;
  Allocator::printMemoryMap();

  std::cout << "End of test 4. =====" << std::endl;

  std::cout << "\nPress Enter to stop" << std::endl;
  std::cin.get();

  Allocator::exit();
}

//
// Main function
//
int
main()
{
	allocatorTest();
	return 0;
}
