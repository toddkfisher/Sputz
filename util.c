#include "sputz-includes.h"

void zero_mem(
  void *p,
  size_t n
  )
{
  explicit_bzero(p, n);
}



void print_summary_string(
  char *s,
  size_t length
  )
{
  char buf[MAX_SUMMARY_LENGTH];
  size_t n = MIN(MAX_SUMMARY_LENGTH, length + 1);
  long slen = strlen(s);
  zero_mem(buf, n);
  if (n) {
    memcpy(buf, s, n);
    printf("%s", buf);
  }
  if (n - 1 < slen) {
    int i;
    for (i = n - 1; i - (n - 1) < 3 && i < slen; ++i) {
      printf(".");
    }
  }
}



static int blanks(
  char *s
  )
{
  while (*s) {
    if (!isspace(*s++)) {
      return 0;
    }
  }
  return 1;
}
