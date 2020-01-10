#include "sputz-includes.h"

#include "enum-str.h"
char *g_lex_unit_names[] ={
#include "enum-L.h"
};

void lx_print(LEX_UNIT *plx)
{
  printf("LEX_UNIT {\n");
  printf("  lx_line_n == %u\n", plx->lx_line_n);
  printf("  lx_col_n  == %u\n", plx->lx_col_n);
  printf("  lx_type == %s,\n", g_lex_unit_names[plx->lx_type]);
  switch (plx->lx_type) {
    case L_NUMBER:
      printf("  lx_number == %g,\n", plx->lx_number);
      break;
    case L_VAR_NAME:
      printf("  lx_var_name == \"%s\",\n", plx->lx_var_name);
      break;
    case L_SYMBOL:
      printf("  lx_sym == \"%s\",\n", plx->lx_sym);
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
  "also",  L_ALSO_KW,
  "and",   L_AND_KW,
  "bind",  L_BIND_KW,
  "else",  L_ELSE_KW,
  "if",    L_IF_KW,
  "not",   L_NOT_KW,
  "null",  L_NULL_KW,
  "or",    L_OR_KW,
  "outer", L_OUTER_KW,
  "then",  L_THEN_KW,
  "",      0
};

bool lx_scan_next(GEN_READ *pinput, LEX_UNIT *plx)
{
  bool retval = false;
  char ch;
  lx_skip_whitespace(pinput);
  plx->lx_line_n = pinput->gr_line_n;
  plx->lx_col_n = pinput->gr_col_n;
  if (gr_get_char(pinput, &ch)) {
    if (';' == ch) {
      plx->lx_type = L_SEQ;
      retval = true;
    } else if ('+' == ch) {
      plx->lx_type = L_PLUS;
      retval = true;
    } else if ('-' == ch) {
      plx->lx_type = L_MINUS;
    } else if ('`' == ch) {
      plx->lx_type = L_CLOSUREIZE;
      retval = true;
    } else if ('%' == ch) {
      plx->lx_type = L_MOD;
      retval = true;
    } else if ('*' == ch) {
      // maybe ...
      plx->lx_type = L_TIMES;
      if (gr_get_char(pinput, &ch)) {
        if ('*' == ch) {
          plx->lx_type = L_POWER;
        } else if (!gr_eof(pinput)) {
          gr_putback_char(pinput, ch);
        }
        retval = true;
      }
    } else if ('{' == ch) {
      plx->lx_type = L_FN_BEGIN;
      retval = true;
    } else if ('}' == ch) {
      plx->lx_type = L_FN_END;
      retval = true;
    } else if (',' == ch) {
      plx->lx_type = L_TUPLECAT;
      retval = true;
    } else if ('!' == ch) {
      if (gr_get_char(pinput, &ch) && '!' == ch) {
        plx->lx_type = L_GUARD;
      } else {
        if (!gr_eof(pinput)) {
          gr_putback_char(pinput, ch);
        }
        plx->lx_type = L_SUCH_THAT;
      }
      retval = true;
    } else if ('=' == ch) {
      if (gr_get_char(pinput, &ch) && '>' == ch) {
        plx->lx_type = L_RESULT;
        retval = true;
      }
    } else if (':' == ch) {
      if (gr_get_char(pinput, &ch) && '=' == ch) {
        plx->lx_type = L_ASSIGN;
        retval = true;
      }
    } else if ('~' == ch || isdigit(ch)) {
      // L_NUMBER
      // Numeric syntax:
      // ['~']<digit>+['.'<digit>*][('e'|'E')['~']<digit>+]
      int8_t mant_sign = 1;
      int8_t exp_sign = 1;
      int64_t int_part = 0;
      int64_t exp_part = 0;
      double frac_part = 0.0;
      // **SIGN**
      if ('~' == ch) {
        mant_sign = -1;
        // Use a harmless value for c to avoid more
        // complicated code involving gr_get_char()
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
      plx->lx_type = L_NUMBER;
      plx->lx_number = mant_sign*(int_part + frac_part)*pow(10.0, exp_sign*exp_part);
      retval = true;
    } else if (isalpha(ch)) {
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
      if (n_chars < MAX_STR) {
        name[n_chars] = '\0';
      }
      if (EOF != ch && !gr_eof(pinput)) {
        gr_putback_char(pinput, ch);
      }
      if (isupper(name[0])) {
        strcpy(plx->lx_sym, name);
        plx->lx_type = L_SYMBOL;
      } else {
        strcpy(plx->lx_var_name, name);
        plx->lx_type = L_VAR_NAME;
        for (uint8_t i = 0; L_VAR_NAME == plx->lx_type && !STREQ("", g_keywords[i].kw_name); ++i) {
          if (STREQ(name, g_keywords[i].kw_name)) {
            plx->lx_type = g_keywords[i].kw_type;
          }
        }
      }
      retval = true;
    }
  } else if (gr_eof(pinput)) {
    plx->lx_type = L_EOF;
    retval = true;
  } else {
    plx->lx_type = L_UNKNOWN;
    retval = false;
  }
#if defined(DEBUG)
  if (retval) {
    printf("Lexical unit scanned:\n");
    lx_print(plx);
  } else {
    printf("Unrecognized: (%c).\n", ch);
  }
#endif
  return retval;
}

#if defined(TEST_LEX)

//----------------------------------------------------------------------------------------------------------------------
// Initialize GEN_READ *r from command-line parameters passed in.
static void init_gr(char test_type, char *arg, GEN_READ *r)
{
  switch (test_type) {
    case 'f':
      if (!gr_open_file(r, arg)) {
        fprintf(stderr, "Can't open %s\n", arg);
        exit(0);
      }
      break;
    case 's':
      if (!gr_open_str(r, arg)) {
        fprintf(stderr, "Can't initialize with string.\n");
        exit(0);
      }
      break;
#if 0
    case 'r':
      if (!gr_open_rdln(r, arg)) {
        fprintf(stderr, "Can't initialize with readline.\n");
        exit(0);
      }
      break;
#endif
    default:
      fprintf(stderr, "Unrecognized input type: %c\n", test_type);
      exit(0);
      break;
  }
}

int main(int argc, char **argv)
{
  GEN_READ gr;
  LEX_UNIT lx;
  init_gr(*argv[1], argv[2], &gr);
  while (lx_scan_next(&gr, &lx)) {
    lx_print(&lx);
  }
  gr_close(&gr);
  return 0;
}

#endif
