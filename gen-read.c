#include "sputz-includes.h"

//------------------------------------------------------------------------------
// Initialize a GEN_READ to get characters from a string.
bool gr_open_str(GEN_READ *r, char *s)
{
  r->gr_type = I_STRING;
  r->gr_str = strdup(s);
  r->gr_strp = r->gr_str;
  r->gr_str_end = r->gr_str + strlen(r->gr_str);
  r->gr_putback_ch = 0;
  r->gr_line_n = 1;
  r->gr_col_n = 1;
  r->gr_eof = false;
  return true;
}

//------------------------------------------------------------------------------
// Initialize a GEN_READ to get characters from a FILE.
bool gr_open_file(GEN_READ *r, char *pth)
{
  r->gr_type = I_FILE;
  r->gr_file = NULL;
  r->gr_putback_ch = 0;
  r->gr_line_n = 1;
  r->gr_col_n = 1;
  r->gr_eof = false;
  return NULL != (r->gr_file = fopen(pth, "r"));
}

#if 0
//------------------------------------------------------------------------------
// Initialize a GEN_READ to get characters from GNU readline.
bool gr_open_rdln(GEN_READ *r, char *prompt)
{
  r->gr_type = I_READLINE;
  r->gr_rdlnp = r->gr_rdln = NULL;
  r->gr_eof = false;
  r->gr_putback_ch = 0;
  if (NULL != prompt) {
    r->gr_prompt = strdup(prompt);
  } else {
    r->gr_prompt = NULL;
  }
  r->gr_line_n = 1;
  r->gr_col_n = 1;
  return true;
}

//------------------------------------------------------------------------------
// If GEN_READ r was initialized to input from GNU readline, then
// set the prompt.
void gr_set_prompt(GEN_READ *r, char *prompt)
{
  if (I_READLINE == r->gr_type) {
    if (NULL != r->gr_prompt) {
      free(r->gr_prompt);
    }
    r->gr_prompt = strdup(prompt);
  }
}

#endif

//------------------------------------------------------------------------------
// Get the next character from GEN_READ r.  If one was placed into the putback
// buffer then that will be returned.  End of file behavior for string inputs
// is exactly the same as EOF behavior for FILEs.  EOF behavior for
// GNU readline inputs occurs when NOTHING is typed on a line 'cept C-d.
bool gr_get_char(GEN_READ *r, char *c)
{
  bool retval = false;
  *c = EOF;
  if (!r->gr_eof && r->gr_putback_ch > 0) {
    *c = r->gr_putback_ch;
    r->gr_putback_ch = 0;
    retval = true;
  } else {
    switch (r->gr_type) {
      case I_STRING:
        if (NULL != r->gr_strp && (r->gr_strp <= r->gr_str_end)) {
          *c = *r->gr_strp++;
          if (*c) {
            retval = true;
          } else {
            // set *c to universal EOF char if end of string.
            *c = EOF;
            retval = true;
          }
        } else {
          // set *c to universal EOF char if past end of string for some reason.
          *c = EOF;
          retval = false;
        }
        break;
#if 0
      case I_READLINE:
        if (r->gr_rdln && !*r->gr_rdlnp && r->gr_ret_nl) {
          // Return newline on first encounter with end of string.
          *c = '\n';
          r->gr_ret_nl = 0;
          retval = true;
        } else if (!r->rdln || !*r->rdlnp) {
          // If the current line doesn't exist, or we are at the
          // terminating nul ...
          if (r->gr_rdln) {
            free(r->gr_rdln);
          }
          r->gr_rdlnp = r->gr_rdln = readline(r->gr_prompt ? r->gr_prompt : "");
          r->gr_ret_nl = 1;
          if (r->rdln && *r->rdlnp && !blanks(r->rdln)) {
            // Only add lines to the history if they:
            // (1) exist.
            // (2) are nonempty and
            // (3) are not blank
            add_history(r->gr_rdln);
          }
        }
        if (r->gr_rdln) {
          // readline() will return NULL when EOF is encountred.
          if (NULL != *r->gr_rdlnp) {
            // Don't go past the terminating nul if an empty line
            // ("") was returned by readline().
            *c = *r->gr_rdlnp++;
            retval = true;
          } else {
            // First time end of string seen, return newline just
            // like above.
            *c = '\n';
            r->gr_ret_nl = 0;
            retval = true;
          }
        } else {
          // r->rdln is NULL which is readline()'s way of telling us
          // that the user pressed ctrl-D.
          *c = EOF;
          r->gr_eof = true;
          r->gr_ret_nl = false;
          retval = true;
        }
        break;
#endif
      case I_FILE:
        retval = true;
        if (NULL != r->gr_file) {
          if (EOF == (*c = fgetc(r->gr_file))) {
            r->gr_eof = true;
          }
        } else {
          *c = EOF;
          retval = false;
        }
        break;
      default:
        break;
    }
  }
  if (retval && !r->gr_eof) {
    if ('\n' == *c) {
      r->gr_line_n += 1;
      r->gr_col_n = 1;
    } else {
      r->gr_col_n += 1;
    }
  }
  r->gr_eof = !retval;
  return retval;
}

//------------------------------------------------------------------------------
// Get the current position in input for later backup.
bool gr_get_pos(GEN_READ *r, GEN_READ_POSITION *pos)
{
  bool retval = true;
  pos->pos_type = r->gr_type;
  switch (r->gr_type) {
    case I_FILE:
      if (-1 == fgetpos(r->gr_file, &pos->pos_file)) {
        retval = false;
      }
      break;
    case I_STRING:
      pos->pos_str = r->gr_strp;
      break;
    default:
      retval = false;
  }
  return retval;
}

//------------------------------------------------------------------------------
// Return to a previously visited position on the input.
bool gr_set_pos(GEN_READ *r, GEN_READ_POSITION *pos)
{
  bool retval = true;
  if (r->gr_type != pos->pos_type) {
    retval = false;
  } else {
    switch (pos->pos_type) {
      case I_FILE:
        if (-1 == fsetpos(r->gr_file, &pos->pos_file)) {
          retval = false;
        }
        break;
      case I_STRING:
        r->gr_strp = pos->pos_str;
        break;
      default:
        retval = false;
    }
  }
  return retval;
}

//------------------------------------------------------------------------------
// Behave like feof() but for all input types.
bool gr_eof(GEN_READ *r)
{
  bool retval = false;
  switch (r->gr_type) {
    case I_STRING:
      retval = r->gr_eof || !r->gr_strp || (r->gr_strp > r->gr_str_end);
      break;
#if 0
    case I_READLINE:
      retval = (NULL == r->gr_rdln) && r->gr_eof;
      break;
#endif
    case I_FILE:
      retval = r->gr_eof || ((NULL != r->gr_file) && feof(r->gr_file));
      break;
    default:
      retval = false;
  }
  return retval;
}

//------------------------------------------------------------------------------
// Place a character onto the putback buffer (1 character) of GEN_READ r.
// ALWAYS sets "end of file" condition to FALSE.
void gr_putback_char(GEN_READ *r, char c)
{
  r->gr_eof = 0;
  r->gr_putback_ch = c;
  if ('\n' == c) {
    r->gr_line_n = MAX(1, r->gr_line_n - 1);
    r->gr_col_n = 1;
  } else {
    r->gr_col_n = MAX(1, r->gr_col_n - 1);
  }
}

void gr_close(GEN_READ *r)
{
  r->gr_putback_ch = 0;
  switch (r->gr_type) {
    case I_STRING:
      if (NULL != r->gr_str) {
        free(r->gr_str);
      }
      r->gr_strp = r->gr_str = NULL;
      break;
#if 0
    case I_READLINE:
      if (NULL != r->gr_rdln) {
        free(r->gr_rdln);
      }
      if (NULL != r->gr_prompt) {
        free(r->gr_prompt);
      }
      r->gr_rdlnp = r->gr_rdln = NULL;
      r->eof = true;
      break;
#endif
    case I_FILE:
      if (NULL != r->gr_file) {
        fclose(r->gr_file);
      }
      r->gr_file = NULL;
      break;
    default:
      break;
  }
}

#ifdef TEST_INPUT

//------------------------------------------------------------------------------
// Initialize GEN_READ *r from command-line parameters passed in.
void init_gr(char test_type, char *arg, GEN_READ *r)
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
  if (argc == 3 && (*argv[1] == 'f' || *argv[1] == 's' || *argv[1] == 'r')) {
    char c;
    int n = 0;
    GEN_READ r;
    GEN_READ_POSITION p;
    init_gr(*argv[1], argv[2], &r);
    if (!gr_get_pos(&r, &p)) {
      printf("Unable to get position at BOF.\n");
      return 0;
    }
    printf("Reading and testing EOF with gr_eof()\n");
    gr_close(&r);
    init_gr(*argv[1], argv[2], &r);
    n = 0;
    while (!gr_eof(&r)) {
      // Won't know that we're at eof until we actually try to get
      // another character.
      if (gr_get_char(&r, &c)) {
        printf("Char #%d, (%c) = (%d)\n", n++, c, c);
      }
    }
    printf("Backing up to BOF and reading 10 characters ... \n");
    if (!gr_set_pos(&r, &p)) {
      printf("Unable to move to BOF.\n");
      return 0;
    }
    for (int i = 0; i < 10; ++i) {
      if (gr_get_char(&r, &c)) {
        printf("Char #%d, (%c) = (%d)\n", n++, c, c);
      }
    }
  } else {
    printf("args: (f <file> | s <string> | r <prompt>)\n");
  }
}
#endif
