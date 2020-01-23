#include "sputz-includes.h"

// This is a simple-minded stack-based memory allocator.  Deallocation takes place over the entire
// memory arena or down to a certain location.  It was written to be used with a parser where a failed
// forces freeing an parse tree or branch thereof.

ARENA *stkalloc_new_arena(size_t arena_size)
{
  ARENA *retval;
  if (NULL != (retval = MALLOC_1(ARENA))) {
    retval->ar_size = arena_size;
    if (NULL != (retval->ar_pbase = malloc(arena_size))) {
      retval->ar_pnext = retval->ar_pbase;
      zero_mem(retval->ar_pbase, arena_size);
      retval->ar_plast = retval->ar_pbase + arena_size - 1;
      retval->ar_n_allocations = 0;
      retval->ar_n_checkpoints = 0;
      retval->ar_n_rollbacks = 0;
    } else {
      free(retval);
      retval = NULL;
    }
  }
  return retval;
}

void *stkalloc_get_checkpoint(ARENA *par)
{
  par->ar_n_checkpoints += 1;
  return par->ar_pnext;
}

bool stkalloc_rollback(ARENA *par, void *chkpt)
{
  bool retval = false;
  if (chkpt >= par->ar_pbase &&
      chkpt <= par->ar_plast &&
      chkpt < par->ar_pnext) {
    par->ar_n_rollbacks += 1;
    par->ar_pnext = chkpt;
    retval = true;
  }
  return retval;
}

void *stkalloc_get_mem(ARENA *par, size_t mem_size)
{
  void *retval = NULL;
#if 0
  DBG_PRINT_VAR(((long) par->ar_pnext), LONG);
  DBG_PRINT_VAR(((long) mem_size), LONG);
  DBG_PRINT_VAR(((long) par->ar_plast), LONG);
  DBG_PRINT_VAR(((long) par->ar_plast - (long) par->ar_pbase), LONG);
  DBG_PRINT_VAR(((long) par->ar_pnext + mem_size), LONG);
  assert(par->ar_pnext + mem_size <= par->ar_plast);
#endif
  if (par->ar_pnext + mem_size <= par->ar_plast) {
    retval = par->ar_pnext;
    par->ar_pnext += mem_size;
    par->ar_n_allocations += 1;
  }
  return retval;
}

void *stkalloc_free_arena(ARENA *par)
{
  if (NULL != par->ar_pbase) {
    free(par->ar_pbase);
    free(par);
  }
}

void stkalloc_print_stats(ARENA *par)
{
  printf("ARENA: %p {\n", par);
  printf("           ar_size == %lu\n", par->ar_size);
  printf("          ar_pbase == %p\n", par->ar_pbase);
  printf("          ar_pnext == %p\n", par->ar_pnext);
  printf("          ar_plast == %p\n", par->ar_plast);
  printf("  ar_n_allocations == %u\n", par->ar_n_allocations);
  printf("  ar_n_checkpoints == %u\n", par->ar_n_checkpoints);
  printf("    ar_n_rollbacks == %u\n", par->ar_n_rollbacks);
  printf("}\n");
}

#if defined(STACKALLOC_TEST)

#define TEST_ARENA_SIZE 64

void stkalloc_test(void)
{
  ARENA *par = stkalloc_new_arena(TEST_ARENA_SIZE);
  void *p;
  void *chkpt;
  // Arena allocated?
  T(NULL != par);
  // Size set correctly?
  T(TEST_ARENA_SIZE == par->ar_size);
  // Next block to be allocated begins at the base?
  T(par->ar_pbase == par->ar_pnext);
  // Able to allocate 16 bytes?
  T(NULL != (p = stkalloc_get_mem(par, 16)));
  T(par->ar_pbase + 16 == par->ar_pnext);
  T(NULL == (p = stkalloc_get_mem(par, 1024)));
  T(par->ar_pbase + 16 == par->ar_pnext);
  T(NULL != (chkpt = stkalloc_get_checkpoint(par)));
  T(par->ar_pbase + 16 == par->ar_pnext);
  T(NULL != (p = stkalloc_get_mem(par, 16)));
  T(par->ar_pbase + 32 == par->ar_pnext);
  T(stkalloc_rollback(par, chkpt));
  T(par->ar_pnext == chkpt);
  stkalloc_print_stats(par);
  stkalloc_free_arena(par);
}

int main(int argc, char **argv)
{
  stkalloc_test();
  return 0;
}

#endif
