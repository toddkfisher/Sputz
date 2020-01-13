#include "sputz-includes.h"

#include "enum-str.h"
char *node_type_names[] = {
#include "enum-PT.h"
};

#define IND do { for (int i = 0; i < indent; ++i) { printf(" "); } } while (0)

#define BINARY_OP_CASE(node_type)                         \
  case node_type:                                         \
    IND; printf("-nd_binop_left_expr:\n");                \
    parse_tree_print(p->nd_binop_left_expr, indent + 2);  \
    IND; printf("- nd_binop_right_expr:\n");              \
    parse_tree_print(p->nd_binop_right_expr, indent + 2); \
    break

void parse_tree_print(PARSE_TREE_NODE *p, uint8_t indent)
{
  IND;
  if (NULL != p) {
    printf("%s\n", node_type_names[p->nd_type]);
    switch (p->nd_type) {
      BINARY_OP_CASE(NT_TUPLECAT_OP);
      BINARY_OP_CASE(NT_AND_OP);
      BINARY_OP_CASE(NT_OR_OP);
      BINARY_OP_CASE(NT_LT_OP);
      BINARY_OP_CASE(NT_GT_OP);
      BINARY_OP_CASE(NT_LE_OP);
      BINARY_OP_CASE(NT_GE_OP);
      BINARY_OP_CASE(NT_EQ_OP);
      BINARY_OP_CASE(NT_ADD_OP);
      BINARY_OP_CASE(NT_SUB_OP);
      BINARY_OP_CASE(NT_MUL_OP);
      BINARY_OP_CASE(NT_DIV_OP);
      BINARY_OP_CASE(NT_MOD_OP);
      BINARY_OP_CASE(NT_SEQ_OP);
      case NT_NOT_OP:
        IND; printf("-nd_unop_expr: ");
        parse_tree_print(p->nd_unop_expr, indent + 2);
        break;
      case NT_IF:
        IND; printf("-nd_if_test:\n");
        parse_tree_print(p->nd_if_test, indent + 2);
        IND; printf("-nd_if_then_branch:\n");
        parse_tree_print(p->nd_if_then_branch, indent + 2);
        IND; printf("-nd_if_else_branch:\n");
        parse_tree_print(p->nd_if_else_branch, indent + 2);
        break;
      case NT_FN_DEF:
        break;
      case NT_SIMPLE_CLOSURE:
      case NT_PATT_CLOSURE:
        IND; printf("-nd_closure_expr:\n");
        parse_tree_print(p->nd_closure_expr, indent + 2);
        break;
      case NT_CLOSUREIZE:
        break;
      case NT_ASSIGN:
        IND; printf("-nd_assign_patt:\n");
        parse_tree_print(p->nd_assign_patt, indent + 2);
        IND; printf("-nd_assign_expr:\n");
        parse_tree_print(p->nd_assign_expr, indent + 2);
        break;
      case NT_VAR_REF:
        IND; printf("-nd_var_name: %s\n", p->nd_var_name);
        IND; printf("-nd_outer_count: %u\n", p->nd_outer_count);
        break;
      case NT_NUM_CONST:
        IND; printf("-nd_num_const : %g\n", p->nd_num_const);
        break;
      case NT_SYM_CONST:
        IND; printf("-%s\n", p->nd_sym_name);
        break;
      case NT_TAGGED_TUPLE:
        IND; printf("-nd_tagged_sym:\n");
        parse_tree_print(p->nd_tagged_sym, indent + 2);
        IND; printf("-nd_tagged_expr:\n");
        parse_tree_print(p->nd_tagged_expr, indent + 2);
        break;
      case NT_PATT_TAGGED_TUPLE:
        break;
      case NT_PATT_SYM:
        break;
      case NT_PATT_VAR_REF:
        break;
      case NT_PATT_NUM_CONST:
        break;
      case NT_PATT_BIND_VAR:
        break;
      case NT_PATT_APPLY:
        break;
      case NT_PATT_ALSO:
        break;
      case NT_PATT_TEST:
        break;
      case NT_PATT_RESULT_PAIR:
        break;
      case NT_PATT_TUPLECAT:
        break;
      case NT_APPLY:
        IND; printf("-nd_app_fn:\n");
        parse_tree_print(p->nd_app_fn, indent + 2);
        IND; printf("-nd_app_args_expr:\n");
        parse_tree_print(p->nd_app_args_expr, indent + 2);
        break;
      default:
        break;
    }
  }
}

// Reorder parse tree so that "(a - b - c) == ((a - b) - c) not (a - (b - c))"
void parse_tree_left_assoc(PARSE_TREE_NODE *np)
{
  if (NT_SUB_OP == np->nd_type && NT_SUB_OP == np->nd_binop_right_expr->nd_type) {
    PARSE_TREE_NODE *a;
    PARSE_TREE_NODE *b;
    PARSE_TREE_NODE *c;
    a = np->nd_binop_left_expr;
    b = np->nd_binop_right_expr->nd_binop_left_expr;
    c = np->nd_binop_right_expr->nd_binop_right_expr;
    np->nd_binop_right_expr->nd_binop_left_expr = a;
    np->nd_binop_right_expr->nd_binop_right_expr = b;
    np->nd_binop_left_expr = np->nd_binop_right_expr;
    np->nd_binop_right_expr = c;
    parse_tree_left_assoc(np->nd_binop_left_expr);
    parse_tree_left_assoc(np->nd_binop_right_expr);
  }
}

#define ERR_NYI(pstate)                                         \
  do {                                                          \
    sprintf((pstate)->pst_err_msg, "%s : NYI.", __FUNCTION__);  \
    (pstate)->pst_status = S_PARSE_ERROR;                       \
    longjmp(g_abort, 1);                                        \
  } while (0)

// Location to jump to if a parse fails.  Used in some functions for backtracking.
// TODO: make thread-safe.
jmp_buf g_abort;

// Arena for memory allocated in stack discipline.  This allows for easy deallocation
// on a failed (partial) parse.
// TODO: make thread-safe.
ARENA *g_pmem;

extern char *g_lex_unit_names[];

// Scan the next lexical unit and store in PARSE_STATE
void parse_scan_lx_unit(PARSE_STATE *pstate)
{
  if (!lx_scan_next(&pstate->pst_input, &pstate->pst_lookahead)) {
    pstate->pst_status = S_LEX_ERROR;
    strcpy(pstate->pst_err_msg, "Scanning error. Unable to read.");
    longjmp(g_abort, 1);
  }
}

PARSE_TREE_NODE *parse_alloc_node(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *p;
  p = stkalloc_get_mem(g_pmem, sizeof(PARSE_TREE_NODE));
  if (NULL == p) {
    pstate->pst_status = S_MEM_OVERFLOW;
    // Maybe some better diagostics in the future.
    strcpy(pstate->pst_err_msg, "Memory overflow.");
    longjmp(g_abort, 1);
  }
  zero_mem(p, sizeof(PARSE_TREE_NODE));
  return p;
}

void parse_expect(uint8_t lx_type, PARSE_STATE *pstate)
{
  if (lx_type != pstate->pst_lookahead.lx_type) {
    pstate->pst_status = S_LEX_ERROR;
    sprintf(pstate->pst_err_msg, "Scanning error. Expected %s.", g_lex_unit_names[lx_type]);
    longjmp(g_abort, 1);
  }
}

PARSE_TREE_NODE *parse_create_binop(uint8_t op, PARSE_TREE_NODE *pleft, PARSE_TREE_NODE *pright,
                                    PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *retval;
  retval = parse_alloc_node(pstate);
  retval->nd_type = op;
  retval->nd_binop_left_expr = pleft;
  retval->nd_binop_right_expr = pright;
  return retval;
}

// Parser entry point.
PARSE_TREE_NODE *parse_sputz_program(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *retval = NULL;
  retval = parse_seq_expr(pstate);
  parse_expect(L_EOF, pstate);
  return retval;
}

// seq-expr =  expr [';' seq-expr]
PARSE_TREE_NODE *parse_seq_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *retval = NULL;
  retval = parse_expr(pstate);
  while (L_SEQ == pstate->pst_lookahead.lx_type) {
    PARSE_TREE_NODE *pleft = retval;
    PARSE_TREE_NODE *pright;
    pleft = retval;
    // Skip ';'
    parse_scan_lx_unit(pstate);
    pright = parse_expr(pstate);
    retval = parse_create_binop(NT_SEQ_OP, pleft, pright, pstate);
  }
  return retval;
}

// expr = ifExpr | assignOrTupleExpr
PARSE_TREE_NODE *parse_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  if (L_IF_KW == pstate->pst_lookahead.lx_type) {
    result = parse_if_expr(pstate);
  } else {
    result = parse_assign_or_tuple_expr(pstate);
  }
  return result;
}

// assign-or-tuple-expr = assign-expr | tuple-expr
PARSE_TREE_NODE *parse_assign_or_tuple_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_assign_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_if_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_tuple_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_tuple_component(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_or_term(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_and_term(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_compare_term(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_term(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_factor(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_number(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  parse_expect(L_NUMBER, pstate);
  result = parse_alloc_node(pstate);
  result->nd_type = NT_NUM_CONST;
  result->nd_num_const = pstate->pst_lookahead.lx_number;
  parse_scan_lx_unit(pstate);
  return result;
}

PARSE_TREE_NODE *parse_symbol(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_var_name(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_data_constructor(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_application(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_closure(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_pattern_alternative(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_pattern_tuple(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_pattern_tuple_component(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_pattern_factor(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_pattern_data_constructor(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

bool parse_init(PARSE_STATE *pstate, char test_type, char *arg, ARENA **ppmem)
{
  bool retval = true;
  init_gr(test_type, arg, &pstate->pst_input);
  pstate->pst_status = lx_scan_next(&pstate->pst_input, &pstate->pst_lookahead) ? S_OK : S_LEX_ERROR;
  return S_OK == pstate->pst_status && (NULL != (*ppmem = stkalloc_new_arena(1024)));
}

void parse_fin(PARSE_STATE *pstate)
{
  gr_close(&pstate->pst_input);
  pstate->pst_status = S_OK;
  strcpy(pstate->pst_err_msg, "");
  pstate->pst_lookahead.lx_type = L_UNKNOWN;
  if (NULL != g_pmem) {
    stkalloc_free_arena(g_pmem);
  }
}

//----------------------------------------------------------------------------------------------------------------------
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

#if defined(TEST_PARSE)

int main(int argc, char **argv)
{
  GEN_READ gr;
  PARSE_STATE pstate;
  PARSE_TREE_NODE *proot;
  int retval = 0;
  if (argc < 3 || !STREQ(argv[1], "-s")) {
    fprintf(stderr, "Usage: %s -s '<text>'\n", argv[0]);
    retval = 1;
  } else if (!parse_init(&pstate, argv[1][1], argv[2], &g_pmem)) {
    fprintf(stderr, "Initialization failure.\n");
    retval = 2;
  } else if (!setjmp(g_abort)) {
    proot = parse_sputz_program(&pstate);
    fprintf(stderr, "Parse successful.\n");
    printf("Result : \n");
    parse_tree_print(proot, 0);
  } else {
    fprintf(stderr, "%s\n", pstate.pst_err_msg);
    retval = 3;
  }
  parse_fin(&pstate);
  return retval;
}

#endif
