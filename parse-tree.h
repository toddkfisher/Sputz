#pragma once

// "NT_" == "Node Type"
// "NC" == "Node Class"

enum {
   NC_UNARY_OP = 0x01,
   NC_BINARY_OP = 0x02,
   NC_REL_OP = 0x04,
   NC_ADD_OP = 0x08,
   NC_MUL_OP = 0x10
};

#include "enum-int.h"
enum {
#include "enum-PT.h"
};

// Parse trees stored here.
typestruct(PARSE_TREE_NODE);
struct PARSE_TREE_NODE {
  TAGGED_ENUM nd_type;
  union {
    // NT_(any binary operator)
    struct {
      PARSE_TREE_NODE *nd_binop_left_expr;
      PARSE_TREE_NODE *nd_binop_right_expr;
    };
    // NT_(any unary operator)
    PARSE_TREE_NODE *nd_unop_expr;
    // NT_IF
    struct {
      PARSE_TREE_NODE *nd_if_test;
      PARSE_TREE_NODE *nd_if_then_branch;
      PARSE_TREE_NODE *nd_if_else_branch;
    };
    // NT_FN_DEF
    struct {
      uint32_t nd_n_patt_result_pairs;
      PARSE_TREE_NODE **nd_patt_result_array;
    };
    // NT_CLOSUREIZE
    TAGGED_ENUM nd_closureize_token;
    // NT_ASSIGN_OP
    struct {
      PARSE_TREE_NODE *nd_assign_target;
      PARSE_TREE_NODE *nd_assign_expr;
    };
    // NT_VAR_REF
    // NT_PATT_VAR_REF
    struct {
      // I wonder if anyone will write 'outer' more than 4294967296 times?
      uint16_t nd_outer_count;
      char *nd_pvar_name;
    };
    // NT_NUM_CONST
    // NT_PATT_NUM_CONST
    double nd_num_const;
    // NT_SYM_CONST
    // NT_PATT_SYM
    char *nd_psym_name;
    // NT_TAGGED_TUPLE, NT_PATT_TAGGED_TUPLE
    struct {
      PARSE_TREE_NODE *nd_tagged_sym;
      PARSE_TREE_NODE *nd_tagged_expr;
    };
    // NT_PATT_BIND_VAR
    struct {
      uint16_t nd_bind_outer_count;
      char *nd_bind_var_name;
    };
    // NT_PATT_APPLY
    struct {
      PARSE_TREE_NODE *nd_patt_closure;
      PARSE_TREE_NODE *nd_patt_appl_args;
    };
    // NT_PATT_RESULT_PAIR
    struct {
      PARSE_TREE_NODE *nd_pattres_patt_expr;
      PARSE_TREE_NODE *nd_pattres_res_expr;
    };
    // NT_SIMPLE_CLOSURE, NT_PATT_CLOSURE
    PARSE_TREE_NODE *nd_closure_expr;
    // NT_APPLY
    struct {
      PARSE_TREE_NODE *nd_app_closure;
      PARSE_TREE_NODE *nd_app_args_expr;
    };
    // NT_VALUE
    PARSE_TREE_NODE *nd_value_expr;
  };
};

// This struct just bundles lookhead, character stream, and status info together.
typestruct(PARSE_STATE);
struct PARSE_STATE {
  // Input source.
  GEN_READ pst_input;
  // Next input to examine.
  LEX_UNIT pst_lookahead;
  // Current status of parse.
  TAGGED_ENUM pst_status;
  // Error message if pst_status != PSTAT_OK.  Empty otherwise.
  char pst_err_msg[MAX_STR];
  // Dynamic memory allocation arena for string table, parse tree nodes, etc.
  ARENA *pst_pmem;
  // Location to jump to if code can't be compiled.
  jmp_buf pst_abort;
  // Hash table for string constants, variable names, symbol names, etc.
  STRTAB *pst_pstrtab;
};

#define IS_OPERATOR_NTYPE(lx_type, op_type) ((NT_BEGIN_##lx_type##_OP) < (op_type) && (op_type) < (NT_END_##lx_type##_OP))
