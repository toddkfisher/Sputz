#include "sputz-includes.h"

#include "enum-str.h"
static char *status_code_names[] = {
#include "enum-status-codes.h"
};

char *scode_name(uint32_t code)
{
  char *result = "UNKNOWN_STATUS_CODE";
  if (code < sizeof(status_code_names)/sizeof(char *)) {
    result = status_code_names[code];
  }
  return result;
}

bool scode_is_error(uint32_t code)
{
  bool result = (code >= BEGIN_ERROR_MARKER) && (code < sizeof(status_code_names)/sizeof(char *));
  return result;
}
