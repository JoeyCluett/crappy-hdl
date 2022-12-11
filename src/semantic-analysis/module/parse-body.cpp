#include <src/semantic-analysis/module/parse-body.h>
#include <src/semantic-analysis/module/expr/parse-local-decl.h>
#include <src/semantic-analysis/parser.h>
#include <src/semantic-analysis/syard.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>

void parse_body(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend) {

    token_t& first_token = *titer++;

    switch(first_token.type) {
    case token_type_t::keyword_local:
        parse_local_decl(rtenv, modptr, p, titer, tend);
    }
}
