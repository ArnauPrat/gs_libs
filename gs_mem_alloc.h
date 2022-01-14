// gs_mem_alloc version 0.0.1 no warranty implied, use at your own risk
//
////////////////////////////////////////////////
////////////////// RELEASE NOTES ///////////////
////////////////////////////////////////////////
//
// - Version 0.0.1: 
//          - First version with stack, scratch and pool allocators
//
//
////////////////////////////////////////////////
////////////////// CONTRIBUTORS  ///////////////
////////////////////////////////////////////////
//
// - Arnau Prat Pérez
//
////////////////////////////////////////////////
////////////////// DOCUMENTATION ///////////////
////////////////////////////////////////////////
//
// This is a single-header C99/C++ library that implements a set of simple allocators: 
//  - GSStack:    a stack allocator with push and pop operations, which must be
//                performed in reverse order 
//  - GSScratch:  a linear allocator (AKA arena) with a push operation that appends 
//                the newly allocated memory block after the last one.
//  - GSPool:     a pool allocator with alloc and free operations to allocate
//                blocks of fixed size
//
//
// DEPENDENCIES:
// - stdbool.h when compiled in C99, 
// - stdio.h and signal.h when compiled GS_MEM_ALLOC_DISABLE_ASSERTS or
//   GS_MEM_ALLOC_DISABLE_CHECKS are not defined, 
// - string.h when GS_MEM_ALLOC_INITIALIZE_TO_ZERO is defined
//
// USAGE:
//
// Include the library as follows in a .c or .cpp file:
// #define GS_MEM_ALLOC_IMPLEMENTATION
// #include "gs_mem_alloc.h"
//
// To create an allocator (e.g. a stack) use the "init" method as follows,
// where ptr is a pointer to a buffer (either allocated with malloc, new, 
// or another allocator) and size is the size of that buffer:
//
// unsigned long long size = 1024*1024;
// void* ptr = malloc(size);
// GSStack stack = gs_stack_init(ptr, size);
//
// All basic memory allocation methods return a GSAlloc structure. This
// structure is like an "optional", and has two methods, one to check if it is
// null, and another one to retrieve the alocation pointer. If the method to
// retrieve the pointer is called without previously calling the  null-checking
// method, and GS_MEM_ALLOC_DISABLE_ASSERTS is not set, an assert will be
// thrown. For instance, a common usage is as follows:
//
// GSAlloc alloc = gs_stack_push(stack, size, GS_MIN_ALIGNMENT);
// if(gs_alloc_is_null(&alloc))
// {
//    // error handling code
// }
// void* data = gs_alloc_ptr(&alloc);
// ...
// gs_stack_pop(stack, data);
//
// The following code will throw an assert if GS_MEM_ALLOC_DISABLE_ASSERTS is
// not defined:
//
// GSAlloc alloc = gs_stack_push(stack, size, GS_MIN_ALIGNMENT);
// void* data = gs_alloc_ptr(&alloc);
//
// GS_MIN_ALINGMENT is a convenience macro specifying the minimum alignment that
// is guaranteed to work for any basic data type. In other words, is the minimum
// alignment that an allocation with malloc would guarantee
//
// Convenience macros are provided to improve readability and usage for all
// allocators (see below). For an stack allocator, these macros look as follows:
//
// GS_STACK_PUSH(stack, size)
// GS_STACK_PUSH_ALIGNED(stack, size, alignment)
// GS_STACK_POP(stack, ptr)
//
// Additionally, allocators provide the "CHECKED" version of the allocation
// methods (and the corresponding macros). These versions do not retorn a
// GSAlloc, but the pointer directly. However, if GS_MEM_ALLOC_DISABLE_CHECKS
// is not set, and the allocation would return a NULL ptr, an assert is thrown.
// 
// CONFIGURATION:
//
// The following are macros that can be defined before including the library.
// Those macros with a default value are automatically defined if not defined
// before.
//
// - GS_MIN_ALINGMENT                 : The minimum alignment guaranteed to work for
//                                      any basic data time. Default: 16
// - GS_PTR_NUMERIC_TYPE              : Numeric type used to manipulate pointer
//                                      addresses. Default: unsigned long long
// - GS_PTR_ALIGNMENT                 : Alignment used when storing pointers in
//                                      memory. Default: sizeof(void*)
// - GS_MEM_ALLOC_DISABLE_ASSERTS     : If defined, disables asserts
// - GS_MEM_ALLOC_DISABLE_CHECKS      : If defined, disables asserts in "CHECKED"
//                                      allocation operations
// - GS_MEM_ALLOC_INITIALIZE_TO_ZERO  : If defined, all allocations are zero
//                                      initialized
//
////////////////////////////////////////////////
/////////////////// LICENSE ////////////////////
////////////////////////////////////////////////
//
// Copyright © 2022 Arnau Prat Pérez
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef GS_MEM_ALLOC_H
#define GS_MEM_ALLOC_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif


#ifndef GS_MIN_ALIGNMENT
#define GS_MIN_ALIGNMENT          16
#endif

#ifndef GS_PTR_NUMERIC_TYPE
#define GS_PTR_NUMERIC_TYPE       unsigned long long 
#endif

#ifndef GS_PTR_ALIGNMENT
#define GS_PTR_ALIGNMENT          sizeof(void*)
#endif

#define GS_PTR_DIFF(ptr1, ptr2)\
            ((GS_PTR_NUMERIC_TYPE)ptr1) - ((GS_PTR_NUMERIC_TYPE)ptr2)


#define GS_ALIGN_PTR(ptr, alignment)\
                    {\
                      int modulo = ((GS_PTR_NUMERIC_TYPE)ptr & (alignment-1));/* this only works for power of two alignments*/\
                      if(modulo != 0)\
                      {\
                        ptr = (void*)((GS_PTR_NUMERIC_TYPE)ptr + (alignment-modulo));\
                      }\
                    }

////////////////////////////////////////////////
////////////////// ALLOC ///////////////////////
////////////////////////////////////////////////

// Represents an allocation. It is like an optional.
typedef struct GSAlloc 
{
  bool  checked;
  void* ptr;
} GSAlloc;

// Checks if the allocation is NULL and sets the alloc as checked
bool
gs_alloc_is_null(GSAlloc* alloc); // The alloc to check NULL for

// Gets the pointer of an alloc. Throws an assert if not checked
void* 
gs_alloc_ptr(GSAlloc* alloc);     // The alloc to get the ptr from

////////////////////////////////////////////////
////////////////// STACK ///////////////////////
////////////////////////////////////////////////

#define GS_STACK_PUSH(stack, size)\
                gs_stack_push(stack,\
                               size,\
                               GS_MIN_ALIGNMENT)

#define GS_STACK_PUSH_CHECKED(stack, size)\
                gs_stack_push_CHECKED(stack,\
                                       size,\
                                       GS_MIN_ALIGNMENT)

#define GS_STACK_PUSH_ALIGNED(stack, size, alignment)\
                gs_stack_push(stack,\
                              size,\
                              alignment)

#define GS_STACK_PUSH_ALIGNED_CHECKED(stack, size, alignment)\
                gs_stack_push_CHECKED(stack,\
                                       size,\
                                       alignment)

#define GS_STACK_PUSH_ALL(stack, size)\
                gs_stack_push_all(stack,\
                                   GS_MIN_ALIGNMENT,\
                                   size)

#define GS_STACK_PUSH_ALL_CHECKED(stack, size)\
                gs_stack_push_all_CHECKED(stack,\
                                          GS_MIN_ALIGNMENT,\
                                          size)

#define GS_STACK_POP(stack, ptr)\
                gs_stack_pop(stack, ptr)

#define GS_STACK_CHECKPOINT(_stack)\
                *(_stack)

#define GS_STACK_RESTORE(_stack, _checkpoint)\
                *(_stack) = _checkpoint

#define GS_STACK_FLUSH(_stack)\
                gs_stack_flush(_stack)


typedef struct GSStack 
{
  bool              valid;
  void*             p_begin;
  void*             p_end;
  void*             p_current;
} GSStack;

typedef GSStack GSStackCheckpoint;

//Returns a new initialized stack allocator. If the operation fails the returned
//stack is not marked as valid
GSStack
gs_stack_init(void* mem_ptr,                                                    // The pointer to the memory region for the stack
              unsigned long long size);                                         // The size of the memory region



//Flushes the stack mem alloc
void
gs_stack_flush(GSStack* stack);                                                 // stack The stack mem alloc to flush 



// Requests a new memory block from the stack memory allocator. The alloc is
// NULL if the requested block cannot be allocated
GSAlloc
gs_stack_push(GSStack* stack,                                                   // The stack memory allocator to request the address from
              unsigned long long size,                                          // The size to reques
              unsigned int alignment);                                          // The alignment of the requested address



// Requests a new memory block from the stack memory allocator. This a CHECKED
// operation, thus it will throw an assert if the allocation fails (the returned
// pointer is NULL) unless GS_MEM_ALLOC_DISABLE_CHECKS is defined
void*
gs_stack_push_CHECKED(GSStack* stack,                                           // The stack to allocate from
                      unsigned long long size,                                  // The size of the allocation
                      unsigned int alignment);                                  // The alignment of the allocation



// Requests all the remaining memory from the stack. The alloc is
// NULL if the requested block cannot be allocated
GSAlloc
gs_stack_push_all(GSStack* stack,                                               // The stack to allocate from
                  unsigned int alignment,                                       // The alignment of the allocation
                  unsigned long long* size);                                    // The size of the returned allocation



// Requests all the remaiing memory from the stack. This a CHECKED
// operation, thus it will throw an assert if the allocation fails (the returned
// pointer is NULL) unless GS_MEM_ALLOC_DISABLE_CHECKS is defined
void*
gs_stack_push_all_CHECKED(GSStack* stack,                                       // The stack to allocate from
                          unsigned int alignment,                               // The alignment of the allocation
                          unsigned long long* size);                            // The size of the returned allocation



// Pops the last allocation from the stack. The ptr to the allocation is passed 
// for correctness. If GS_MEM_ALLOC_DISABLE_ASSERTS is not defined, 
// the implementation will check that ptr is actually the allocation at the top and
// throw an assert if this is not the case.
void
gs_stack_pop(GSStack* stack,                                                    // The stack memory allocator to pop from
             void* ptr);                                                        // The start address region expected to pop, passed for correctness checks.


////////////////////////////////////////////////
/////////////////// SCRATCH ////////////////////
////////////////////////////////////////////////

#define GS_SCRATCH_PUSH(scratch, size)\
          gs_scratch_push(scratch, size, GS_MIN_ALIGNMENT)

#define GS_SCRATCH_PUSH_CHECKED(scratch, size)\
          gs_scratch_push_CHECKED(scratch, size, GS_MIN_ALIGNMENT)

#define GS_SCRATCH_PUSH_ALIGNED(scratch, size, alignment)\
          gs_scratch_push(scratch, size, alignment)

#define GS_SCRATCH_PUSH_ALIGNED_CHECKED(scratch, size, alignment)\
          gs_scratch_push_CHECKED(scratch, size, alignment)

#define GS_SCRATCH_PUSH_ALL(scratch, allocated)\
          gs_scratch_push_all(scratch, GS_MIN_ALIGNMENT, allocated)

#define GS_SCRATCH_PUSH_ALL_CHECKED(scratch, allocated)\
          gs_scratch_push_all_CHECKED(scratch, GS_MIN_ALIGNMENT, allocated)

#define GS_SCRATCH_CHECKPOINT(_scratch)\
          *(_scratch)

#define GS_SCRATCH_RESTORE(_scratch, _checkpoint)\
          {\
          *(_scratch) = _checkpoint;\
          }

#define GS_SCRATCH_FLUSH(_scratch)\
         gs_scratch_flush(_scratch)

typedef struct GSScratch
{
  bool valid;
  void* p_begin; 
  void* p_current; 
  void* p_end; 
} GSScratch;

typedef GSScratch GSScratchCheckpoint;

 // Returns a new initialized scratch maked valid if the operation succeeds
GSScratch
gs_scratch_init(void* base_addr,                                                // base_addr The base address of the allocator
                unsigned long long size);                                       // The size of the allocator



// Returns a new memory block from the scratch. The alloc is
// NULL if the requested block cannot be allocated
GSAlloc
gs_scratch_push(GSScratch* scratch,                                             // The memory allocator to allocate from
                 unsigned long long  size,                                      // The size to allocate
                 unsigned int alignment);                                       // The requested alignment of the allocation



// Retursn a new memory block from the scratch. This a CHECKED
// operation, thus it will throw an assert if the allocation fails (the returned
// pointer is NULL) unless GS_MEM_ALLOC_DISABLE_CHECKS is defined
void*
gs_scratch_push_CHECKED(GSScratch* scratch,                                     // The memory allocator to allocate from
                         unsigned long long  size,                              // The size to allocate                      
                         unsigned int alignment);                               // The requested alignment of the allocation



// Returns all the remianing memory in the scratch. The alloc is
// NULL if the requested block cannot be allocated
GSAlloc
gs_scratch_push_all(GSScratch* scratch,                                         // The mem alloc to allocate to
                     unsigned int alignment,                                    // The alignment of the requested allocation
                     unsigned long long * size);                                // The allocated size 



// Returns all the remianing memory in the scratch. This a CHECKED
// operation, thus it will throw an assert if the allocation fails (the returned
// pointer is NULL) unless GS_MEM_ALLOC_DISABLE_CHECKS is defined
void*
gs_scratch_push_all_CHECKED(GSScratch* scratch,                                 // The mem alloc to allocate to
                             unsigned int alignment,                            // The alignment of the requested allocation  
                             unsigned long long * size);                        // The allocated size 



// Flushes the scratch memory allocator
void
gs_scratch_flush(GSScratch* scratch);                                            // The scratch to flush


////////////////////////////////////////////////
/////////////////// POOL ///////////////////////
////////////////////////////////////////////////

#define GS_POOL_ALLOC_ALIGNED(pool, size, alignment)\
    gs_pool_alloc(pool,\
                  size,\
                  alignment);

#define GS_POOL_ALLOC_ALIGNED_CHECKED(pool, size, alignment)\
    gs_pool_alloc_CHECKED(pool,\
                          size,\
                          alignment);

#define GS_POOL_FREE(pool, ptr)\
    gs_pool_free(pool, ptr);

#define GS_POOL_FLUSH(pool)\
    gs_pool_flush(pool);


typedef struct GSPool 
{
  bool              valid;
  void*             p_begin;
  void*             p_end;
  void*             p_current;
  void*             p_next_free;
  unsigned int      bsize;
  unsigned int      alignment;
  unsigned int      stride;
} GSPool;

 // Returns a new initialized pool maked valid if the operation succeeds
GSPool
gs_pool_init(void* mem_ptr,                                                     // The pointer to the starting address for the pool
             unsigned long long size,                                           // The size of the pool in bytes
             unsigned long long bsize,                                          // The size of the blocks to be allocated
             unsigned int alignment);                                           // The alignment of the blocks to be allocated



// Flushes the memory allocator
void
gs_pool_flush(GSPool* pool);



// Returns a new block of memory from the pool. The returned block size is that
// specified during the pool initialization. The size and alignment
// parameters are used for checking the usage correctness. The alloc is NULL if 
// there is not enough space in the pool
GSAlloc
gs_pool_alloc(GSPool* pool,                                                     // The pool mem alloc to use
              unsigned long long size,                                          // The size of the memory block (used for debugging purposes)
              unsigned int alignment);                                          // The alignment of the memory block (used for debugging purposes)



// Returns a new block of memory from the pool. The returned block size is that
// specified during the pool initialization. The size and alignment
// parameters are used for checking the usage correctness. This a CHECKED
// operation, thus it will throw an assert if the allocation fails (the returned
// pointer is NULL) unless GS_MEM_ALLOC_DISABLE_CHECKS is defined
void*
gs_pool_alloc_CHECKED(GSPool* pool,                                             // The pool mem alloc to use
                      unsigned long long size,                                  // The size of the memory block (used for debugging purposes)
                      unsigned int alignment);                                  // The alignment of the memory block (used for debugging purposes)



// Frees a block allocated with the pool 
void 
gs_pool_free(GSPool* pool,                                                      // The pool mem alloc to use
             void* ptr);                                                        // The address to the block to deallocate


#ifdef __cplusplus
}
#endif
#endif

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#ifdef GS_MEM_ALLOC_IMPLEMENTATION

#ifndef GS_MEM_ALLOC_DISABLE_ASSERTS
#ifndef GS_MEM_ALLOC_DISABLE_CHECKS
#include <signal.h>
#include <stdio.h>
#endif
#endif

#ifdef GS_MEM_ALLOC_INITIALIZE_TO_ZERO
#include <string.h>
#endif

#ifdef GS_MEM_ALLOC_DISABLE_ASSERTS
#define GS_ASSERT(_cond)
#else
#define GS_ASSERT(_cond) \
{\
  if(!(_cond)) \
  {\
    printf("%s\n", #_cond);\
    raise(SIGABRT);\
  }\
}
#endif

#define GS_PERMA_ASSERT(_cond) \
{\
  if(!(_cond)) \
  {\
    printf("%s\n", #_cond);\
    raise(SIGABRT);\
  }\
}

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////
/////////////////// ALLOC //////////////////////
////////////////////////////////////////////////
bool
gs_alloc_is_null(GSAlloc* alloc)
{
  alloc->checked = true;
  return alloc->ptr == NULL;
}

void* 
gs_alloc_ptr(GSAlloc* alloc)
{
  GS_ASSERT(alloc->checked && "GSAlloc cannot get ptr of an unchecked alloc");
  return alloc->ptr;
}

////////////////////////////////////////////////
////////////////// STACK ///////////////////////
////////////////////////////////////////////////

GSStack
gs_stack_init(void* mem_ptr, 
              unsigned long long size)
{
  GS_ASSERT(mem_ptr != NULL && 
            "GSStack mem ptr cannot be NULL")
  GSStack stack; 
  stack.p_begin = mem_ptr;
  stack.p_current = mem_ptr; 
  stack.p_end = ((char*)mem_ptr)+size;
  stack.valid = true;
  return stack;
}

GSAlloc
gs_stack_push(GSStack* stack, 
                        unsigned long long size,
                        unsigned int alignment)
{
  GS_ASSERT(stack->valid == true && 
            "GSStack cannot push an invalid stack mem alloc")

  void* ret = stack->p_current;
  GS_ALIGN_PTR(ret, alignment);

  GS_ASSERT(((unsigned long long )ret) % alignment == 0 && 
            "GSStack has a bug at computing a properly aligned address")


  char* new_current = ret+size;
  GS_ALIGN_PTR(new_current, GS_PTR_ALIGNMENT);
  new_current+=GS_PTR_ALIGNMENT;

  if(new_current >= (char*)stack->p_end)
  {
    GSAlloc alloc;
    alloc.ptr = NULL;
    alloc.checked = false;
    return alloc; 
  }

  GS_ASSERT(stack->p_current && "GSStack previous base cannot be set to NULL");
  *(GS_PTR_NUMERIC_TYPE*)(new_current - GS_PTR_ALIGNMENT) = (GS_PTR_NUMERIC_TYPE)stack->p_current;
  stack->p_current = new_current; 

#ifdef GS_MEM_ALLOC_INITIALIZE_TO_ZERO
  memset(ret, 0, size);
#endif
  GSAlloc alloc;
  alloc.ptr = ret;
  alloc.checked = false;
  return alloc;
}

void*
gs_stack_push_CHECKED(GSStack* stack, 
                        unsigned long long size,
                        unsigned int alignment)
{
  GSAlloc alloc = gs_stack_push(stack, size, alignment);
#ifndef GS_MEM_ALLOC_DISABLE_CHECKS
  GS_PERMA_ASSERT(!gs_alloc_is_null(&alloc));
#else
  alloc.checked = true;
#endif
  return gs_alloc_ptr(&alloc);
}

GSAlloc
gs_stack_push_all(GSStack* stack, 
                            unsigned int alignment,
                            unsigned long long* size)
{

  GS_ASSERT(stack->valid == true && 
            "GSStack cannot push an invalid stack mem alloc")

  void* ret = stack->p_current;
  GS_ALIGN_PTR(ret, alignment)

  GS_ASSERT(((unsigned long long )ret) % alignment == 0 && 
            "GSStack has a bug at computing a properly aligned address")

  char* new_current = stack->p_end;
  new_current -= GS_PTR_ALIGNMENT;

  // We need to ensure that the previous base address is aligned to
  // GS_PTR_ALINGMENT
  int prev_base_slack = ((GS_PTR_NUMERIC_TYPE)new_current % GS_PTR_ALIGNMENT);
  if(prev_base_slack != GS_PTR_ALIGNMENT)
  {
    new_current -= prev_base_slack;
  }

  if(new_current >= (char*)stack->p_end)
  {
    GSAlloc alloc;
    alloc.ptr = NULL;
    alloc.checked = false;
    return alloc;
  }

  GS_ASSERT(stack->p_current && "GSStack previous base cannot be set to NULL");
  *(GS_PTR_NUMERIC_TYPE*)(new_current) = (GS_PTR_NUMERIC_TYPE)stack->p_current;
  *size = GS_PTR_DIFF(new_current, ret);
  stack->p_current = new_current + GS_PTR_ALIGNMENT; 

#ifdef GS_MEM_ALLOC_INITIALIZE_TO_ZERO
  memset(ret, 0, *size);
#endif
  GSAlloc alloc;
  alloc.ptr = ret;
  alloc.checked = false;
  return alloc;
}

void*
gs_stack_push_all_CHECKED(GSStack* stack, 
                            unsigned int alignment,
                            unsigned long long* size)
{
  GSAlloc alloc = gs_stack_push_all(stack, alignment, size);
#ifndef GS_MEM_ALLOC_DISABLE_CHECKS
  GS_PERMA_ASSERT(!gs_alloc_is_null(&alloc));
#else
  alloc.checked = true;
#endif
  return gs_alloc_ptr(&alloc);
}

void
gs_stack_pop(GSStack* stack, 
                   void* ptr)
{
  void* prev_stack_base = stack->p_begin;
  if(stack->p_current != stack->p_begin)
  {
    prev_stack_base = (void*)(*(unsigned long long *)(((char*)stack->p_current) - GS_PTR_ALIGNMENT));
  }
  
  GS_ASSERT(prev_stack_base <= ptr && 
            "GSStack cannot pop from this address.\
            Popping must be performed in reverse order of push");
  GS_ASSERT(prev_stack_base && "Stack previous memory address cannot be null");

  stack->p_current = prev_stack_base;
}

void
gs_stack_flush(GSStack* stack)
{
  GS_ASSERT(stack->valid == true && 
            "GSStack cannot flush an invalid stack mem alloc")
  stack->p_current = stack->p_begin;
}

////////////////////////////////////////////////
////////////////// SCRATCH  ////////////////////
////////////////////////////////////////////////

GSScratch
gs_scratch_init(void* base_addr, 
                unsigned long long size)
{
  GS_ASSERT(base_addr != NULL && 
            "GSScratch base addr cannot be NULL")
  GS_ASSERT(((unsigned long long)base_addr % GS_MIN_ALIGNMENT) == 0 && 
            "GSScratch mem ptr must be aligned to GS_MIN_ALINGMENT")

  GSScratch scratch;
  scratch.p_begin = base_addr; 
  scratch.p_current = base_addr;
  scratch.p_end = (char*)base_addr + size;
  scratch.valid = true;
  return scratch;
}

GSAlloc
gs_scratch_push(GSScratch* scratch, 
                 unsigned long long size, 
                 unsigned int alignment)
{
  GS_ASSERT(scratch->valid && "GSScratch not properly initialized")
  void* ret = scratch->p_current;
  GS_ALIGN_PTR(ret, alignment)

  GS_ASSERT(((GS_PTR_NUMERIC_TYPE)ret) % alignment == 0 && 
            "GSScrachMemAlloc aligned address is not correclty computed")

  char* new_current = ((char*)ret) + size;
  if(new_current >= (char*)scratch->p_end)
  {
    GSAlloc alloc;
    alloc.ptr = NULL;
    alloc.checked = false;
    return alloc;
  }
  scratch->p_current = new_current;

#ifdef GS_MEM_ALLOC_INITIALIZE_TO_ZERO
  memset(ret, 0, size);
#endif
  GSAlloc alloc;
  alloc.ptr = ret;
  alloc.checked = false;
  return alloc;
}

void*
gs_scratch_push_CHECKED(GSScratch* scratch, 
                         unsigned long long size, 
                         unsigned int alignment)
{
  GSAlloc alloc = gs_scratch_push(scratch, size, alignment);
#ifndef GS_MEM_ALLOC_DISABLE_CHECKS
  GS_PERMA_ASSERT(!gs_alloc_is_null(&alloc));
#else
  alloc.checked = true;
#endif
  return gs_alloc_ptr(&alloc);
}

GSAlloc
gs_scratch_push_all(GSScratch* scratch, 
                     unsigned int alignment,
                     unsigned long long* size)
{
  GS_ASSERT(scratch->valid && "GSScratch not properly initialized")
  void* ret = scratch->p_current;
  GS_ALIGN_PTR(ret, alignment);

  GS_ASSERT(((GS_PTR_NUMERIC_TYPE)ret) % alignment == 0 && 
            "GSScrachMemAlloc aligned address is not correclty computed")

  if((char*)ret >= (char*)scratch->p_end)
  {
    GSAlloc alloc;
    alloc.ptr = NULL;
    alloc.checked = false;
    return alloc;
  }

  *size = ((GS_PTR_NUMERIC_TYPE)scratch->p_end) - ((GS_PTR_NUMERIC_TYPE)ret);
  scratch->p_current = scratch->p_end;

#ifdef GS_MEM_ALLOC_INITIALIZE_TO_ZERO
  memset(ret, 0, *size);
#endif
  GSAlloc alloc;
  alloc.ptr = ret;
  alloc.checked = false;
  return alloc;
}

void*
gs_scratch_push_all_CHECKED(GSScratch* scratch, 
                             unsigned int alignment,
                             unsigned long long* size)
{
  GSAlloc alloc = gs_scratch_push_all(scratch, alignment, size);
#ifndef GS_MEM_ALLOC_DISABLE_CHECKS
  GS_PERMA_ASSERT(!gs_alloc_is_null(&alloc));
#else 
  alloc.checked = true;
#endif
  return gs_alloc_ptr(&alloc);
}

void
gs_scratch_flush(GSScratch* scratch)
{
  GS_ASSERT(scratch->valid && "GSScratch not properly initialized")
  scratch->p_current = scratch->p_begin;
}


////////////////////////////////////////////////
/////////////////// POOL ///////////////////////
////////////////////////////////////////////////


GSPool
gs_pool_init(void* mem_ptr, 
                       unsigned long long size, 
                       unsigned long long bsize, 
                       unsigned int alignment)
{
  GS_ASSERT(mem_ptr != NULL && 
            "GSPool mem ptr cannot be NULL")

  GSPool pool;
  pool.p_begin = mem_ptr;
  pool.p_end = (char*)mem_ptr + size; 
  pool.bsize = bsize; 
  pool.alignment = alignment;
  pool.p_next_free = NULL;

  if(pool.bsize < sizeof(GS_PTR_NUMERIC_TYPE))
  {
    pool.bsize = sizeof(GS_PTR_NUMERIC_TYPE);
  }
  GS_ALIGN_PTR(pool.p_begin, alignment)

  pool.p_current = pool.p_begin;
  pool.stride = bsize;
  unsigned int modulo = bsize & (alignment-1);
  if(modulo != 0)
  {
    pool.stride += alignment - modulo;
  }
  pool.valid = true;
  return pool;
}


void
gs_pool_flush(GSPool* pool)
{
  GS_ASSERT(pool->valid == true && 
            "GSPool cannot flush an invalid pool mem alloc")
  pool->p_current = pool->p_begin;
}


GSAlloc
gs_pool_alloc(GSPool* pool, 
              unsigned long long size, 
              unsigned int alignment)
{
  GS_ASSERT(pool->valid == true && 
            "GSPool cannot allocate from an invalid pool mem alloc")
  GS_ASSERT(pool->alignment == alignment && 
            "GSPool incompatible alignment in allocation ")
  GS_ASSERT(pool->bsize == size || 
            (size < sizeof(GS_PTR_NUMERIC_TYPE) && pool->bsize == sizeof(GS_PTR_NUMERIC_TYPE)) && 
            "GSPool incompatible size in allocation")

  char* ret = NULL;
  if(pool->p_next_free != NULL)
  {
    void* next_free = (void*)*(GS_PTR_NUMERIC_TYPE*)pool->p_next_free;
    ret = pool->p_next_free;
    pool->p_next_free = next_free;
  }
  else
  {
    ret = pool->p_current;
    pool->p_current = (char*)pool->p_current + pool->stride;
    GS_ASSERT((GS_PTR_NUMERIC_TYPE)pool->p_current % alignment == 0)
  }

  if(((unsigned long long)ret) % alignment != 0 )
  {
    GS_ASSERT(((unsigned long long)ret) % alignment == 0 && 
            "GSPool has a bug at computing a properly aligned address")
  }

  if((void*)(ret + size) >= pool->p_end)
  {
    GSAlloc alloc;
    alloc.ptr = NULL;
    alloc.checked = false;
    return alloc;
  }

#ifdef GS_MEM_ALLOC_INITIALIZE_TO_ZERO
  memset(ret, 0, pool->bsize);
#endif

  GSAlloc alloc;
  alloc.ptr = ret;
  alloc.checked = false;
  return alloc;
}

void*
gs_pool_alloc_CHECKED(GSPool* pool, 
              unsigned long long size, 
              unsigned int alignment)
{
  GSAlloc alloc = gs_pool_alloc(pool, size, alignment);
#ifndef GS_MEM_ALLOC_DISABLE_CHECKS
  GS_PERMA_ASSERT(!gs_alloc_is_null(&alloc));
#else
  alloc.checked = true;
#endif
  return gs_alloc_ptr(&alloc);
}


void 
gs_pool_free(GSPool* pool, 
             void* ptr)
{
  GS_ASSERT(pool->valid == true && 
            "GSPool cannot free from an invalid pool mem alloc")
  GS_ASSERT(((GS_PTR_NUMERIC_TYPE)ptr % pool->alignment == 0) && "GSPool this should not happen")
  GS_ASSERT(((GS_PTR_NUMERIC_TYPE)ptr >= (GS_PTR_NUMERIC_TYPE)pool->p_begin && (GS_PTR_NUMERIC_TYPE)ptr < (GS_PTR_NUMERIC_TYPE)pool->p_current) && "GSPool invalid freed ptr")

  *(GS_PTR_NUMERIC_TYPE*)ptr = (GS_PTR_NUMERIC_TYPE)pool->p_next_free;
  pool->p_next_free = ptr;
}

#ifdef __cplusplus
}
#endif

#endif
