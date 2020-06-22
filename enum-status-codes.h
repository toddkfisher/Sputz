// "ST" == "strtab"
ENUM(ST_NEW_STR_INSERTED, 0, 0),
ENUM(ST_STR_EXISTS, 1, 0),
// "S_" == "status"
ENUM(S_OK, 2, 0),
// "LX_" == "lex status"
ENUM(LX_SCAN_OK, 3, 0),
// Everything that follows is an error status
ENUM(LX_UNABLE_TO_SAVE_NAME_OR_STRING, 4, SC_ERROR),
ENUM(ST_UNABLE_TO_INSERT, 5, SC_ERROR),
ENUM(S_MEM_OVERFLOW, 6, SC_ERROR),
ENUM(S_PARSE_ERROR, 7, SC_ERROR),
ENUM(S_LEX_ERROR, 8, SC_ERROR),
ENUM(LX_UNKNOWN_CHAR, 9, SC_ERROR),
ENUM(S_UNABLE_TO_OPEN_FILE, 10, SC_ERROR),
ENUM(S_UNABLE_TO_OPEN_STRING, 11, SC_ERROR),
ENUM(S_UNABLE_TO_OPEN_READLINE, 12, SC_ERROR),
ENUM(S_UNKNOWN_INPUT_TYPE, 13, SC_ERROR),
ENUM(S_UNABLE_TO_CREATE_ARENA, 14, SC_ERROR),
ENUM(S_UNABLE_TO_CREATE_STRTAB, 15, SC_ERROR),
