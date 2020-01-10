#pragma once

enum {
  I_STRING,
  I_READLINE,
  I_FILE
};

typedef struct GEN_READ_POSITION {
  uint8_t pos_type;
  union {
    // I_STRING
    char *pos_str;
    // I_FILE
    fpos_t pos_file;
  };
} GEN_READ_POSITION;

typedef struct GEN_READ {
  uint8_t gr_type;
  // Next character to be read if > 0.
  uint8_t gr_putback_ch;
  // Current line/column numbers in input.
  uint32_t gr_line_n;
  uint32_t gr_col_n;
  union {
    // I_STRING
    struct {
      // Pointer to beginning of string to read.
      char *gr_str;
      // Pointer to current character to read in str.
      char *gr_strp;
      // End of string used for "EOF" test.
      char *gr_str_end;
    };
    // I_READLINE
    struct {
      // Prompt to display if more input required.
      char *gr_prompt;
      // Pointer to beginning of current line begin read which is the
      // last line of input from user.
      char *gr_rdln;
      // Pointer to current character to be read in rdln.
      char *gr_rdlnp;
      // Did readline() return a NULL?  This exists because
      // rdln == NULL is one of the signals to get more input.
      bool gr_eof;
      // Flag to tell gr_get_char() to return a newline on
      // first encounter with end of string.
      bool gr_ret_nl;
    };
    // I_FILE
    struct {
      FILE *gr_file;
    };
  };
} GEN_READ;

#define GR_NEXT_CHAR(r)\
    do {\
        char c;\
        gr_get_char(r, &c);\
        gr_putback_char(r, c);\
    } while (0)
