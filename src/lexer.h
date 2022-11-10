#pragma once

#include <vector>
#include <string>

typedef std::vector<char>::const_iterator src_iter_t;
typedef const std::vector<char>           src_t;

enum class token_type_t {
    UNKNOWN,
    keyword_integer,
    keyword_uinteger,
    keyword_string,
    keyword_bit,
    keyword_module,
    keyword_out,
    keyword_in,
    keyword_start,
    keyword_end,
    keyword_void,
    keyword_local,
    keyword_ref,
    keyword_builtin,
    keyword_true_,          // true literal
    keyword_false_,         // false literal
    variable_name,
    bit_literal, // @00_1010_0101 etc.
    semicolon,   // ;
    colon,       // :
    period,      // .
    lbracket,
    rbracket,
    comma,       // ,
    dollar,
    lparen,
    rparen,
    lbrace,
    rbrace,
    greater_than, // >
    less_than,    // <
    greater_eq,   // >=
    less_eq,      // <=
    equiv,        // ==
    not_equiv,    // != 
    pipe,         // |  bitwise OR
    ampersand,    // &  bitwise AND
    caret,        // ^
    plus,         // +
    minus,        // -
    divide,       // /
    star,         // *
    assign,       // =
    invert,       // ~ bitwise NOT
    not_,         // ! boolean NOT    
    number_dec,
    number_hex,
    number_bin,
    string_literal,
    unary_negative, // - lexer assumes always minus, parser can change to negate
    bit_assign,     // :=
};

struct token_t {
    token_type_t type;
    int start;
    int end;
};

typedef std::vector<token_t>::iterator token_iterator_t;

//
// no return type because this function throws exceptions on error
//
void lexical_analyze(src_t& src, const std::string& filename, std::vector<token_t>& tkns);

typedef std::string string_t;

const string_t lexer_token_type(token_type_t);
const string_t lexer_token_value(const token_t& tok, src_t& src);
const string_t lexer_token_desc(const token_t& tok, src_t& src);
void print_lexer_tokens(std::vector<token_t>& tkns);


