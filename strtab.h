#pragma once

#define MAX_RECS 59093
#define N_SLOTS 3803
#define MAX_STRINGS 2048
#define STR_AREA_SIZE MIB(5)

typestruct(STRREC);
struct STRREC {
  char *sr_string;
  STRREC *sr_next;
};

typestruct(STRTAB);
struct STRTAB {
  STRREC *st_htab[N_SLOTS];
  uint32_t st_next_rec_idx;
  STRREC st_slots[MAX_RECS];
  uint32_t st_next_char;
  char st_strings[STR_AREA_SIZE];
  uint32_t st_n_strings_inserted;
};
