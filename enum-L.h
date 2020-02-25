// "L" == "Lexical Unit"
// "KW" == "Keyword"
ENUM(L_NUMBER),
ENUM(L_SYMBOL),
ENUM(L_VAR_NAME),
ENUM(L_IF_KW),
ENUM(L_THEN_KW),
ENUM(L_ELSE_KW),
ENUM(L_AND_KW),
ENUM(L_OR_KW),
ENUM(L_NOT_KW),
ENUM(L_BIND_KW),
ENUM(L_OUTER_KW),
ENUM(L_ALSO_KW),
ENUM(L_NULL_KW),
ENUM(L_VALUE_KW),
ENUM(L_SEQ),          // ';'  Syntax
ENUM(L_LPAREN),
ENUM(L_RPAREN),
ENUM(L_FN_BEGIN),     // '{'  Syntax
ENUM(L_FN_END),       // '}'  Syntax
ENUM(L_TUPLECAT),     // ','  Operator
ENUM(L_GUARD),        // '!!' Syntax
ENUM(L_SUCH_THAT),    // '!'  Syntax
ENUM(L_RESULT),       // '=>' Syntax
ENUM(L_ASSIGN),       // ':=' Syntax
ENUM(L_PLUS),         // '+'  Operator
ENUM(L_MINUS),        // '-'  Operator
ENUM(L_TIMES),        // '*'  Operator
ENUM(L_POWER),        // '**' Operator
ENUM(L_DIVIDE),       // '/'  Operator
ENUM(L_MOD),          // '%'  Operator
ENUM(L_CLOSUREIZE),   // '`'  Syntax
ENUM(L_PATTERN_ALT),  // '|'  Syntax
ENUM(L_UNKNOWN),
ENUM(L_EOF)
