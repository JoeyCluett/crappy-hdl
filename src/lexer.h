#pragma once

#include <vector>
#include <string>
#include <utility>
#include <tuple>

typedef std::vector<char>::const_iterator src_iter_t;
typedef const std::vector<char>           src_t;

enum class token_type_t {
    UNKNOWN,
    keyword_integer,  // type
    keyword_uinteger, // ...
    keyword_string,   // ...
    keyword_vector,   // ...
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
    keyword_true_,  // true literal
    keyword_false_, // false literal
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
    function,  // one of a variety of native functions (see function_type_t below)
};

enum class function_type_t : uint8_t {
    UNKNOWN,
    push,
    last,
    print,
    cast,
};

struct token_t {
    token_type_t type;
    int start;
    int end;
};

typedef std::vector<token_t>::iterator token_iterator_t;

const bool operator==(const token_t& tok, token_type_t tt);

//
// no return type because this function throws exceptions on error
//
void lexical_analyze(src_t& src, const std::string& filename, std::vector<token_t>& tkns);

const bool lexer_token_is_typespec(const token_t& tok);

typedef std::string string_t;

const string_t lexer_token_type(token_type_t);
const string_t lexer_token_value(const token_t& tok, src_t& src);
const string_t lexer_token_desc(const token_t& tok, src_t& src);
void print_lexer_tokens(std::vector<token_t>& tkns);

size_t lexer_token_to_uinteger(const token_t& tok, struct parse_info_t& p);

std::tuple<bool, string_t, token_type_t> lexer_token_is_keyword(const std::string& s);
std::tuple<bool, string_t, function_type_t> lexer_token_is_function(const std::string& s);

