#include <src/semantic-analysis/module/parse-body.h>
#include <src/semantic-analysis/module/expr/parse-local-decl.h>
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
            parse_local_or_ref_decl(rtenv, modptr, p, titer, tend, token_type_t::keyword_local);
            break;

        case token_type_t::keyword_ref:
            parse_local_or_ref_decl(rtenv, modptr, p, titer, tend, token_type_t::keyword_ref);
            break;

        case token_type_t::keyword_start:
            modptr->scope_levels++;
            break;

        case token_type_t::keyword_end:
            modptr->scope_levels--;
            opc::pop_scope(modptr);
            break;

        case token_type_t::keyword_for: {
            parse_scope_info_t pinfo;
            pinfo.type = parse_scope_type_t::for_loop;

            shunting_stack_t shunt_stack;
            process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::semicolon }, shunt_behavior_after);

            p.scope.push_back(pinfo);
            break;
        }

        }
    }

    opc::return_(modptr);
}
