#include <src/semantic-analysis/module/parse-body.h>
#include <src/semantic-analysis/parser.h>
#include <src/semantic-analysis/syard.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>
#include <src/bytecode-data/opcodes.h>

void parse_body(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend) {

    while(modptr->scope_levels > 0 && titer < tend) {

        token_t& first_token = *titer++;
        switch(first_token.type) {
        case token_type_t::keyword_local:
        case token_type_t::keyword_ref:
        case token_type_t::function:
        case token_type_t::variable_name:
        case token_type_t::keyword_in:
        case token_type_t::keyword_out:
        {
            titer--;
            shunting_stack_t shunt_stack;
            process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::semicolon }, shunt_behavior_after);            
            break;
        }

        case token_type_t::keyword_start:
            modptr->scope_levels++;
            break;

        case token_type_t::keyword_end:
            if(p.scope.size() > 0ul) {
                auto& scope_info = p.scope.back();

                if(scope_info.type == parse_scope_type_t::for_loop) {
                    opc::jump_exe(modptr, scope_info.for_type.afterthought_tag);
                    module_desc_define_jump_label(modptr, scope_info.for_type.end_scope_tag, modptr->bytecode.size());

                    modptr->scope_levels--;
                    p.scope.pop_back();
                    opc::pop_scope(modptr);
                } else if(scope_info.type == parse_scope_type_t::if_statement) {
                    INTERNAL_ERR();
                } else {
                    INTERNAL_ERR();
                }

            } else {
                modptr->scope_levels--;
                opc::pop_scope(modptr);
            }
            break;

        case token_type_t::keyword_for: {
            parse_scope_info_t pinfo;
            pinfo.type = parse_scope_type_t::for_loop;
            pinfo.for_type.condition_tag    = module_desc_alloc_jump_label(modptr);
            pinfo.for_type.afterthought_tag = module_desc_alloc_jump_label(modptr);
            pinfo.for_type.end_scope_tag    = module_desc_alloc_jump_label(modptr);
            pinfo.for_type.body_tag         = module_desc_alloc_jump_label(modptr);
            p.scope.push_back(pinfo);

            { // initialization
                shunting_stack_t shunt_stack;
                process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::semicolon }, shunt_behavior_after);
            }

            module_desc_define_jump_label(modptr, pinfo.for_type.condition_tag, modptr->bytecode.size());

            { // condition
                shunting_stack_t shunt_stack;
                process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::semicolon }, shunt_behavior_before);
                shunting_yard_eval_semicolon(rtenv, modptr, p, titer, tend, shunt_stack);

                opc::jump_on_true(modptr, pinfo.for_type.body_tag);
                opc::jump_on_false(modptr, pinfo.for_type.end_scope_tag);

                opc::clear_stack(modptr);
            }

            module_desc_define_jump_label(modptr, pinfo.for_type.afterthought_tag, modptr->bytecode.size());

            { // afterthought
                shunting_stack_t shunt_stack;
                process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::keyword_start }, shunt_behavior_before);
                shunting_yard_eval_semicolon(rtenv, modptr, p, titer, tend, shunt_stack);
                opc::clear_stack(modptr);
                opc::jump_exe(modptr, pinfo.for_type.condition_tag);
                titer--;
            }

            module_desc_define_jump_label(modptr, pinfo.for_type.body_tag, modptr->bytecode.size());

            break;
        }
        default: {
            throw_parse_error("Statement cannot start with " + lexer_token_desc(first_token, p.src), p.filename, p.src, first_token);
        }
        }
    }

    if(modptr->scope_levels > 0l) {
        throw_parse_error("Missing closing `end'", p.filename, p.src, *(tend-1));
    }
 
    opc::return_(modptr);
}
