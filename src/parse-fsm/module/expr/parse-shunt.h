#pragma once

#include <vector>
#include <stack>
#include <string>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>

enum shunting_token_type_t {
    shunting_token_type_none         =  0, // placeholder
    shunting_token_type_local_reference  , // local reference, token is variable name
    shunting_token_type_field_reference  , // tokenized as variable name, parser figures out if its actually a field reference
    shunting_token_type_binary_operator  , // binary operator
    shunting_token_type_unary_operator   , // unary operator (one of '-', '!', '~')
    shunting_token_type_function_sentinal, // start of function call
    shunting_token_type_global_reference , // like type_local above, but this refers to a global type
    shunting_token_type_constant         , // one of constant numeric types
    shunting_token_type_string           , // string constant
    shunting_token_type_builtin_module   , // one of the builtin module types
    shunting_token_type_builtin_function , // one of the builtin function types
    shunting_token_type_end_scope        , // 'end' keyword
    shunting_token_type_lparen_sentinal  , // (
    shunting_token_type_rparen_sentinal  , // )
    shunting_token_type_lrange_sentinal  , // {
    shunting_token_type_rrange_sentinal  , // }
    shunting_token_type_lindex_sentinal  , // [
    shunting_token_type_rindex_sentinal  , // ]
    shunting_token_type_bitliteral       , // @'010....'
};

const int shunt_flag_normal       = 1;
const int shunt_flag_control_flow = 3;

struct shunting_token_t {
    shunting_token_type_t type;
    LexerToken_t token;

    union {
        int function_ident;
        int builtin_ident;
        hdl_module_t* module_ptr;
    };

};

bool parse_shunt(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr,
        std::vector<shunting_token_t>& work_stack,
        std::vector<shunting_token_t>& output_queue,
        const int flags);


