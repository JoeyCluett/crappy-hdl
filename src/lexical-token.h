#pragma once

#include <string>
#include <vector>

const int LexerToken_UNKNOWN      = -1;

const int LexerToken_KW_requires  =  0;
const int LexerToken_KW_global    =  1;
const int LexerToken_KW_integer   =  2; // typename
const int LexerToken_KW_module    =  3;
const int LexerToken_KW_out_ports =  4;
const int LexerToken_KW_in_ports  =  5;
const int LexerToken_KW_start     =  6;
const int LexerToken_KW_end       =  7;
const int LexerToken_KW_string    =  8; // typename
const int LexerToken_KW_header    =  9;
const int LexerToken_KW_empty     = 10;
const int LexerToken_KW_void      = 11;
const int LexerToken_KW_uinteger  = 12; // typename
const int LexerToken_KW_local     = 13;
const int LexerToken_KW_builtin   = 14;

const int LexerToken_VarName          = 15;
const int LexerToken_BitLiteralMulti  = 16;
const int LexerToken_BitLiteralSingle = 17;

#define NVALUE (LexerToken_BitLiteralSingle + 1)

const int LexerToken_Syntax_Semicolon = NVALUE +  0; // ;
const int LexerToken_Syntax_Colon     = NVALUE +  1; // :
const int LexerToken_Syntax_Assign    = NVALUE +  2; // =    special
const int LexerToken_Syntax_Period    = NVALUE +  3; // .
const int LexerToken_Syntax_LBracket  = NVALUE +  4; // [
const int LexerToken_Syntax_RBracket  = NVALUE +  5; // ]
const int LexerToken_Syntax_Comma     = NVALUE +  6; // ,
const int LexerToken_Syntax_Star      = NVALUE +  7; // *
const int LexerToken_Syntax_Dollar    = NVALUE +  8; // $
const int LexerToken_Syntax_LParen    = NVALUE +  9; // (
const int LexerToken_Syntax_RParen    = NVALUE + 10; // )
const int LexerToken_Syntax_Invert    = NVALUE + 11; // ~
const int LexerToken_Syntax_Not       = NVALUE + 12; // !     special
const int LexerToken_Syntax_LBrace    = NVALUE + 13; // {
const int LexerToken_Syntax_RBrace    = NVALUE + 14; // }
const int LexerToken_Syntax_At        = NVALUE + 15; // @

const int LexerToken_Syntax_GrThan    = NVALUE + 16; // >     special
const int LexerToken_Syntax_LsThan    = NVALUE + 17; // <     special
const int LexerToken_Syntax_GrEq      = NVALUE + 18; // >=
const int LexerToken_Syntax_LsEq      = NVALUE + 19; // <=
const int LexerToken_Syntax_Equiv     = NVALUE + 20; // ==
const int LexerToken_Syntax_NotEquiv  = NVALUE + 21; // !=

const int LexerToken_Syntax_Pipe      = NVALUE + 22; // |   bitwise OR
const int LexerToken_Syntax_Ampersand = NVALUE + 23; // &   bitwise AND
const int LexerToken_Syntax_Caret     = NVALUE + 24; // ^   bitwise XOR

const int LexerToken_Syntax_Plus      = NVALUE + 25; // +
const int LexerToken_Syntax_Minus     = NVALUE + 26; // -

#undef NVALUE
#define NVALUE (LexerToken_Syntax_Minus + 1)

const int LexerToken_NumberDec        = NVALUE + 0; // 0, 1, 2, 2321212
const int LexerToken_NumberHex        = NVALUE + 1; // 0xff, 0x1234
const int LexerToken_NumberBin        = NVALUE + 2; // 0b001010010
const int LexerToken_StringLiteral    = NVALUE + 3;

#undef NVALUE

struct LexerToken_t {
    int type;
    int start;
    int end;
};

const std::string lexer_token_name(int token_type);

const std::string lexer_token_value(const LexerToken_t& token, const std::vector<char>& src);

const std::string lexer_token_name_and_value(const LexerToken_t& token, const std::vector<char>& src);

size_t lexer_token_number_to_ulong(LexerToken_t token, const std::vector<char>& src);
long   lexer_token_number_to_long(LexerToken_t token, const std::vector<char>& src);

const std::string lexer_token_string_eval(LexerToken_t token, const std::vector<char>& src);

const bool lexer_token_is_number_type(const LexerToken_t& token);

const bool lexer_token_is_typename(const LexerToken_t& token);

const bool lexer_token_is_bitliteral_type(const LexerToken_t& token);

const size_t lexer_token_bitliteral_size(const LexerToken_t& token);

const std::string lexer_token_bitliteral_str_repr(const LexerToken_t& token);
