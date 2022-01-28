#pragma once

#include <string>
#include <vector>

const int LexerToken_UNKNOWN      = -1;

const int LexerToken_KW_requires  =  0;
const int LexerToken_KW_global    =  1;
const int LexerToken_KW_integer   =  2;
const int LexerToken_KW_module    =  3;
const int LexerToken_KW_out_ports =  4;
const int LexerToken_KW_in_ports  =  5;
const int LexerToken_KW_start     =  6;
const int LexerToken_KW_end       =  7;
const int LexerToken_KW_string    =  8;
const int LexerToken_KW_header    =  9;
const int LexerToken_KW_empty     = 10;

const int LexerToken_VarName      = 11;

#define NVALUE (LexerToken_VarName + 1)

const int LexerToken_Syntax_Semicolon = NVALUE +  0; // ;
const int LexerToken_Syntax_Colon     = NVALUE +  1; // :
const int LexerToken_Syntax_Assign    = NVALUE +  2; // =
const int LexerToken_Syntax_Period    = NVALUE +  3; // .
const int LexerToken_Syntax_LBracket  = NVALUE +  4; // [
const int LexerToken_Syntax_RBracket  = NVALUE +  5; // ]
const int LexerToken_Syntax_Comma     = NVALUE +  6; // ,
const int LexerToken_Syntax_Star      = NVALUE +  7; // *
const int LexerToken_Syntax_Dollar    = NVALUE +  8; // $
const int LexerToken_Syntax_LParen    = NVALUE +  9; // (
const int LexerToken_Syntax_RParen    = NVALUE + 10; // )

#undef NVALUE
#define NVALUE (LexerToken_Syntax_RParen + 1)

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
