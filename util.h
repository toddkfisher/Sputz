#pragma once

#if !defined(MAX)
#  define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#if !defined(MIN)
#  define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define MAX_SUMMARY_LENGTH 128
#define MAX_STR 1024

#define STREQ(s0, s1) (!strcmp((s0), (s1)))
#define STRIEQ(s0, s1) (!strcasecmp((s0), (s1)))
#define STRNEQ(s0, s1, n) (!strncmp((s0), (s1), (n)))
#define SCMP(s0, op, s1) (strcmp((s0), (s1)) op 0)

#define MALLOC_ARRAY(type, n) ((type *) calloc((n), sizeof(type)))

#define MALLOC_ARRAY_OR_DIE(avar, type, n)                                        \
  do {                                                                            \
    avar = MALLOC_ARRAY(type, n);                                                 \
    if (NULL == (avar)) {                                                         \
      fprintf(stderr, "Memory overflow in %s() attempting to allocate array of"   \
              " size %d of %s to %s.\n", __FUNCTION__, n, #type, #avar);          \
      exit(1);                                                                    \
    }                                                                             \
  } while (0);

#define MALLOC_1(type) ((type *) malloc(sizeof(type)))

#define ABORT_ON_FALSE(val, label)              \
  do {                                          \
    if (!(val)) {                               \
      DBG_PRINT_FN;                             \
      DBG_MSG("ABORT_ON_FALSE : aborting.\n");  \
      goto label;                               \
    }                                           \
  } while (0)

#define ABORT_ON_NULL(val, label)               \
  do {                                          \
    if (NULL == (val)) {                        \
      DBG_PRINT_FN;                             \
      DBG_MSG("ABORT_ON_NULL : aborting.\n");   \
      goto label;                               \
    }                                           \
  } while (0)

#define MALLOC_OR_ABORT(var, type, errlabel)    \
  do {                                          \
    if (NULL == (var = MALLOC_1(type))) {       \
      DBG_PRINT_FN;                             \
      DBG_MSG("MALLOC_OR_ABORT : aborting.\n"); \
      goto errlabel;                            \
    }                                           \
  } while (0)

#define MALLOC_1_OR_DIE(pvar, type)                                     \
  do {                                                                  \
    pvar = MALLOC_1(type);                                              \
    if (NULL == pvar) {                                                 \
      fprintf(stderr, "Memory overflow in %s() attempting to allocate %s to %s\n" \
              __FUNCTION__, #type, #pvar);                              \
      exit(1);                                                          \
    }                                                                   \
  } while (0)

// This longish macro is a exists since we don't have templates in straight C ...
// (and we don't want them either).
#define MALLOC_MATRIX_OR_DIE(mvar, type, rows, cols)                    \
  do {                                                                  \
    int i;                                                              \
    int j;                                                              \
    MALLOC_ARRAY_OR_DIE(mvar, type *, rows);                            \
    for (i = 0; i < (rows); ++i) {                                      \
      mvar[i] = MALLOC_ARRAY(type, cols);                               \
      if (NULL == (mvar)) {                                             \
        fprintf(stderr, "Memory overflow in %s() attempting to allocate %d x %d" \
                " matrix of %s to %s.\n", __FUNCTION__, rows, cols, #type, #mvar); \
        if (i > 0) {                                                    \
          for (j = 0; j < i - 1; ++j) {                                 \
            free(mvar[j]);                                              \
          }                                                             \
        }                                                               \
        exit(0);                                                        \
      }                                                                 \
    }                                                                   \
  } while (0)

// Simple macro for testing.
#define T(e)                                             \
  do {                                                   \
    printf("%s %d: TEST> " #e "\n", __FILE__, __LINE__); \
    assert(e);                                           \
  } while (0)

// Because I should never have to write: "typedef struct PARSE_TREE_NODE PARSE_TREE_NODE;"
#define typestruct(s) typedef struct s s

#define INT     "%d"
#define FLOAT   "%f"
#define DOUBLE  "%ld"
#define HEX     "%x"
#define POINTER "%p"

#ifdef DEBUG
#  define DBG_STUB(s) do { s; } while (0)
#  define DBG_PRINT_VAR(v, fmt) printf(#v " == " fmt "\n", v);
#  define DBG_PRINT(args, ...) printf(args, __VA_ARGS__)
#  define DBG_PRINT_FN printf("In %s()\n", __FUNCTION__)
#  define DBG_MSG(msg) printf("%s", (msg))
#  define DBG_ENTER printf("Entering: %s()\n", __FUNCTION__)
#  define DBG_LEAVE printf("Leaving: %s()\n", __FUNCTION__)
#else
#  define DBG_STUB(s) printf("STUB '%s' only active in DEBUG mode.\n")
#  define DBG_PRINT_VAR(v, fmt) do { } while (0)
#  define DBG_PRINT(args, ...) do { } while (0)
#  define DBG_PRINT_FN do { } while (0)
#  define DBG_MSG(msg) do { } while (0)
#  define DBG_ENTER do { } while (0)
#  define DBG_LEAVE do { } while (0)
#endif
