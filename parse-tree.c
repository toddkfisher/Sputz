#include "sputz-includes.h"

#include "enum-str.h"
char *node_type_names[] = {
#include "enum-PT.h"
};

#define INDENT_SPACES 2

extern STRTAB *g_pstrtab;

#define IND do { for (int i = 0; i < indent; ++i) { printf(" "); } } while (0)


bool parse_is_binary_op(uint32_t type)
{
  bool result = type > NT_BEGIN_BINARY_OP && type < NT_END_BINARY_OP;
  return result;
}

void parse_print_binary_op(PARSE_TREE_NODE *p, uint8_t indent)
{
  IND; printf("left_expr:\n");
  parse_tree_print(p->nd_binop_left_expr, indent + INDENT_SPACES);
  IND; printf("right_expr:\n");
  parse_tree_print(p->nd_binop_right_expr, indent + INDENT_SPACES);
}

bool parse_is_unary_op(uint32_t type)
{
  bool result = type > NT_BEGIN_UNARY_OP && type < NT_END_UNARY_OP;
  return result;
}

void parse_print_unary_op(PARSE_TREE_NODE *p, uint8_t indent)
{
  IND; printf("unop_expr:\n");
  parse_tree_print(p->nd_unop_expr, indent + INDENT_SPACES);
}

void parse_tree_print(PARSE_TREE_NODE *p, uint8_t indent)
{
  IND;
  if (NULL != p) {
    printf("%s\n", node_type_names[p->nd_type]);
    if (parse_is_binary_op(p->nd_type)) {
      parse_print_binary_op(p, indent);
    } else if (parse_is_unary_op(p->nd_type)) {
      parse_print_unary_op(p, indent);
    } else {
      switch (p->nd_type) {
        case NT_IF:
          IND; printf("if_test:\n");
          parse_tree_print(p->nd_if_test, indent + INDENT_SPACES);
          IND; printf("then_branch:\n");
          parse_tree_print(p->nd_if_then_branch, indent + INDENT_SPACES);
          IND; printf("else_branch:\n");
          parse_tree_print(p->nd_if_else_branch, indent + INDENT_SPACES);
          break;
        case NT_FN_DEF:
          break;
        case NT_CLOSUREIZE:
          break;
        case NT_SIMPLE_CLOSURE:
          break;
        case NT_PATT_CLOSURE:
          break;
        case NT_VAR_REF:
          break;
        case NT_NUM_CONST:
          break;
        case NT_SYM_CONST:
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
        case NT_PATT_RESULT_PAIR:
          break;
        case NT_APPLY:
          break;
        case NT_RESULT:
          break;
      }
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
    longjmp((pstate)->pst_abort, 1);                            \
  } while (0)

// Scan the next lexical unit and store in PARSE_STATE
void parse_scan_lx_unit(PARSE_STATE *pstate)
{
  if (LX_SCAN_OK != lx_scan_next(&pstate->pst_input, &pstate->pst_lookahead, pstate->pst_pstrtab)) {
    pstate->pst_status = S_LEX_ERROR;
    strcpy(pstate->pst_err_msg, "Scanning error. Unable to read.");
    longjmp(pstate->pst_abort, 1);
  }
}

PARSE_TREE_NODE *parse_alloc_node(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *p;
  p = stkalloc_get_mem(pstate->pst_pmem, sizeof(PARSE_TREE_NODE));
  if (NULL == p) {
    pstate->pst_status = S_MEM_OVERFLOW;
    // Maybe some better diagostics in the future.
    strcpy(pstate->pst_err_msg, "Memory overflow.");
    longjmp(pstate->pst_abort, 1);
  }
  zero_mem(p, sizeof(PARSE_TREE_NODE));
  return p;
}

void parse_expect(uint8_t lx_type, PARSE_STATE *pstate)
{
  if (lx_type != pstate->pst_lookahead.lx_type) {
    pstate->pst_status = S_LEX_ERROR;
    sprintf(pstate->pst_err_msg, "Scanning error. Expected %s.", lx_name(lx_type));
    longjmp(pstate->pst_abort, 1);
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

// expr = ifExpr | resultExpr | assignExpr
PARSE_TREE_NODE *parse_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  if (L_IF_KW == pstate->pst_lookahead.lx_type) {
    result = parse_if_expr(pstate);
  } else if (L_RESULT_KW == pstate->pst_lookahead.lx_type) {
    result = parse_result_expr(pstate);
  } else {
    result = parse_assign_expr(pstate);
  }
  return result;
}

// assignExpr = varName ':=' tupleExpr
PARSE_TREE_NODE *parse_assign_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  PARSE_TREE_NODE *expr = NULL;
  char *var_name;
  parse_expect(L_VAR_NAME, pstate);
  var_name = pstate->pst_lookahead.lx_pvar_name;
  return result;
}

// ifExpr = 'if' assignExpr 'then' assignExpr 'else' assignExpr
PARSE_TREE_NODE *parse_if_expr(PARSE_STATE *pstate)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}

PARSE_TREE_NODE *parse_result_expr(PARSE_STATE *pstate)
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

uint32_t parse_init(PARSE_STATE *pstate, char test_type, char *arg, ARENA **ppmem,
  STRTAB **ppstrtab)
{
  uint32_t result = S_OK;
  ABORT_ON_FALSE(S_OK == (result = init_gr(test_type, arg, &pstate->pst_input)), INIT_READ_FAIL);
  ABORT_ON_NULL(*ppmem = stkalloc_new_arena(MIB(10)), INIT_MEM_FAIL0);
  ABORT_ON_NULL(*ppstrtab = strtab_new(*ppmem), INIT_MEM_FAIL1);
  pstate->pst_status = lx_scan_next(&pstate->pst_input, &pstate->pst_lookahead, pstate->pst_pstrtab);
  result = pstate->pst_status;
  DBG_PRINT_VAR(scode_name(result), STRING);
  return result;
  INIT_READ_FAIL: {
    return result;
  }
  INIT_MEM_FAIL0: {
    result = S_UNABLE_TO_CREATE_ARENA;
    return result;
  }
  INIT_MEM_FAIL1: {
    result = S_UNABLE_TO_CREATE_STRTAB;
    return result;
  }
}

void parse_fin(PARSE_STATE *pstate)
{
  gr_close(&pstate->pst_input);
  pstate->pst_status = S_OK;
  strcpy(pstate->pst_err_msg, "");
  pstate->pst_lookahead.lx_type = L_UNKNOWN;
  if (NULL != pstate->pst_pmem) {
    stkalloc_free_arena(pstate->pst_pmem);
  }
}

//----------------------------------------------------------------------------------------------------------------------
// Initialize GEN_READ *r from command-line parameters passed in.
uint32_t init_gr(char test_type, char *arg, GEN_READ *r)
{
  uint32_t result = S_OK;
  switch (test_type) {
    case 'f':
      if (!gr_open_file(r, arg)) {
        result = S_UNABLE_TO_OPEN_FILE;
      }
      break;
    case 's':
      if (!gr_open_str(r, arg)) {
        result = S_UNABLE_TO_OPEN_STRING;
      }
      break;
#if 0
    case 'r':
      if (!gr_open_rdln(r, arg)) {
        result = S_UNABLE_TO_OPEN_READLINE;
      }
      break;
#endif
    default:
      result = S_UNKNOWN_INPUT_TYPE;
      break;
  }
  return result;
}

#if defined(TEST_PARSE)

int main(int argc, char **argv)
{
  GEN_READ gr;
  PARSE_STATE pstate;
  PARSE_TREE_NODE *proot;
  int retval = 0;
  uint32_t init_status_code = S_OK;
  if (argc < 3 || !STREQ(argv[1], "-s")) {
    fprintf(stderr, "Usage: %s -s '<text>'\n", argv[0]);
    retval = 1;
  } else if (scode_is_error(init_status_code = parse_init(&pstate,
                                                          argv[1][1],
                                                          argv[2],
                                                          &pstate.pst_pmem,
                                                          &pstate.pst_pstrtab))) {
    fprintf(stderr, "Initialization failure: %s.\n", scode_name(init_status_code));
    retval = 2;
  } else if (!setjmp(pstate.pst_abort)) {
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
