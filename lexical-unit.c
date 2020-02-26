#include "sputz-includes.h"

#include "enum-str.h"
char *g_lex_unit_names[] ={
#include "enum-L.h"
};

char *lx_name(uint32_t lx_type)
{
  char *result = "UNKNOWN_LEXICAL_TYPE";
  if (lx_type < sizeof(g_lex_unit_names)/sizeof(char *)) {
    result = g_lex_unit_names[lx_type];
  }
  return result;
}

void lx_print(LEX_UNIT *plx)
{
  printf("LEX_UNIT {\n");
  printf("  lx_line_n == %u\n", plx->lex_line_n);
  printf("  lx_col_n  == %u\n", plx->lex_col_n);
  printf("  lx_type == %s,\n", g_lex_unit_names[plx->lex_type]);
  switch (plx->lex_type) {
    case L_NUMBER:
      printf("  lx_number == %g,\n", plx->lex_number);
      break;
    case L_VAR_NAME:
      printf("  lx_var_name == \"%s\",\n", plx->lex_pvar_name);
      break;
    case L_SYMBOL:
      printf("  lx_sym == \"%s\",\n", plx->lex_psym_name);
      break;
    default:
      break;
  }
  printf("}\n");
}

void lx_skip_whitespace(GEN_READ *pinput)
{
  char ch = 0;
  for (;gr_get_char(pinput, &ch) && isspace(ch);) {
  }
  if (ch && !gr_eof(pinput)) {
    gr_putback_char(pinput, ch);
  }
}

#define IS_NAME_CHAR(ch) (isalnum(ch) || '_' == (ch) || '?' == (ch))

struct {
  char *kw_name;
  uint8_t kw_type;
} g_keywords[]= {
  "also",    L_ALSO_KW,
  "and",     L_AND_KW,
  "bind",    L_BIND_KW,
  "else",    L_ELSE_KW,
  "if",      L_IF_KW,
  "not",     L_NOT_KW,
  "null",    L_NULL_KW,
  "or",      L_OR_KW,
  "outer",   L_OUTER_KW,
  "then",    L_THEN_KW,
  "valueis", L_VALUE_KW,
  "",        0
};

typedef uint32_t (*LX_SCAN_FN)(GEN_READ *, LEX_UNIT *, STRTAB *);

// '!'  --> L_SUCH_THAT
// '!!' --> L_GUARD
uint32_t lx_scan_exclamation(GEN_READ *pinput, LEX_UNIT *plx, STRTAB *pstrtab)
{
  uint32_t result = LX_SCAN_OK;
  char ch;
  // Skip '!'
  gr_get_char(pinput, &ch);
  if (gr_get_char(pinput, &ch)) {
    if ('!' == ch) {
      plx->lex_type = L_GUARD;
    } else {
      plx->lex_type = L_SUCH_THAT;
      gr_putback_char(pinput, ch);
    }
  }
  return result;
}

// L_NUMBER
// Numeric syntax:
// ['~']<digit>+['.'<digit>*][('e'|'E')['~']<digit>+]
uint32_t lx_scan_number(GEN_READ *pinput, LEX_UNIT *plx, STRTAB *pstrtab)
{
  uint32_t result = LX_SCAN_OK;
  char ch;
  gr_get_char(pinput, &ch);
  if ('~' == ch || isdigit(ch)) {
    int8_t mant_sign = 1;
    int8_t exp_sign = 1;
    int64_t int_part = 0;
    int64_t exp_part = 0;
    double frac_part = 0.0;
    // **SIGN**
    if ('~' == ch) {
      mant_sign = -1;
      // Use a harmless value for ch (to the 'do' loop below) to avoid more
      // complicated code involving gr_get_char().
      // And yes, that means that '~' alone is zero.
      ch = '0';
    }
    // **INTEGER PART**
    do {
      int_part = int_part * 10 + ch - '0';
    } while (gr_get_char(pinput, &ch) && isdigit(ch));
    // **FRACTIONAL PART**
    if (!gr_eof(pinput) && '.' == ch) {
      double d = 10.0;
      while (gr_get_char(pinput, &ch) && isdigit(ch)) {
        frac_part += (ch - '0') / d;
        d *= 10;
      }
    }
    // **EXPONENT**
    if (!gr_eof(pinput) && ('e' == ch || 'E' == ch)) {
      gr_get_char(pinput, &ch);
      if ('~' == ch) {
        exp_sign = -1;
        gr_get_char(pinput, &ch);
      }
      while (!gr_eof(pinput) && isdigit(ch)) {
        exp_part = exp_part * 10 + ch - '0';
        gr_get_char(pinput, &ch);
      }
    }
    if (!gr_eof(pinput)) {
      gr_putback_char(pinput, ch);
    }
    plx->lex_type = L_NUMBER;
    plx->lex_number = mant_sign*(int_part + frac_part)*pow(10.0, exp_sign*exp_part);
    result = LX_SCAN_OK;
  }
  return result;
}

// ':=' --> L_ASSIGN
uint32_t lx_scan_assign(GEN_READ *pinput, LEX_UNIT *plx, STRTAB *pstrtab)
{
  uint32_t result = LX_SCAN_OK;
  char ch;
  // skip over ':'
  gr_get_char(pinput, &ch);
  if (gr_eof(pinput) || !gr_get_char(pinput, &ch)) {
    result = LX_UNKNOWN_CHAR;
  } else if ('=' == ch) {
    plx->lex_type = L_ASSIGN;
  } else {
    result = LX_UNKNOWN_CHAR;
    gr_putback_char(pinput, ch);
  }
  return result;
}

// '>'  --> L_GT
// '>=' --> L_GE
// '<'  --> L_LT
// '<=' --> L_LE
// '='  --> L_EQ
// '<>' --> L_NE
uint32_t lx_scan_relop(GEN_READ *pinput, LEX_UNIT *plx, STRTAB *pstrtab)
{
  uint32_t result = LX_SCAN_OK;
  char ch;
  char next_ch;
  gr_get_char(pinput, &ch);
  switch (ch) {
    case '>':
    case '<':
      if (gr_eof(pinput) || !gr_get_char(pinput, &next_ch)) {
        plx->lex_type = ('>' == ch ? L_GT : L_LT);
      } else if ('=' == next_ch) {
        // '>=' || '<='
        plx->lex_type = ('>' == ch ? L_GE : L_LE);
      } else if ('<' == ch && '>' == next_ch) {
        // '<>'
        plx->lex_type = L_NE;
      } else {
        // ('<' | '>') <some random character>
        plx->lex_type = ('>' == ch ? L_GT : L_LT);
        gr_putback_char(pinput, next_ch);
      }
      break;
    case '=':
      plx->lex_type = L_EQ;
      break;
    default:
      result = LX_UNKNOWN_CHAR;
      break;
  }
  return result;
}

uint32_t lx_scan_symbol(GEN_READ *pinput, LEX_UNIT *plx, STRTAB *pstrtab)
{
  uint32_t result = LX_SCAN_OK;
  char *pname = lx_scan_generic_name(pinput, pstrtab);
  if (NULL != pname) {
    plx->lex_psym_name = pname;
    plx->lex_type = L_SYMBOL;
  } else {
    result = LX_UNABLE_TO_SAVE_NAME_OR_STRING;
  }
  return result;
}

// Return pointer to name in string table (pstrtab).
// At least one character must remain to be pinput.
char *lx_scan_generic_name(GEN_READ *pinput, STRTAB *pstrtab)
{
  char name[MAX_STR];
  char ch;
  uint32_t n_chars = 0;
  char *result = NULL;
  gr_get_char(pinput, &ch);
  do {
    if (n_chars < MAX_STR - 1) {
      name[n_chars++] = ch;
    } else if (MAX_STR - 1 == n_chars) {
      name[n_chars++] = '\0';
    }
  } while (gr_get_char(pinput, &ch) && IS_NAME_CHAR(ch));
  if (n_chars <= MAX_STR - 1) {
    name[n_chars] = '\0';
  }
  if (EOF != ch && !gr_eof(pinput)) {
    gr_putback_char(pinput, ch);
  }
  // Not checking return code since result will be NULL if insert fails and NULL is
  // this function's return value for an error.
  strtab_insert(pstrtab, name, &result);
  return result;
}

uint32_t lx_scan_var_name_or_kw(GEN_READ *pinput, LEX_UNIT *plx, STRTAB *pstrtab)
{
  uint32_t result = LX_SCAN_OK;
  char *pname = lx_scan_generic_name(pinput, pstrtab);
  if (NULL != pname) {
    int i;
    for (i = 0; !STREQ(pname, g_keywords[i].kw_name) && !STREQ("", g_keywords[i].kw_name); ++i)
      ;
    if (STREQ("", g_keywords[i].kw_name)) {
      plx->lex_pvar_name = pname;
      plx->lex_type = L_VAR_NAME;
    } else {
      plx->lex_type = g_keywords[i].kw_type;
    }
  } else {
    result = LX_UNABLE_TO_SAVE_NAME_OR_STRING;
  }
  return result;
}

// Map: character -> lexical unit code (if single char) OR
//                   scanning function (if multi char)
struct {
  char ch;
  bool is_multi_char;
  union {
    // If not multi char,
    uint32_t lex_unit_code;
    // If multi char,
    LX_SCAN_FN scanfn;
  };
} g_scan_map[] = {
    ['!' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_exclamation    },
    ['"' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['#' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['$' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['%' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_MOD           },
    ['&' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['\'' - '!'] =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['(' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_LPAREN        },
    [')' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_RPAREN        },
    ['*' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_TIMES         },
    ['+' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_PLUS          },
    [',' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_TUPLECAT      },
    ['-' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_MINUS         },
    ['.' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['/' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_DIVIDE        },
    ['0' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['1' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['2' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['3' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['4' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['5' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['6' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['7' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['8' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    ['9' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         },
    [':' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_assign         },
    [';' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_SEQ           },
    ['<' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_relop          },
    ['=' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_relop          },
    ['>' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_relop          },
    ['?' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['@' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['A' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['B' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['C' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['D' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['E' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['F' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['G' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['H' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['I' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['J' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['K' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['L' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['M' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['N' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['O' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['P' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['Q' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['R' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['S' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['T' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['U' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['V' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['W' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['X' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['Y' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['Z' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_symbol         },
    ['[' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['\\' - '!'] =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    [']' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       },
    ['^' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_UNKNOWN       } ,
    ['_' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['`' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_CLOSUREIZE    },
    ['a' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['b' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['c' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['d' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['e' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['f' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['g' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['h' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['i' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['j' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['k' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['l' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['m' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['n' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['o' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['p' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['q' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['r' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['s' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['t' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['u' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['v' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['w' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['x' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['y' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['z' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_var_name_or_kw },
    ['{' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_FN_BEGIN      },
    ['|' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_PATTERN_ALT   },
    ['}' - '!']  =  { .is_multi_char = false, .lex_unit_code = L_FN_END        },
    ['~' - '!']  =  { .is_multi_char = true,  .scanfn = lx_scan_number         }
};

// Driver for various lexical unit scanners.
uint32_t lx_scan_next(GEN_READ *pinput, LEX_UNIT *plx, STRTAB *pstrtab)
{
  uint32_t retval = LX_UNKNOWN_CHAR;
  char ch;
  lx_skip_whitespace(pinput);
  plx->lex_line_n = pinput->gr_line_n;
  plx->lex_col_n = pinput->gr_col_n;
  if (gr_get_char(pinput, &ch)) {
    if (g_scan_map[ch - '!'].is_multi_char) {
      // Put character back onto pinput so that it will be seen by scanfn.
      gr_putback_char(pinput, ch);
      retval = (*g_scan_map[ch - '!'].scanfn)(pinput, plx, pstrtab);
    } else {
      retval = LX_SCAN_OK;
      plx->lex_type = g_scan_map[ch - '!'].lex_unit_code;
      // Skip single character since scan is complete.
      gr_get_char(pinput, &ch);
    }
  }
  return retval;
}

#if 0
uint32_t lx_scan_next(GEN_READ *pinput, LEX_UNIT *plx, STRTAB *pstrtab)
{
  uint32_t retval = LX_UNKNOWN_CHAR;
  char ch;
  lx_skip_whitespace(pinput);
  plx->lex_line_n = pinput->gr_line_n;
  plx->lex_col_n = pinput->gr_col_n;
  if (gr_get_char(pinput, &ch)) {
    if (';' == ch) {
      plx->lx_type = L_SEQ;
      retval = LX_SCAN_OK;
    } else if ('+' == ch) {
      plx->lx_type = L_PLUS;
      retval = LX_SCAN_OK;
    } else if ('-' == ch) {
      plx->lx_type = L_MINUS;
      retval = LX_SCAN_OK;
    } else if ('`' == ch) {
      plx->lx_type = L_CLOSUREIZE;
      retval = LX_SCAN_OK;
    } else if ('%' == ch) {
      plx->lx_type = L_MOD;
      retval = LX_SCAN_OK;
    } else if ('*' == ch) {
      plx->lx_type = L_TIMES;
      if (gr_get_char(pinput, &ch)) {
        if ('*' == ch) {
          plx->lx_type = L_POWER;
        } else if (!gr_eof(pinput)) {
          gr_putback_char(pinput, ch);
        }
        retval = LX_SCAN_OK;
      }
    } else if ('{' == ch) {
      plx->lx_type = L_FN_BEGIN;
      retval = LX_SCAN_OK;
    } else if ('}' == ch) {
      plx->lx_type = L_FN_END;
      retval = LX_SCAN_OK;
    } else if (',' == ch) {
      plx->lx_type = L_TUPLECAT;
      retval = LX_SCAN_OK;
    } else if ('!' == ch) {
      if (gr_get_char(pinput, &ch) && '!' == ch) {
        plx->lx_type = L_GUARD;
      } else {
        if (!gr_eof(pinput)) {
          gr_putback_char(pinput, ch);
        }
        plx->lx_type = L_SUCH_THAT;
      }
      retval = LX_SCAN_OK;
    } else if ('=' == ch) {
      if (gr_get_char(pinput, &ch) && '>' == ch) {
        plx->lx_type = L_RESULT;
        retval = LX_SCAN_OK;
      }
    } else if (':' == ch) {
      if (gr_get_char(pinput, &ch) && '=' == ch) {
        plx->lx_type = L_ASSIGN;
        retval = LX_SCAN_OK;
      }
    }  else if (isalpha(ch)) {
      // L_SYMBOL or L_VAR_NAME or L_..._KW
      uint32_t n_chars = 0;
      char name[MAX_STR];
      do {
        if (n_chars < MAX_STR - 1) {
          name[n_chars] = ch;
        } else if (MAX_STR - 1 == n_chars) {
          name[n_chars] = '\0';
        }
        n_chars += 1;
      } while (gr_get_char(pinput, &ch) && IS_NAME_CHAR(ch));
      if (n_chars < MAX_STR - 1) {
        name[n_chars] = '\0';
      }
      if (EOF != ch && !gr_eof(pinput)) {
        gr_putback_char(pinput, ch);
      }
      if (isupper(name[0])) {
        if (ST_UNABLE_TO_INSERT == strtab_insert(pstrtab, name, &plx->lx_psym_name)) {
          retval = LX_UNABLE_TO_SAVE_NAME_OR_STRING;
        } else {
          plx->lx_type = L_SYMBOL;
          retval = LX_SCAN_OK;
        }
      } else {
        plx->lx_type = L_VAR_NAME;
        for (uint8_t i = 0; L_VAR_NAME == plx->lx_type && !STREQ("", g_keywords[i].kw_name); ++i) {
          if (STREQ(name, g_keywords[i].kw_name)) {
            retval = plx->lx_type = g_keywords[i].kw_type;
          }
        }
      }
      if (L_VAR_NAME == plx->lx_type) {
        if (ST_UNABLE_TO_INSERT == strtab_insert(pstrtab, name, &plx->lx_pvar_name)) {
          retval = LX_UNABLE_TO_SAVE_NAME_OR_STRING;
        } else {
          retval = LX_SCAN_OK;
        }
      }
    }
  } else if (gr_eof(pinput)) {
    plx->lx_type = L_EOF;
    retval = LX_SCAN_OK;
  } else {
    plx->lx_type = L_UNKNOWN;
    retval = LX_UNKNOWN_CHAR;
  }
#if defined(DEBUG)
  if (!scode_is_error(retval)) {
    printf("Lexical unit scanned:\n");
    lx_print(plx);
  } else {
    printf("Error: %s (%c) (%02d).\n", scode_name(retval), ch, ch);
    printf("Line: %u, char: %u\n", pinput->gr_line_n, pinput->gr_col_n);
  }
#endif
  return retval;
}
#endif

#if defined(TEST_LEX)

int main(int argc, char **argv)
{
  GEN_READ gr;
  LEX_UNIT lx;
  ARENA *pmem = stkalloc_new_arena(MIB(10));
  STRTAB *pstrtab = strtab_new(pmem);
  init_gr(*argv[1], argv[2], &gr);
  while (LX_SCAN_OK == lx_scan_next(&gr, &lx, pstrtab)) {
    lx_print(&lx);
  }
  gr_close(&gr);
  return 0;
}

#endif
