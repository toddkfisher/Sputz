// "ST" == "strtab"
ENUM(ST_NEW_STR_INSERTED),
ENUM(ST_STR_EXISTS),
// "S_" == "status"
ENUM(S_OK),
// "LX_" == "lex status"
ENUM(LX_SCAN_OK),
//----------------------------------------------------------------------------------------------------------------------
// Everything that follows is an error status
ENUM(BEGIN_ERROR_MARKER),
ENUM(LX_UNABLE_TO_SAVE_NAME_OR_STRING),
ENUM(ST_UNABLE_TO_INSERT),
ENUM(S_MEM_OVERFLOW),
ENUM(S_PARSE_ERROR),
ENUM(S_LEX_ERROR),
ENUM(LX_UNKNOWN_CHAR),
ENUM(S_UNABLE_TO_OPEN_FILE),
ENUM(S_UNABLE_TO_OPEN_STRING),
ENUM(S_UNABLE_TO_OPEN_READLINE),
ENUM(S_UNKNOWN_INPUT_TYPE),
ENUM(S_UNABLE_TO_CREATE_ARENA),
ENUM(S_UNABLE_TO_CREATE_STRTAB)
