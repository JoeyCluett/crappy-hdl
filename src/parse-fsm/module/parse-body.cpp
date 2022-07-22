#include "parse-body.h"

#include <iostream>
#include <vector>
#include <string>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>
#include <src/parse-fsm/module/expr/parse-shunt.h>

static const int state_expect_start = 0;
static const int state_default      = 1; // iterate through until we hit 'end'
static const int state_expr_default = 2;

static int state_current = state_expect_start;

static std::stack<shunting_token_t> work_stack;
static std::vector<shunting_token_t> output_queue;

static bool parse_module_body_default_state(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr);

bool parse_module_body(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    switch(state_current) {
    case state_expect_start:
        if(token.type == LexerToken_KW_start) {
            state_current = state_default;
            token_iter++;
            return false;
        } else {
            throw_parse_error(
                "Expecting 'start'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_default:
        return parse_module_body_default_state(rt, token, token_iter, src, filename, module_ptr);

    case state_expr_default:
        if(parse_shunt(rt, token, token_iter, src, filename, module_ptr, work_stack, output_queue, shunt_flag_normal)) {
            state_current = state_default;
        }
        break;

    default:
        break; // TODO : catch internal error
    }


}

static bool parse_module_body_default_state(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    switch(token.type) {
    case LexerToken_KW_end:
        // check scope depth first
        if(module_ptr->parse.scope_depth == 0) {
            token_iter++;
            state_current = state_expect_start;
            return true;
        }
        else {
            module_ptr->parse.scope_depth--;
            // TODO : add to output queue
            return false;
        }
        break;

    case LexerToken_KW_start:
        module_ptr->parse.scope_depth++;
        // TODO : add to output queue
        return false;

    case LexerToken_VarName:
        state_current = state_expr_default;
        while(!work_stack.empty())
            work_stack.pop();
        output_queue.clear();
        //token_iter++;
        return false;

    default:
        // TODO : error?
        token_iter++;
        break;
    }
}
