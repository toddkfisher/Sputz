#pragma once

#include "enum-int.h"
enum {
#include "enum-L.h"
};

typestruct(LEX_UNIT);
struct LEX_UNIT {
  uint8_t lex_type;
  // line/col position of lexical unit in input source.
  uint32_t lex_line_n;
  uint32_t lex_col_n;
  union {
    // L_NUMBER
    double lex_number;
    // L_VAR_NAME
    char *lex_pvar_name;
    // L_SYMBOL
    char *lex_psym_name;
  };
};
