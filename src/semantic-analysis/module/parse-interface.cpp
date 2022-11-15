#include <src/semantic-analysis/module/parse-interface.h>
#include <src/semantic-analysis/module/parse-module.h>
#include <src/semantic-analysis/parser.h>
#include <src/semantic-analysis/syard.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>
#include <src/lexer.h>
#include <src/error-util.h>



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

            token_t& colon = *titer++;
            
            if(inout.type != token_type_t::keyword_in && inout.type != token_type_t::keyword_out)
                throw_parse_error("Expected 'in' or 'out', found " + lexer_token_desc(inout, p.src), p.filename, p.src, inout);

            if(colon.type != token_type_t::colon)
                throw_parse_error("Expected ':', found " + lexer_token_desc(colon, p.src), p.filename, p.src, colon);

            inout_type = (inout.type == token_type_t::keyword_in) ?
                    module_desc_t::interface_type_t::in :
                    module_desc_t::interface_type_t::out;
            current_state      = iterate_elements;
            break;
        }
        case iterate_elements: {
            token_t& name       = *titer++;
            token_t& namefollow = *titer++;

            if(name.type != token_type_t::variable_name)
                throw_parse_error("Expected variable name, found " + lexer_token_desc(name, p.src), p.filename, p.src, name);

            switch(namefollow.type) {
            case token_type_t::comma: // interface element is single bit type
            case token_type_t::semicolon:
                module_desc_add_interface_element(modptr, lexer_token_value(name, p.src), inout_type, module_desc_t::interface_size_t::single);
                if(namefollow.type == token_type_t::semicolon) {
                    current_state = expect_in_out;
                }
                break;

            case token_type_t::lbracket: {
                // requires extra processing
                shunting_stack_t shunt_stack;
                process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::rbracket }, 0ul);

                if((titer - 1)->type != token_type_t::rbracket || shunt_stack.eval_stack.size() != 1ul)
                    throw_parse_error("Invalid size expression starting at " + lexer_token_desc(*((&namefollow) + 1), p.src), p.filename, p.src, *((&namefollow) + 1));


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

