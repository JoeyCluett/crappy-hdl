#include "parse-util.h"

#include <src/lexical-token.h>

#include <vector>
#include <map>
#include <string>

#define BUILTIN_REFERENCE  0
#define FUNCTION_REFERENCE 1

static const std::map<std::string, std::pair<const int, const int>> function_and_builtin_lookup = {
    { "match",   { BUILTIN_REFERENCE, BuiltinRef_match   }},
    { "cmpeq",   { BUILTIN_REFERENCE, BuiltinRef_cmqeq   }},
    { "cmpneq",  { BUILTIN_REFERENCE, BuiltinRef_cmpneq  }},
    { "decoder", { BUILTIN_REFERENCE, BuiltinRef_decoder }},

    { "last",    { FUNCTION_REFERENCE, FunctionRef_last    }},
    { "first",   { FUNCTION_REFERENCE, FunctionRef_first   }},
    { "str",     { FUNCTION_REFERENCE, FunctionRef_str     }},
    { "println", { FUNCTION_REFERENCE, FunctionRef_println }},
    { "print",   { FUNCTION_REFERENCE, FunctionRef_print   }},
};

const int parse_util_token_function_ident_fast(const std::vector<char>& src, const LexerToken_t& tok) {
    std::string s = lexer_token_value(tok, src);
    return function_and_builtin_lookup.at(s).second;
}

const int parse_util_token_builtin_ident_fast(const std::vector<char>& src, const LexerToken_t& tok) {
    std::string s = lexer_token_value(tok, src);
    return function_and_builtin_lookup.at(s).second;
}

const bool parse_util_token_is_builtin(const std::vector<char>& src, const LexerToken_t& tok) {
    std::string s = lexer_token_value(tok, src);
    auto iter = function_and_builtin_lookup.find(s);
    if(iter == function_and_builtin_lookup.end()) {
        return false;
    }
    return iter->second.first == BUILTIN_REFERENCE;
}

const bool parse_util_token_is_function(const std::vector<char>& src, const LexerToken_t& tok) {
    std::string s = lexer_token_value(tok, src);
    auto iter = function_and_builtin_lookup.find(s);
    if(iter == function_and_builtin_lookup.end()) {
        return false;
    }
    return iter->second.first == FUNCTION_REFERENCE;
}

const LexerToken_t& parse_util_previous_token(std::vector<LexerToken_t>::const_iterator& token_iter) {
    return *(token_iter - 1);
}

const bool parse_util_is_operator(const LexerToken_t& tok) {
    switch(tok.type) {
    case LexerToken_Syntax_Period: // .
    case LexerToken_Syntax_Star: // *
    case LexerToken_Syntax_Invert: // ~
    case LexerToken_Syntax_Not: // !
    case LexerToken_Syntax_GrThan: // >
    case LexerToken_Syntax_LsThan: // <
    case LexerToken_Syntax_GrEq:  // >=
    case LexerToken_Syntax_LsEq:  // <=
    case LexerToken_Syntax_Equiv: // ==
    case LexerToken_Syntax_NotEquiv: // !=
    case LexerToken_Syntax_Pipe: // |
    case LexerToken_Syntax_Ampersand: // &
    case LexerToken_Syntax_Caret: // ^
    case LexerToken_Syntax_Plus: // +
    case LexerToken_Syntax_Minus: // -
    case LexerToken_Syntax_UnaryNegative: // - (but different from Minus)
        return true;
    default:
        return false;
    }
}

const bool parse_util_minus_is_unary(std::vector<LexerToken_t>::const_iterator& token_iter) {
    auto prev_token = parse_util_previous_token(token_iter);
    switch(prev_token.type) {
    case LexerToken_Syntax_LParen:
    case LexerToken_Syntax_LBrace:
    case LexerToken_Syntax_LBracket:
        return true;
    default:
        return parse_util_is_operator(parse_util_previous_token(token_iter));
    }
}



