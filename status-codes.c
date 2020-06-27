#include "sputz-includes.h"

#include "enum-str.h"
static char *g_status_code_names[] = {
#include "enum-status-codes.h"
};



char *scode_name(
  TAGGED_ENUM code
)
{
  char *result = "UNKNOWN_STATUS_CODE";
  uint32_t code_ordinal = GET_ORDINAL(code);
  if (code_ordinal < sizeof(g_status_code_names)/sizeof(char *)) {
    result = g_status_code_names[code_ordinal];
  }
  return result;
}



bool scode_is_error(
  TAGGED_ENUM code
)
{
  bool result = HAS_ANY_TYPE(code, SC_ERROR);
  return result;
}
