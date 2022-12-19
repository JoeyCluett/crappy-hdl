#include <src/semantic-analysis/module/parse-module.h>
#include <src/semantic-analysis/module/parse-arg-list.h>
#include <src/semantic-analysis/module/parse-interface.h>
#include <src/semantic-analysis/module/parse-body.h>
#include <src/semantic-analysis/parser.h>
#include <src/lexer.h>
#include <src/error-util.h>
#include <src/bytecode-data/disassemble.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>

void parse_module(
        runtime_env_t* rtenv,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend) {

    token_t& modulename = *titer++;
    string_t modnamestr = lexer_token_value(modulename, p.src);

    if(modulename.type != token_type_t::variable_name) {
        throw_parse_error(
                "Expecting module name, found " + lexer_token_desc(modulename, p.src), p.filename, p.src, modulename);
    }

    module_desc_t* mod = runtime_env_create_new_module(rtenv, modnamestr, p, modulename);

    token_t& openparen = *titer++;
    if(openparen.type != token_type_t::lparen) {
        throw_parse_error(
            "Expecting open paren '(', found " + lexer_token_desc(openparen, p.src), p.filename, p.src, openparen);
    }

    parse_arg_list(rtenv, mod, p, titer, tend);
    parse_interface(rtenv, mod, p, titer, tend);
    parse_body(rtenv, mod, p, titer, tend);

    disassemble_bytecode(std::cout, mod);
}

