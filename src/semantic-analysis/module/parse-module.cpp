#include <src/semantic-analysis/module/parse-module.h>
#include <src/semantic-analysis/parser.h>
#include <src/lexer.h>
#include <src/error-util.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>

void parse_module(
        runtime_env_t* rtenv,
        parse_info_t& p,
        token_iterator_t& titer,
        token_iterator_t tend) {

    token_t& modulename = *titer++;
    string_t modnamestr = lexer_token_value(modulename, p.src);

    if(modulename.type != token_type_t::variable_name) {
        throw_parse_error(
                "Expecting module name, found " + lexer_token_desc(modulename, p.src), p.filename, p.src, modulename);
    }

    module_desc_t* mod = runtime_env_create_new_module(rtenv, modnamestr, p, modulename);

    titer++;
    
}

