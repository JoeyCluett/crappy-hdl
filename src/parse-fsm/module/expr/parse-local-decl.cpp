#include "parse-local-decl.h"

#include <vector>
#include <string>
#include <stdexcept>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>

#include <src/ir/opcodes.h>

static const int state_expect_local    = 0;
static const int state_expect_varname  = 1;
static const int state_expect_colon    = 2;
static const int state_expect_typename = 3;
static const int state_semic_or_assign = 4;
static const int state_init_value      = 5;
static const int state_end_semic       = 6;

static int state_current = state_expect_local;

//
// parse expression of the form:
//
// local varname : typename;
// local varname : typename = INIT_VALUE;
//

static LexerToken_t local_name;
static LexerToken_t local_type;
static LexerToken_t init_value;

static size_t gen_ir_for_new_local_decl(
        hdl_module_t* module_ptr,
        const std::vector<char>& src,
        const LexerToken_t& local_name, 
        const LexerToken_t& local_type);

static void gen_ir_for_local_init(
        hdl_module_t* module_ptr,
        const std::vector<char>& src,
        const LexerToken_t& local_name, 
        const LexerToken_t& local_type,
        const LexerToken_t& init_value);

bool parse_local_decl(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    switch(token.type == LexerToken_KW_local) {
    case state_expect_local:
        if(token.type == LexerToken_KW_local) {
            token_iter++;
            state_current = state_expect_varname;
            return false;
        }
        else {
            throw_parse_error(
                "Expected 'local'. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_expect_varname:
        if(token.type == LexerToken_VarName) {
            local_name = token;
            token_iter++;
            state_current = state_expect_colon;
            return false;
        }
        else {
            throw_parse_error(
                "Expected variable name. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_expect_colon:
        if(token.type == LexerToken_Syntax_Colon) {
            token_iter++;
            state_current = state_expect_typename;
            return false;
        }
        else {
            throw_parse_error(
                "Expected ':'. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_expect_typename:
        if(lexer_token_is_typename(token)) {
            local_type = token;
            token_iter++;
            state_current = state_semic_or_assign;
            return false;
        }
        else {
            throw_parse_error(
                "Expected type specifier. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_semic_or_assign:
        if(token.type == LexerToken_Syntax_Semicolon) {
            gen_ir_for_new_local_decl(module_ptr, local_name, local_type);
            token_iter++;
            state_current = state_expect_local;
            return true;
        }
        else if(token.type == LexerToken_Syntax_Assign) {
            gen_ir_for_new_local_decl(module_ptr, local_name, local_type);
            token_iter++;
            state_current = state_init_value;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting one of ';' or '='. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_init_value:

    case state_end_semic:
        if(token.type == LexerToken_Syntax_Semicolon) {
            token_iter++;
            state_current = state_expect_local;
            return true;
        }
        else {
            throw_parse_error(
                "Expecting ';'. Received token of type [" +
                lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    }

}

static size_t gen_ir_for_new_local_decl(
        hdl_module_t* module_ptr,
        const std::vector<char>& src,
        const LexerToken_t& local_name, 
        const LexerToken_t& local_type) {

    size_t idx = module_ptr->string_array.size();
    const std::string local_name_str = lexer_token_value(local_name, src);
    module_ptr->string_array.push_back(local_name_str);

    if(local_type.type == LexerToken_KW_integer) {
        
    }
    else if(local_type.type == LexerToken_KW_uinteger) {

    }
    else if(local_type.type == LexerToken_KW_string) {

    }
    else {
        throw std::runtime_error("unknown internal error");
    }
}

static void gen_ir_for_local_init(
        hdl_module_t* module_ptr,
        const std::vector<char>& src,
        const LexerToken_t& local_name, 
        const LexerToken_t& local_type,
        const LexerToken_t& init_value) {

    

}
