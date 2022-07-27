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

static std::vector<shunting_token_t> work_stack;
static std::vector<shunting_token_t> output_queue;

static void parse_module_error_print_wip(
        std::vector<shunting_token_t>& work_stack, 
        std::vector<shunting_token_t>& output_queue,
        const std::vector<char>& src);

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

    parse_module_error_print_wip(work_stack, output_queue, src);
    std::cout << "\n    token: " << lexer_token_name_and_value(token, src) << std::endl;


    switch(state_current) {
    case state_expect_start:
        //std::cout << "parse_module_body : state_expect_start\n" << std::flush;
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
        //std::cout << "parse_module_body : state_default\n" << std::flush;
        return parse_module_body_default_state(rt, token, token_iter, src, filename, module_ptr);

    case state_expr_default:
        //std::cout << "parse_module_body : state_expr_default\n" << std::flush;
        if(parse_shunt(rt, token, token_iter, src, filename, module_ptr, work_stack, output_queue, shunt_flag_normal)) {
            state_current = state_default;
        }
        return false;
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

    //std::cout << "    parse_module_body_default_state\n" << std::flush;
    
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
            token_iter++;
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
        work_stack.clear();
        output_queue.clear();
        //token_iter++;
        return false;

    default:
        // TODO : error?
        //token_iter++;
        throw_parse_error(
                "Unknown token [" + lexer_token_name_and_value(token, src) + "]", 
                filename, src, token.start, token);
        break;
    }
}

static void parse_module_error_print_wip(
        std::vector<shunting_token_t>& work_stack, 
        std::vector<shunting_token_t>& output_queue,
        const std::vector<char>& src) {

    //

    std::cout << "Output Queue {\n";    
    for(auto& st : output_queue) {
        std::cout << "    " << lexer_token_name_and_value(st.token, src) << std::endl;
    }
    std::cout << "}\n" << std::flush;

    std::cout << "Work Stack {\n";
    for(auto& st : work_stack) {
        std::cout << "    " << lexer_token_name_and_value(st.token, src) << std::endl;
    }
    std::cout << "}\n";
    std::cout << std::flush;

}
