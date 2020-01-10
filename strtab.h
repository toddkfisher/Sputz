#pragma once

#define N_SLOTS 59093
#define MAX_STRINGS 2048
#define STR_AREA_SIZE 1048576

typestruct(STRREC);
struct STRREC {
  char *sr_string;
  STRREC *sr_next;
};

typestruct(STRTAB);
struct STRTAB {
  STRREC *st_htab[N_SLOTS];
  uint32_t st_next_slot;
  STRREC st_slots[N_SLOTS];
  uint32_t st_next_char;
  char st_strings[STR_AREA_SIZE];
};
