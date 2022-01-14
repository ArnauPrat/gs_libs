

#include <stdio.h>
#include <stdlib.h>

#define GS_MEM_ALLOC_IMPLEMENTATION
#include "gs_mem_alloc.h"

#define GS_STACK_TEST_SIZE 1024*1024

bool gs_stack_test()
{
  void* ptr = malloc(GS_STACK_TEST_SIZE);

  GSStack stack = gs_stack_init(ptr, GS_STACK_TEST_SIZE);

  int count_allocations = 0;
  for(int i = 0; i < count_allocations; ++i)
  {
  }

  free(ptr);
  return true;
}


int 
main(int argc, char** argv)
{
  printf("Executing gs_mem_alloc_test\n");
  int EXIT_CODE = 0;

  if(!gs_stack_test())
  {
    EXIT_CODE = 1;
    goto exit;
  }

exit:
  return EXIT_CODE;
}
