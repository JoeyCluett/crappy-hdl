#pragma once

#include <src/lexical-token.h>

#include <vector>

const int BuiltinRef_match   = 0;
const int BuiltinRef_cmqeq   = 1;
const int BuiltinRef_cmpneq  = 2;
const int BuiltinRef_decoder = 3;
const int BuiltinRef_cla     = 4; // carry lookahead adder
const int BUiltinRef_rca     = 5; // ripple-carry adder

const int FunctionRef_last    = 0;
const int FunctionRef_first   = 1;
const int FunctionRef_str     = 2;
const int FunctionRef_println = 3;
const int FunctionRef_print   = 4;
const int FunctionRef_size    = 5;
const int FunctionRef_typestr = 6; // type-string

//
// return reference to token that comes before given token
//
const LexerToken_t& parse_util_previous_token(std::vector<LexerToken_t>::const_iterator& token_iter);

//
// return whether given token is a builtin module reference
//
const bool parse_util_token_is_builtin(const std::vector<char>& src, const LexerToken_t& tok);

//
// return whether given token is a built-in function reference
//
const bool parse_util_token_is_function(const std::vector<char>& src, const LexerToken_t& tok);

//
// return the const int identifier associated with given function token
// does NOT perform error checking
//
const int parse_util_token_function_ident_fast(const std::vector<char>& src, const LexerToken_t& tok);

//
// return the const int identifier associated with given builtin-module token
// does NOT perform error checking
//
const int parse_util_token_builtin_ident_fast(const std::vector<char>& src, const LexerToken_t& tok);

//
// return whether given token is an operator
//
const bool parse_util_is_operator(const LexerToken_t& tok);

//
// return whether minus operator is a binary operator
// assumes the given token_iter points to a minus token
//
const bool parse_util_minus_is_unary(std::vector<LexerToken_t>::const_iterator& token_iter);


