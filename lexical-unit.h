#pragma once

#include "enum-int.h"
enum {
#include "enum-L.h"
};

typestruct(LEX_UNIT);
struct LEX_UNIT {
  uint8_t lx_type;
  // line/col position of lexical unit in input source.
  uint32_t lx_line_n;
  uint32_t lx_col_n;
  union {
    // L_NUMBER
    double lx_number;
    // L_VAR_NAME
    char *lx_pvar_name;
    // L_SYMBOL
    char *lx_psym_name;
  };
};
