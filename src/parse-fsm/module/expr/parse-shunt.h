#pragma once

#include <vector>
#include <string>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>

const int shunting_token_type_none              =  0;
const int shunting_token_type_local             =  1;
const int shunting_token_type_operator          =  2;
const int shunting_token_type_function_sentinal =  3;
const int shunting_token_type_local_reference   =  4;
const int shunting_token_type_global_reference  =  5;
const int shunting_token_type_constant          =  6;
const int shunting_token_type_range_operator    =  7;
const int shunting_token_type_index_operator    =  8;
const int shunting_token_type_builtin           =  9;
const int shunting_token_type_function          = 10;
const int shunting_token_type_unary_operator    = 11;

struct shunting_token_t {
    int type;
    LexerToken_t token;
};

bool parse_shunt(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr);


