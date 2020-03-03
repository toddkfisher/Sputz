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
    result->st_n_strings_inserted = 0;
  }
  return result;
}



uint32_t strtab_hash(char *s)
{
  uint64_t n = 0;
  while (*s) {
    n += *s++;
  }
  return n%N_SLOTS;
}



STRREC *strtab_new_rec(STRTAB *st)
{
  STRREC *result = NULL;
  if (st->st_next_rec_idx < MAX_RECS) {
    result = &st->st_slots[st->st_next_rec_idx++];
  }
  return result;
}



char *strtab_insert_string(STRTAB *st, char *str)
{
  uint32_t len;
  char *result = NULL;
  len = strlen(str);
  if (st->st_next_char + len < STR_AREA_SIZE) {
    result = st->st_strings + st->st_next_char;
    strcpy(result, str);
    st->st_next_char += len + 1;
  }
  return result;
}



char *strtab_exists(STRTAB *st, char *s)
{
  uint32_t h;
  char *result = NULL;
  STRREC *prec;
  result = false;
  h = strtab_hash(s);
  for (prec = st->st_htab[h]; !result && NULL != prec; prec = prec->sr_next) {
    if (STREQ(s, prec->sr_string)) {
      result = prec->sr_string;
    }
  }
  return result;
}



uint8_t strtab_insert(STRTAB *st, char *s, char **ppstr)
{
  uint32_t h = strtab_hash(s);
  STRREC *pnewrec;
  STRREC *prec;
  uint32_t result;
  result = ST_UNABLE_TO_INSERT;
  if (NULL == (*ppstr = strtab_exists(st, s))) {
    ABORT_ON_NULL(pnewrec = strtab_new_rec(st), MEM_OVERFLOW0);
    ABORT_ON_NULL(*ppstr = strtab_insert_string(st, s), MEM_OVERFLOW1);
    pnewrec->sr_string = *ppstr;
    pnewrec->sr_next = st->st_htab[h];
    st->st_htab[h] = pnewrec;
    result = ST_NEW_STR_INSERTED;
    st->st_n_strings_inserted += 1;
  } else {
    result = ST_STR_EXISTS;
  }
  return result;
  MEM_OVERFLOW0: {
    return ST_UNABLE_TO_INSERT;
  }
  MEM_OVERFLOW1: {
    st->st_next_rec_idx -= 1;
    return ST_UNABLE_TO_INSERT;
  }
}



void strtab_print_stats(STRTAB *pstab)
{
  uint32_t avg_slot_count = 0;
  uint32_t n_filled_slots = 0;
  printf("STRTAB {\n");
  printf("           st_next_char    == %u\n", pstab->st_next_char);
  printf("           st_next_rec_idx == %u\n", pstab->st_next_rec_idx);
  for (uint32_t i = 0; i < N_SLOTS; ++i) {
    if (NULL != pstab->st_htab[i]) {
      n_filled_slots += 1;
    }
  }
  printf("         n_filled_slots    == %u\n", n_filled_slots > 0 ? n_filled_slots : 0);
  printf("         avg slot count    == %lf\n", n_filled_slots > 0 ? ((double) pstab->st_next_rec_idx)/((double) n_filled_slots) : 0);
  printf("         (over filled slots)\n");
  printf("         avg slot count    == %lf\n", n_filled_slots > 0 ? ((double) N_SLOTS)/((double) n_filled_slots) : 0);
  printf("         (over all slots)\n");
  printf("  st_n_strings_inserted    == %u\n", pstab->st_n_strings_inserted);
  printf("}\n");
}



#if defined(TEST_STRTAB)

char gen_rand_visible_char(void)
{
  int i;
  char result = '\0';
  result = rand()%(126 - 32 + 1) + 32;
  return result;
}



char *gen_rand_str(void)
{
  int i;
  int n = rand()%(MAX_STR - 1) + 1;
  static char result[MAX_STR];
  zero_mem(result, MAX_STR);
  for (i = 0; i < n; ++i) {
    result[i] = gen_rand_visible_char();
  }
  return result;
}



#define N_STRINGS 5000

char str_list[N_STRINGS][MAX_STR];
char *inserted_pointer_list[N_STRINGS];



int main(int argc, char **argv)
{
  int i;
  ARENA *par;
  STRTAB *pstab;
  uint32_t retcode;
  if (NULL == (par = stkalloc_new_arena(MIB(10)))) {
    fprintf(stderr, "Unable to create 1 Mib arena.\n");
    return 1;
  }
  if (NULL == (pstab = strtab_new(par))) {
    fprintf(stderr, "Unable to create new string table.\n");
    return 1;
  }
  srand(time(NULL));
  for (i = 0; i < N_STRINGS; ++i) {
    strcpy(str_list[i], gen_rand_str());
    //printf("Inserting %04d : %s\n", i, str_list[i]);
    if (ST_NEW_STR_INSERTED != (retcode = strtab_insert(pstab, str_list[i], &inserted_pointer_list[i]))) {
      fprintf(stderr, "Unable to insert into string table.  retcode == %s\n", scode_name(retcode));
      return 1;
    } //else {
      //printf("Inserted. retcode == %s\n", scode_name(retcode));
    //}
  }
  for (i = 0; i < N_STRINGS; ++i) {
    //printf("Checking 'insert' : %s\n", str_list[i]);
    if (ST_STR_EXISTS != (retcode = strtab_insert(pstab, str_list[i], &inserted_pointer_list[i]))) {
      fprintf(stderr, "Not inserted properly.  retcode == %s\n", scode_name(retcode));
      return 1;
    } //else {
      //printf("Was inserted. retcode == %s\n", scode_name(retcode));
    //}
  }
  strtab_print_stats(pstab);
  return 0;
}

#endif
