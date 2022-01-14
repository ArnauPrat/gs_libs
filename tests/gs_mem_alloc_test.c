

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GS_MEM_ALLOC_IMPLEMENTATION
#include "gs_mem_alloc.h"

#define GS_STACK_TEST_SIZE 1024*1024
#define GS_SCRATCH_TEST_SIZE 1024*1024
#define GS_POOL_TEST_SIZE 1024*1024

bool 
gs_stack_test()
{
  void* ptr = malloc(GS_STACK_TEST_SIZE);
  if(!ptr)
    return false;

  unsigned long long allocation_sizes[] = {4, 8, 16, 20, 24, 30, 32, 48, 128, 160, 256, 500, 512, 720, 1024};
  int count_sizes = sizeof(allocation_sizes) / sizeof(unsigned long long);
  unsigned int allocation_alignments[] = {4, 8, 16, 32, 64};
  int count_alignments = sizeof(allocation_alignments) / sizeof(unsigned int);
  int max_allocations = 1024;
  void** allocations = malloc(sizeof(void*)*max_allocations);
  if(!allocations)
    return false;

  GSStack stack = gs_stack_init(ptr, GS_STACK_TEST_SIZE);
  // Testing full stack allocation and deallocation
  int count_allocations = 0;
  while(count_allocations < max_allocations)
  {
    unsigned long long next_size = allocation_sizes[(unsigned int)rand() % count_sizes];
    unsigned int next_alignment = allocation_alignments[(unsigned int)rand() % count_alignments];
    GSAlloc alloc = GS_STACK_PUSH_ALIGNED(&stack, next_size, next_alignment);
    if(gs_alloc_is_null(&alloc))
      break;
    allocations[count_allocations++] = gs_alloc_ptr(&alloc);
  } 

  for (int i = count_allocations-1; i > 0; --i) 
  {
    GS_STACK_POP(&stack, allocations[i]);
  }
  GS_ASSERT(stack.p_current = stack.p_begin)

  // Testing stack flush 
  while(count_allocations < max_allocations)
  {
    unsigned long long next_size = allocation_sizes[(unsigned int)rand() % count_sizes];
    unsigned int next_alignment = allocation_alignments[(unsigned int)rand() % count_alignments];
    GSAlloc alloc = GS_STACK_PUSH_ALIGNED(&stack, next_size, next_alignment);
    if(gs_alloc_is_null(&alloc))
      break;
    allocations[count_allocations++] = gs_alloc_ptr(&alloc);
  } 
  GS_STACK_FLUSH(&stack);
  GS_ASSERT(stack.p_current = stack.p_begin)

  // Testing random stack usage
  int count_iterations = 1000000;
  count_allocations = 0;
  for(int i = 0; i < count_iterations; ++i)
  {
    bool push = (unsigned int)rand() % 2 == 0;
    if((push && count_allocations > 0 && count_allocations < max_allocations) || count_allocations == 0)
    {
      unsigned long long next_size = allocation_sizes[(unsigned int)rand() % count_sizes];
      unsigned int next_alignment = allocation_alignments[(unsigned int)rand() % count_alignments];
      GSAlloc alloc = GS_STACK_PUSH_ALIGNED(&stack, next_size, next_alignment);
      if(gs_alloc_is_null(&alloc))
      {
        push = false;
      }
      allocations[count_allocations++] = gs_alloc_ptr(&alloc);
    }

    if(!push) // this is not an else because it can happen (although improbable) that we attempt to allocate and the stack is full
    {
      GS_STACK_POP(&stack, allocations[count_allocations-1]);
      --count_allocations;
    }
  }
  GS_STACK_FLUSH(&stack);
  GS_ASSERT(stack.p_current = stack.p_begin)

  free(allocations);
  free(ptr);
  return true;
}

bool
gs_scratch_test()
{
  void* ptr = malloc(GS_SCRATCH_TEST_SIZE);
  if(!ptr)
    return false;

  unsigned long long allocation_sizes[] = {4, 8, 16, 20, 24, 30, 32, 48, 128, 160, 256, 500, 512, 720, 1024};
  int count_sizes = sizeof(allocation_sizes) / sizeof(unsigned long long);
  unsigned int allocation_alignments[] = {4, 8, 16, 32, 64};
  int count_alignments = sizeof(allocation_alignments) / sizeof(unsigned int);
  int max_allocations = 1024;

  GSScratch scratch = gs_scratch_init(ptr, GS_SCRATCH_TEST_SIZE);

  GSAlloc alloc = {};
  do
  {
    unsigned long long next_size = allocation_sizes[(unsigned int)rand() % count_sizes];
    unsigned int next_alignment = allocation_alignments[(unsigned int)rand() % count_alignments];
    alloc = GS_SCRATCH_PUSH_ALIGNED(&scratch, next_size, next_alignment);
  } while(!gs_alloc_is_null(&alloc));

  GS_SCRATCH_FLUSH(&scratch);
  GS_ASSERT(scratch.p_current = scratch.p_begin)

  free(ptr);
  return true;
}

bool
gs_pool_test()
{
  void* ptr = malloc(GS_POOL_TEST_SIZE);
  if(!ptr)
    return false;

  unsigned long long block_sizes[] = {8, 16, 20, 24, 30, 32, 48, 128, 160, 256, 500, 512, 720, 1024};
  int count_sizes = sizeof(block_sizes) / sizeof(unsigned long long);
  unsigned int block_alignments[] = {4, 8, 16, 32, 64};
  int count_alignments = sizeof(block_alignments) / sizeof(unsigned int);
  int max_allocations = 1024;
  void** allocations = malloc(sizeof(void*)*max_allocations);
  if(!allocations)
    return false;



  for(int i = 0; i < count_sizes; ++i)
  {
    for (int j = 0; j < count_alignments; ++j) 
    {
      unsigned long long block_size = block_sizes[i];
      unsigned int block_alignment = block_alignments[j];
      GSPool pool = gs_pool_init(ptr, 
                                 GS_POOL_TEST_SIZE, 
                                 block_size,
                                 block_alignment);
      memset(allocations, 0, sizeof(void*)*max_allocations);

      for(int k = 0; k < max_allocations; ++k)
      {
        int index = (unsigned int)rand() % max_allocations;
        if(allocations[index] == NULL)
        {
          GSAlloc alloc = GS_POOL_ALLOC_ALIGNED(&pool, 
                                                block_size, 
                                                block_alignment);
          if(gs_alloc_is_null(&alloc))
          {
            allocations[index] = NULL;
          }
          allocations[index] = gs_alloc_ptr(&alloc);
        }
        else
        {
          GS_POOL_FREE(&pool, allocations[index]);
          allocations[k] = NULL;
        }
      }
      GS_POOL_FLUSH(&pool);
      GS_ASSERT(pool.p_current = pool.p_begin)
    }
  }


  free(ptr);
  return true;
}

int 
main(int argc, char** argv)
{
  int EXIT_CODE = 0;

  if(!gs_stack_test())
  {
    EXIT_CODE = 1;
    goto exit;
  }

  if(!gs_scratch_test())
  {
    EXIT_CODE = 1;
    goto exit;
  }
  
  if(!gs_pool_test())
  {
    EXIT_CODE = 1;
    goto exit;
  }

exit:
  return EXIT_CODE;
}
