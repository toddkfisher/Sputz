#include "sputz-includes.h"

STRTAB *strtab_new(ARENA *pmem)
{
  STRTAB *result = NULL;
  uint32_t i;
  if (NULL != (result = stkalloc_get_mem(pmem, sizeof(STRTAB)))) {
    // Wipe them out - all of them!
    zero_mem(result, sizeof(STRTAB));
    for (i = 0; i < N_SLOTS; ++i) {
      result->st_htab[i] = NULL;
    }
  }
  return result;
}

static uint32_t strtab_hash(STRTAB *st, char *s)
{
  uint32_t n;
  while (*s) {
    n += *s++;
  }
  return n % N_SLOTS;
}

static STRREC *strtab_new_rec(STRTAB *st)
{
  STRREC *result = NULL;
  if (st->st_next_slot < N_SLOTS) {
    result = &st->st_slots[st->st_next_slot++];
  }
  return result;
}

static char *strtab_insert_string(STRTAB *st, char *str)
{
  uint32_t len;
  char *result = NULL;
  len = strlen(str);
  if (st->st_next_slot + len < STR_AREA_SIZE) {
    result = st->st_strings + st->st_next_slot;
    strcpy(result, str);
    st->st_next_slot += len + 1;
  }
  return result;
}

static bool strtab_exists(STRTAB *st, char *s)
{
  uint32_t h;
  bool result;
  STRREC *prec;
  result = false;
  h = strtab_hash(st, s);
  for (prec = st->st_htab[h]; !result && NULL != prec; prec = prec->sr_next) {
    if (STREQ(s, prec->sr_string)) {
      result = true;
    }
  }
  return result;
}


uint8_t strtab_insert(STRTAB *st, char *s, char **ppstr)
{
  uint32_t h;
  STRREC *pnewrec;
  STRREC *prec;
  uint32_t result;
  result = ST_UNABLE_TO_INSERT;
  if (!strtab_exists(st, s)) {
    ABORT_ON_NULL(pnewrec = strtab_new_rec(st), MEM_OVERFLOW0);
    ABORT_ON_NULL(*ppstr = strtab_insert_string(st, s), MEM_OVERFLOW1);
    pnewrec->sr_next = NULL;
    st->st_htab[h] = pnewrec;
    result = ST_NEW_STR_INSERTED;
  } else {
    result = ST_STR_EXISTS;
  }
  return result;
  MEM_OVERFLOW0: {
    return ST_UNABLE_TO_INSERT;
  }
  MEM_OVERFLOW1: {
    st->st_next_slot -= 1;
    return ST_UNABLE_TO_INSERT;
  }
}
