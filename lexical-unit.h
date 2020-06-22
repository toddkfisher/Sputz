#pragma once

// "L" == "Lexical Unit"
// "LC" == "lexical unit class"
// "KW" == "Keyword"

enum {
   LC_UNARY_OP  = 0x01,
   LC_BINARY_OP = 0x02,
   LC_REL_OP    = 0x04,
   LC_ADD_OP    = 0x08,
   LC_MUL_OP    = 0x10
};

#include "enum-int.h"
enum {
#include "enum-L.h"
};

#define IS_OPERATOR_LTYPE(lx_type, op_type) ((L_BEGIN_##lx_type##_OP) < (op_type) && (op_type) < (L_END_##lx_type##_OP))

typestruct(LEX_UNIT);
struct LEX_UNIT {
  TAGGED_ENUM lex_type;
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
