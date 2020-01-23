#pragma once

typestruct(ARENA);
struct ARENA {
  size_t ar_size;
  void *ar_pbase;
  void *ar_pnext;
  void *ar_plast;
  uint32_t ar_n_allocations;
  uint32_t ar_n_checkpoints;
  uint32_t ar_n_rollbacks;
};
