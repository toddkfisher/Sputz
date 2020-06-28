#include "sputz-includes.h"

#include "enum-str.h"
char *g_node_type_names[] = {
#include "enum-PT.h"
};

#define INDENT_SPACES 2

#define IND(n)                               \
  do {                                       \
    for (int i = 0; i < indent*(n); ++i) {   \
      printf(" ");                           \
    }                                        \
  } while (0)



char *parse_get_node_type_name(
  TAGGED_ENUM pt_type
)
{
  char *result = "UNKNOWN_NODE_TYPE";
  if (GET_ORDINAL(pt_type) < sizeof(g_node_type_names)/sizeof(char *)) {
    result = g_node_type_names[GET_ORDINAL(pt_type)];
  }
  return result;
}



void parse_print_binary_op(
  PARSE_TREE_NODE *p,
  uint8_t indent
)
{
  IND(1);
  printf("-left_expr:\n");
  parse_tree_print(p->nd_binop_left_expr,
                   indent + INDENT_SPACES);
  IND(1);
  printf("-right_expr:\n");
  parse_tree_print(p->nd_binop_right_expr,
                   indent + INDENT_SPACES);
}



void parse_print_unary_op(
  PARSE_TREE_NODE *p,
  uint8_t indent
)
{
  IND(1);
  printf("-unop_expr:\n");
  parse_tree_print(p->nd_unop_expr,
                   indent + INDENT_SPACES);
}



// Dot node names are "N_"<node address> and so are unique.
#define GET_DOT_NODE_NAME(s, p) sprintf((s), "N_%lu", (unsigned long) (p));



void parse_tree_dot_binary_op(
  PARSE_TREE_NODE *p
)
{
  char dot_name_left[MAX_STR];
  char dot_name_right[MAX_STR];
  char dot_name_node[MAX_STR];
  GET_DOT_NODE_NAME(dot_name_left, p->nd_binop_left_expr);
  GET_DOT_NODE_NAME(dot_name_right, p->nd_binop_right_expr);
  GET_DOT_NODE_NAME(dot_name_node, p);
  // N563 [shape=record, label="{ordered choice|{<left>left|<right>right|}}"];
 printf("%s [shape=record, label=\"{%s|{<left>left|<right>right}}\"];\n",
        dot_name_node,
        parse_get_node_type_name(p->nd_type));
  printf("%s:left -> %s;\n", dot_name_node, dot_name_left);
  printf("%s:right -> %s;\n", dot_name_node, dot_name_right);
  parse_tree_node_to_dot(p->nd_binop_left_expr);
  parse_tree_node_to_dot(p->nd_binop_right_expr);
}



void parse_tree_dot_unary_op(
  PARSE_TREE_NODE *p
)
{
  char dot_name_node[MAX_STR];
  char dot_name_expr[MAX_STR];
  GET_DOT_NODE_NAME(dot_name_expr, p->nd_unop_expr);
  GET_DOT_NODE_NAME(dot_name_node, p);
  printf("%s [shape=record, label=\"{%s|{<expr>expr}}\"];\n",
         dot_name_node,
         parse_get_node_type_name(p->nd_type));
  printf("%s:expr -> %s;\n", dot_name_node, dot_name_expr);
  parse_tree_node_to_dot(p->nd_unop_expr);
}



void parse_tree_node_to_dot(
  PARSE_TREE_NODE *p
)
{
  char dot_name[MAX_STR];
  if (HAS_ANY_TYPE(p->nd_type, NC_BINARY_OP)) {
    parse_tree_dot_binary_op(p);
  } else if (HAS_ANY_TYPE(p->nd_type, NC_UNARY_OP)) {
    parse_tree_dot_unary_op(p);
  } else {
    switch (p->nd_type) {
      case NT_ASSIGN_OP:
        break;
      case NT_NUM_CONST:
      case NT_PATT_NUM_CONST:
        GET_DOT_NODE_NAME(dot_name, p);
        printf("%s [shape=record, label=\"{%lf}\"];\n",
               dot_name,
               p->nd_num_const);
        break;
      case NT_IF:
        break;
      default:
        break;
    }
  }
}



void parse_tree_to_dot(
  PARSE_TREE_NODE *p
)
{
  printf("digraph sputz_parse_tree {\n");
  parse_tree_node_to_dot(p);
  printf("}\n");
}



void parse_tree_print(
  PARSE_TREE_NODE *p,
  uint8_t indent
)
{
  if (NULL != p) {
    IND(1);
    printf("%s\n", parse_get_node_type_name(p->nd_type));
    if (HAS_ANY_TYPE(p->nd_type, NC_BINARY_OP)) {
      parse_print_binary_op(p, indent);
    } else if (HAS_ANY_TYPE(p->nd_type, NC_UNARY_OP)) {
      parse_print_unary_op(p, indent);
    } else {
      switch (p->nd_type) {
        case NT_ASSIGN_OP:
          IND(1);
          printf("-assign_var  : %s\n",
                 p->nd_assign_var_name);
          IND(1);
          printf("-assign_expr :\n");
          parse_tree_print(p->nd_assign_expr,
                           indent + INDENT_SPACES);
          break;
        case NT_NUM_CONST:
        case NT_PATT_NUM_CONST:
          IND(1);
          printf("-num_const: %lf\n", p->nd_num_const);
          break;
        case NT_IF:
          IND(1);
          printf("-if_test     :\n");
          parse_tree_print(p->nd_if_test,
                           indent + INDENT_SPACES);
          IND(1);
          printf("-then_branch :\n");
          parse_tree_print(p->nd_if_then_branch,
                           indent + INDENT_SPACES);
          IND(1);
          printf("-else_branch :\n");
          parse_tree_print(p->nd_if_else_branch,
                           indent + INDENT_SPACES);
          break;
        default:
          IND(1);
          printf("-unprintable\n");
          break;
      }
    }
  }
}



// Not yet implemented.
#define ERR_NYI(pstate)                                         \
  do {                                                          \
    sprintf((pstate)->pst_err_msg, "%s : NYI.", __FUNCTION__);  \
    (pstate)->pst_status = S_PARSE_ERROR;                       \
    longjmp((pstate)->pst_abort, 1);                            \
  } while (0)



// Scan the next lexical unit and store in PARSE_STATE
void parse_scan_lx_unit(
  PARSE_STATE *pstate
)
{
  uint32_t lex_return_code;
  if (LX_SCAN_OK != (lex_return_code = lx_scan_next(&pstate->pst_input,
                                                    &pstate->pst_lookahead,
                                                    pstate->pst_pstrtab))) {
    pstate->pst_status = S_LEX_ERROR;
    sprintf(pstate->pst_err_msg,
            "Scanning error on line:%d col: %d - %s. Unable to read.",
            pstate->pst_lookahead.lex_line_n,
            pstate->pst_lookahead.lex_col_n,
            scode_name(lex_return_code));
    longjmp(pstate->pst_abort, 1);
  }
}



PARSE_TREE_NODE *parse_alloc_node(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *p;
  p = stkalloc_get_mem(pstate->pst_pmem,
                       sizeof(PARSE_TREE_NODE));
  if (NULL == p) {
    pstate->pst_status = S_MEM_OVERFLOW;
    // Maybe some better diagostics in the future.
    strcpy(pstate->pst_err_msg,
           "Memory overflow.");
    longjmp(pstate->pst_abort, 1);
  }
  zero_mem(p, sizeof(PARSE_TREE_NODE));
  return p;
}



void parse_expect(
  TAGGED_ENUM lx_type,
  PARSE_STATE *pstate
)
{
  if(lx_type != pstate->pst_lookahead.lex_type) {
    pstate->pst_status = S_LEX_ERROR;
    sprintf(pstate->pst_err_msg,
            "Scanning error. Expected %s, but found: %s, line: %d, col: %d",
            lx_name(lx_type),
            lx_name(pstate->pst_lookahead.lex_type),
            pstate->pst_lookahead.lex_line_n,
            pstate->pst_lookahead.lex_col_n);
    longjmp(pstate->pst_abort, 1);
  }
}



void parse_expect_and_skip(
  TAGGED_ENUM lx_type,
  PARSE_STATE *pstate
)
{
  parse_expect(lx_type, pstate);
  parse_scan_lx_unit(pstate);
}



PARSE_TREE_NODE *parse_create_binop(
  TAGGED_ENUM op,
  PARSE_TREE_NODE *pleft,
  PARSE_TREE_NODE *pright,
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result;
  result = parse_alloc_node(pstate);
  result->nd_type = op;
  result->nd_binop_left_expr = pleft;
  result->nd_binop_right_expr = pright;
  return result;
}



PARSE_TREE_NODE *parse_create_unop(
  TAGGED_ENUM op,
  PARSE_TREE_NODE *puexpr,
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result;
  result = parse_alloc_node(pstate);
  result->nd_type = op;
  result->nd_unop_expr = puexpr;
  return result;
}



// Parser entry point.
PARSE_TREE_NODE *parse_sputz_program(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  result = parse_seq_expr(pstate);
  parse_expect(L_EOF, pstate);
  return result;
}



// seq-expr =  expr [';' seq-expr]
PARSE_TREE_NODE *parse_seq_expr(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  result = parse_expr(pstate);
  while (L_SEQ == pstate->pst_lookahead.lex_type) {
    PARSE_TREE_NODE *pleft = result;
    PARSE_TREE_NODE *pright = NULL;
    // Skip ';'
    parse_scan_lx_unit(pstate);
    pright = parse_expr(pstate);
    result = parse_create_binop(NT_SEQ_OP, pleft, pright, pstate);
  }
  return result;
}



// expr = ifExpr | valueExpr | assignExpr
PARSE_TREE_NODE *parse_expr(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  if (L_IF_KW == pstate->pst_lookahead.lex_type) {
    result = parse_if_expr(pstate);
  } else if (L_VALUE_KW == pstate->pst_lookahead.lex_type) {
    result = parse_value_expr(pstate);
  } else {
    result = parse_assign_expr(pstate);
  }
  return result;
}



// assignExpr = varName ':=' tupleExpr
PARSE_TREE_NODE *parse_assign_expr(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  PARSE_TREE_NODE *expr = NULL;
  parse_expect(L_VAR_NAME, pstate);
  result = parse_alloc_node(pstate);
  result->nd_type = NT_ASSIGN_OP;
  // No strcpy necessary since all names permanently reside in the STRTAB.
  result->nd_assign_var_name = pstate->pst_lookahead.lex_pvar_name;
  // Skip L_VAR_NAME
  parse_scan_lx_unit(pstate);
  // Expect ':=' then skip over it.
  parse_expect_and_skip(L_ASSIGN, pstate);
  result->nd_assign_expr = parse_tuple_expr(pstate);
  return result;
}



// ifExpr = 'if' tupleExpr 'then' expr 'else' expr
PARSE_TREE_NODE *parse_if_expr(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  parse_expect_and_skip(L_IF_KW, pstate);
  result = parse_alloc_node(pstate);
  result->nd_type = NT_IF;
  result->nd_if_test = parse_tuple_expr(pstate);
  parse_expect_and_skip(L_THEN_KW, pstate);
  result->nd_if_then_branch = parse_tuple_expr(pstate);
  parse_expect_and_skip(L_ELSE_KW, pstate);
  result->nd_if_else_branch = parse_tuple_expr(pstate);
  return result;
}



// valueExpr = 'valueis' tupleExpr
PARSE_TREE_NODE *parse_value_expr(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  PARSE_TREE_NODE *puexpr = NULL;
  parse_expect_and_skip(L_VALUE_KW, pstate);
  puexpr = parse_tuple_expr(pstate);
  result = parse_create_unop(NT_VALUE, puexpr, pstate);
  return result;
}



// tupleExpr = tupleComponent (tupleCatOp tupleComponent )*
PARSE_TREE_NODE *parse_tuple_expr(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  result = parse_tuple_component(pstate);
  while (L_TUPLECAT == pstate->pst_lookahead.lex_type) {
    PARSE_TREE_NODE *pleft = result;
    PARSE_TREE_NODE *pright = NULL;
    // Skip ','
    parse_scan_lx_unit(pstate);
    pright = parse_tuple_component(pstate);
    result = parse_create_binop(NT_TUPLECAT_OP, pleft, pright, pstate);
  }
  return result;
}



// tupleComponent = orTerm (orOp orTerm)*
PARSE_TREE_NODE *parse_tuple_component(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  result = parse_or_term(pstate);
  while (L_OR_KW == pstate->pst_lookahead.lex_type) {
    PARSE_TREE_NODE *pleft = result;
    PARSE_TREE_NODE *pright = NULL;
    // Skip 'or'
    parse_scan_lx_unit(pstate);
    pright = parse_or_term(pstate);
    result = parse_create_binop(NT_OR_OP, pleft, pright, pstate);
  }
  return result;
}



// orTerm = andTerm (andOp andTerm)*
PARSE_TREE_NODE *parse_or_term(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  DBG_PRINT_FN;
  result = parse_and_term(pstate);
  while (L_AND_KW == pstate->pst_lookahead.lex_type) {
    PARSE_TREE_NODE *pleft = result;
    PARSE_TREE_NODE *pright = NULL;
    // Skip 'or'
    parse_scan_lx_unit(pstate);
    pright = parse_and_term(pstate);
    result = parse_create_binop(NT_AND_OP, pleft, pright, pstate);
  }
  return result;
}



// andTerm = [notOp] compareTerm (compareOp compareTerm)?
PARSE_TREE_NODE *parse_and_term(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  DBG_PRINT_FN;
  bool negated = false;
  if (L_NOT_KW == pstate->pst_lookahead.lex_type) {
    negated = true;
    // Skip 'not'
    parse_scan_lx_unit(pstate);
  }
  result = parse_compare_term(pstate);
  printf("About to check for relational.\n");
  if (HAS_ANY_TYPE(pstate->pst_lookahead.lex_type, LC_REL_OP)) {
    printf("Relational found.\n");
    PARSE_TREE_NODE *pleft = result;
    PARSE_TREE_NODE *pright = NULL;
    TAGGED_ENUM lx_rel_op = pstate->pst_lookahead.lex_type;
    TAGGED_ENUM nt_rel_op;
    // Skip relational operator
    printf("Relational: %s\n", lx_name(lx_rel_op));
    parse_scan_lx_unit(pstate);
    printf("Lexical unit following: %s\n", lx_name(pstate->pst_lookahead.lex_type));
    switch (lx_rel_op) {
      case L_GT:
        nt_rel_op = NT_GT_OP;
        break;
      case L_GE:
        nt_rel_op = NT_GE_OP;
        break;
      case L_LT:
        printf("case L_LT\n");
        nt_rel_op = NT_LT_OP;
        break;
      case L_LE:
        nt_rel_op = NT_LE_OP;
        break;
      case L_EQ:
        nt_rel_op = NT_EQ_OP;
        break;
      case L_NE:
        nt_rel_op = NT_NE_OP;
        break;
    }
    pright = parse_compare_term(pstate);
    result = parse_create_binop(nt_rel_op, pleft, pright, pstate);
  }
  if (negated) {
    result = parse_create_unop(NT_NOT_OP, result, pstate);
  }
  return result;
}



// compareTerm = term (addOp term)*
PARSE_TREE_NODE *parse_compare_term(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = parse_number(pstate);
  return result;
}



PARSE_TREE_NODE *parse_term(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_factor(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_number(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  parse_expect(L_NUMBER, pstate);
  result = parse_alloc_node(pstate);
  result->nd_type = NT_NUM_CONST;
  result->nd_num_const = pstate->pst_lookahead.lex_number;
  // Skip to next lexical unit.
  parse_scan_lx_unit(pstate);
  return result;
}



PARSE_TREE_NODE *parse_symbol(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_var_name(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_data_constructor(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_application(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_closure(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_pattern_alternative(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_pattern_tuple(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_pattern_tuple_component(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_pattern_factor(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



PARSE_TREE_NODE *parse_pattern_data_constructor(
  PARSE_STATE *pstate
)
{
  PARSE_TREE_NODE *result = NULL;
  ERR_NYI(pstate);
  return result;
}



TAGGED_ENUM parse_init(
  PARSE_STATE *pstate,
  char test_type,
  char *arg,
  ARENA **ppmem,
  STRTAB **ppstrtab
)
{
  TAGGED_ENUM result = S_OK;
  // init_gr() is temporary here so don't check for errors when calling it.
  init_gr(test_type, arg, &pstate->pst_input);
  ABORT_ON_NULL(*ppmem = stkalloc_new_arena(MIB(10)), INIT_MEM_FAIL0);
  ABORT_ON_NULL(*ppstrtab = strtab_new(*ppmem), INIT_MEM_FAIL1);
  pstate->pst_status = lx_scan_next(&pstate->pst_input,
                                    &pstate->pst_lookahead,
                                    pstate->pst_pstrtab);
  result = pstate->pst_status;
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



void parse_fin(
  PARSE_STATE *pstate
)
{
  gr_close(&pstate->pst_input);
  pstate->pst_status = S_OK;
  strcpy(pstate->pst_err_msg, "");
  pstate->pst_lookahead.lex_type = L_UNKNOWN;
  if (NULL != pstate->pst_pmem) {
    stkalloc_free_arena(pstate->pst_pmem);
  }
}



#if defined(TEST_PARSE)

int main(
  int argc,
  char **argv
)
{
  GEN_READ gr;
  PARSE_STATE pstate;
  PARSE_TREE_NODE *proot;
  int retval = 0;
  bool print_parse_tree_as_outline = false;
  bool print_parse_tree_as_dot_graph = false;
  bool initialized = false;
  TAGGED_ENUM init_status_code = S_OK;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s [-o | -d] -s '<text>'\n", argv[0]);
    retval = 1;
  } else {
    int i;
    for (i = 1; !retval && i < argc;) {
      if (STREQ(argv[i], "-s")) {
        if (scode_is_error(init_status_code = parse_init(&pstate,
                                                         argv[i][1],
                                                         argv[i + 1],
                                                         &pstate.pst_pmem,
                                                         &pstate.pst_pstrtab))) {
          fprintf(stderr, "Initialization failure: %s.\n",
                  scode_name(init_status_code));
          retval = 2;
        } else {
          // Skip past '-s' and '<text>'
          i += 2;
          initialized = true;
        }
      } else if (STREQ(argv[i], "-d")) {
        print_parse_tree_as_dot_graph = true;
        print_parse_tree_as_outline = false;
        i += 1;
      } else if (STREQ(argv[i], "-o")) {
        print_parse_tree_as_outline = true;
        print_parse_tree_as_dot_graph = false;
        i += 1;
      } else {
        fprintf(stderr, "Unknown command line option: %s\n", argv[i]);
        retval = 3;
      }
    }
  }
  if (!retval) {
    if (!setjmp(pstate.pst_abort)) {
      proot = parse_sputz_program(&pstate);
      if (print_parse_tree_as_outline) {
        parse_tree_print(proot, 0);
      } else if (print_parse_tree_as_dot_graph) {
        parse_tree_to_dot(proot);
      } else {
        printf("Parse successful.\n");
      }
    } else {
      fprintf(stderr, "%s\n", pstate.pst_err_msg);
      retval = 4;
    }
    parse_fin(&pstate);
  }
  return retval;
}

#endif
