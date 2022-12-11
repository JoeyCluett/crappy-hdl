#include <src/semantic-analysis/module/parse-interface.h>
#include <src/semantic-analysis/module/parse-module.h>
#include <src/semantic-analysis/parser.h>
#include <src/semantic-analysis/syard.h>
#include <src/bytecode-data/opcodes.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>
#include <src/lexer.h>
#include <src/error-util.h>

#include <iostream>

void parse_interface(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend) {

    enum {
        expect_in_out,
        iterate_elements,
    } current_state = expect_in_out;

    module_desc_t::interface_type_t inout_type;

    while(titer < tend) {
        switch(current_state) {
        case expect_in_out: {
            token_t& inout = *titer++;

            // TODO : inout could also be "start"

            if(inout.type == token_type_t::keyword_start) {
                std::cout << *modptr << std::flush;
                return;
            }

            token_t& colon = *titer++;
            
            if(inout.type != token_type_t::keyword_in && inout.type != token_type_t::keyword_out)
                throw_parse_error("Expected 'in' or 'out', found " + lexer_token_desc(inout, p.src), p.filename, p.src, inout);

            if(colon.type != token_type_t::colon)
                throw_parse_error("Expected ':', found " + lexer_token_desc(colon, p.src), p.filename, p.src, colon);

            inout_type = (inout.type == token_type_t::keyword_in) ?
                    module_desc_t::interface_type_t::in :
                    module_desc_t::interface_type_t::out;
            current_state = iterate_elements;
            break;
        }
        case iterate_elements: {
            token_t& name       = *titer++;
            token_t& namefollow = *titer++;

            if(name.type != token_type_t::variable_name)
                throw_parse_error("Expected variable name, found " + lexer_token_desc(name, p.src), p.filename, p.src, name);

            switch(namefollow.type) {
            case token_type_t::comma: // interface element is single bit type
            case token_type_t::semicolon: {
                auto tup = module_desc_add_interface_element(modptr, lexer_token_value(name, p.src), inout_type, module_desc_t::interface_size_t::single);
                if(std::get<0>(tup) == false)
                    throw_parse_error(std::get<2>(tup), p.filename, p.src, name);

                if(namefollow.type == token_type_t::semicolon)
                    current_state = expect_in_out;

                break;
            }

            case token_type_t::lbracket: {
                auto tup = module_desc_add_interface_element(modptr, lexer_token_value(name, p.src), inout_type, module_desc_t::interface_size_t::array);
                if(std::get<0>(tup) == false)
                    throw_parse_error(std::get<2>(tup), p.filename, p.src, name);

                inout_type == module_desc_t::interface_type_t::in ?
                        opc::push_in_ref(modptr, std::get<1>(tup)) :
                        opc::push_out_ref(modptr, std::get<1>(tup));

                // requires extra processing
                shunting_stack_t shunt_stack;
                process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::rbracket }, shunt_behavior_before);

                shunting_yard_print_eval_stack(shunt_stack);
                while(shunt_stack.eval_stack.size() > 1ul) {
                    token_t t = shunt_stack.op_stack.back();
                    if(!token_is_operator(t.type))
                        throw_parse_error("Expecting operator, found " + lexer_token_desc(t, p.src), p.filename, p.src, t);
                    
                    shunting_yard_eval_operator(rtenv, modptr, p, titer, tend, shunt_stack, t);
                    shunt_stack.op_stack.pop_back();
                }

                if((titer - 1)->type != token_type_t::rbracket || shunt_stack.eval_stack.size() != 1ul)
                    throw_parse_error("Invalid size expression starting at " + lexer_token_desc(*((&namefollow) + 1), p.src), p.filename, p.src, *((&namefollow) + 1));

                shunt_stack.eval_stack.clear();

                inout_type == module_desc_t::interface_type_t::in ?
                        opc::set_interface_input_size(modptr) :
                        opc::set_interface_output_size(modptr);
                opc::clear_stack(modptr);

                token_t& after_arr = *titer++;
                if(after_arr.type == token_type_t::comma) {
                    ; // nothing, let parsing continue as normal
                } else if(after_arr.type == token_type_t::semicolon) {
                    current_state = expect_in_out;
                } else {
                    throw_parse_error("Expecting ',' or ';', found " + lexer_token_desc(after_arr, p.src), p.filename, p.src, after_arr);
                }
                break;
            }
            default:
                throw_parse_error("Expected one of ',' or '[' or ';', found " + lexer_token_desc(namefollow, p.src), p.filename, p.src, namefollow);
            }

            break;
        }
        default:
            INTERNAL_ERR();
        }
    }
}

