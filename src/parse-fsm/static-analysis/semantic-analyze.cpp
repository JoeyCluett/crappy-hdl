#include "semantic-analyze.h"

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stack>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>
#include <src/ir/opcodes.h>
#include <src/parse-fsm/parse-util.h>
#include <src/parse-fsm/module/expr/parse-shunt.h>

const uint32_t analyze_flag_temporary = (1u << 0u);
const uint32_t analyze_flag_integer   = (1u << 1u);
const uint32_t analyze_flag_signed    = (1u << 2u);
const uint32_t analyze_flag_unsigned  = (1u << 3u);
const uint32_t analyze_flag_string    = (1u << 4u);
const uint32_t analyze_flag_module_input_bit  = (1u << 5u);
const uint32_t analyze_flag_module_input_vec  = (1u << 6u);
const uint32_t analyze_flag_module_output_bit = (1u << 7u);
const uint32_t analyze_flag_module_output_vec = (1u << 8u);
const uint32_t analuze_flag_range_object    = (1u << 9u);
const uint32_t analyze_flag_index_specifier = (1u << 10u);

struct analyze_info_t {
    uint32_t flags;
    shunting_token_t stok;
};

static const bool parse_analyze_eval_local_field(
        HDL_Runtime_t* rt, 
        hdl_module_t* 
        module_ptr, 
        const std::string& local_name, 
        const std::string& field_name,
        std::map<std::string, hdl_module_t*>& local_modules,
        std::stack<analyze_info_t>& stack);

void semantic_analyze(
        HDL_Runtime_t* rt,
        hdl_module_t* module_ptr,
        std::vector<shunting_token_t>& output_queue, 
        shunting_token_t& stok, 
        const std::vector<char>& src,
        const std::string& filename) {

    return;

/*
    std::stack<analyze_info_t> stack;
    std::map<std::string, hdl_module_t*> local_modules; // local name, module pointer

    auto top_two_elements = [&stack]() -> std::pair<analyze_info_t, analyze_info_t> {
        analyze_info_t _0 = stack.top();
        stack.pop();
        analyze_info_t _1 = stack.top();
        stack.pop();
        return { _1, _0 };
    };

    auto top_element = [&stack]() -> analyze_info_t {
        analyze_info_t _0 = stack.top();
        stack.pop();
    };

    analyze_info_t ainfo;

    for(auto& st : output_queue) {

        switch(st.type) {
        case shunting_token_type_constant:
            ainfo.flags = analyze_flag_token | analyze_flag_unsigned | analyze_flag_literal;
            ainfo.stok = st;
            stack.push(ainfo);
            break;

        case shunting_token_type_string:
            ainfo.flags = analyze_flag_token | analyze_flag_string | analyze_flag_literal;
            ainfo.stok = st;
            stack.push(ainfo);
            break;

        case shunting_token_type_local_reference:
            ainfo.flags = analyze_flag_token | analyze_flag_local;
            ainfo.stok = st;
            stack.push(ainfo);
            break;

        case shunting_token_type_unary_operator:
        {
            if(stack.size() == 0ul) {
                // ERROR
            }

            auto top = top_element();

            if(st.token.type == LexerToken_Syntax_Invert) { // ~
                const unsigned int int_type_flag = top.flags & (analyze_flag_unsigned | analyze_flag_signed);
                if(int_type_flag) {
                    analyze_info_t _ai;
                    _ai.flags = int_type_flag | analyze_flag_temporary;
                    stack.push(_ai);
                }
                // TODO : add support for bit literals
                else {
                    // ERROR
                }

            }
            else if(st.token.type == LexerToken_Syntax_Not) { // !
                const unsigned int int_type_flag = top.flags & (analyze_flag_unsigned | analyze_flag_signed);
                if(int_type_flag) {
                    analyze_info_t _ai;
                    _ai.flags = int_type_flag | analyze_flag_temporary;
                    stack.push(_ai);
                }
                else {
                    // ERROR : cannot NOT a non-integer type
                }
            }
            else if(st.token.type == LexerToken_Syntax_UnaryNegative) { // -
                if(top.flags & (analyze_flag_unsigned | analyze_flag_signed)) {
                    analyze_info_t _ai;
                    _ai.flags = analyze_flag_signed | analyze_flag_temporary;
                    stack.push(_ai);
                }
                else {
                    // ERROR : cannot negate a non-integer type
                }
            }
            else {
                throw_parse_error(
                        "unrecognized unary operator [" + lexer_token_name_and_value(st.token, src) + "]", 
                        filename, src, st.token.start, st.token);
            }
        }
        break;

        case shunting_token_type_binary_operator:
        {
            if(stack.size() < 2ul) {
                throw_parse_error(
                        "unsufficient operands for operator [" + lexer_token_name_and_value(st.token, src) + "]",
                        filename, src, st.token.start, st.token);
            }

            auto top = top_two_elements(); // will always .first <operand> .second
            switch(st.token.type) {
            case LexerToken_Syntax_Plus:
            case LexerToken_Syntax_Minus:
            case LexerToken_Syntax_Divide:
            case LexerToken_Syntax_Star:
            {
                const unsigned int signed_flag = (top.first.flags & analyze_flag_signed) | (top.second.flags & analyze_flag_signed);
                const bool left_int  = parse_analyze_is_integer(top.first.flags);
                const bool right_int = parse_analyze_is_integer(top.second.flags);
                const bool both_ints = left_int && right_int;

                if(!both_ints) {
                    // ERROR : both types need to be integer types
                    if(!left_int) {
                        throw_parse_error(
                                "subexpression to the left of [" + lexer_token_name_and_value(st.token, src) + "] is not an integer type",
                                filename, src, st.token.start, st.token);
                    } else {
                        throw_parse_error(
                                "subexpression to the right of [" + lexer_token_name_and_value(st.token, src) + "] is not an integer type",
                                filename, src, st.token.start, st.token);
                    }
                }

                analyze_info_t _ai;
                _ai.flags = (signed_flag ? analyze_flag_signed : analyze_flag_unsigned) | analyze_flag_temporary;
                stack.push(_ai);
            }
            break;

            case LexerToken_Syntax_Period:
            {
                const bool left_local_ref  = top.first.flags & analyze_flag_local;
                const bool right_field_ref = top.second.flags & analyze_flag_field_ref;

                if(!left_local_ref) {
                    // ERROR
                }

                if(!right_field_ref) {
                    // ERROR
                }

                std::string local_name = lexer_token_value(st.token, src);
                std::string field_name = lexer_token_value(st.token, src);
                parse_analyze_eval_local_field(rt, module_ptr, local_name, field_name, local_modules, stack);
            }
            break;

            }

        }

        }
    }
*/
}


static const bool parse_analyze_eval_local_field(
        HDL_Runtime_t* rt, 
        hdl_module_t* module_ptr, 
        const std::string& local_name, 
        const std::string& field_name,
        std::map<std::string, hdl_module_t*>& local_modules,
        std::stack<analyze_info_t>& stack) {

    return false;

    //

/*
    auto module_iter = local_modules.find(local_name);
    if(module_iter == local_modules.end()) {

    }

    auto field_iter = module_iter->second->module_io_port_interface.find(field_name);
    if(field_iter == module_iter->second->module_io_port_interface.end()) {
        // ERROR
    }

    module_port_t& module_port = field_iter->second;

    analyze_info_t _ai;
    if(module_port.type == module_port_type_bit) { _ai.flags = analyze_flag_module_bit;
    } else {                                       _ai.flags = analyze_flag_module_bitvec; }

    if(module_port.io_type == module_port_input) { _ai.flags |= analyze_flag_module_in;
    } else {                                       _ai.flags |= analyze_flag_module_out; }
*/

}

