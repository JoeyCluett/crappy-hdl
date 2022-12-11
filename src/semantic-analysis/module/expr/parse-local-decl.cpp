#include <src/semantic-analysis/module/expr/parse-local-decl.h>
#include <src/semantic-analysis/parser.h>
#include <src/semantic-analysis/syard.h>
#include <src/bytecode-data/opcodes.h>
#include <src/lexer.h>
#include <src/error-util.h>
#include <src/error-util.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>

#include <vector>
#include <string>

void parse_local_decl(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend) {

    token_t& local_name   = *titer++;
    token_t& expect_colon = *titer++;
    token_t& local_type   = *titer++;

    if(local_name.type != token_type_t::variable_name)
        throw_parse_error("Expecting variable name, found " + lexer_token_desc(local_name, p.src), p.filename, p.src, local_name);

    if(expect_colon.type != token_type_t::colon)
        throw_parse_error("Expecting ':', found " + lexer_token_desc(expect_colon, p.src), p.filename, p.src, expect_colon);

    if(!lexer_token_is_typespec(local_type))
        throw_parse_error("Expecting type specifier, found " + lexer_token_desc(local_type, p.src), p.filename, p.src, local_type);    

    // add new variable into environment
    size_t local_idx = module_desc_add_string_constant(modptr, lexer_token_value(local_name, p.src));
    opc::push_new_local(modptr, local_type.type, local_idx);

    token_t& after_type = *titer++;
    if(after_type.type == token_type_t::semicolon) {
        opc::clear_stack(modptr);
    } else if(after_type.type == token_type_t::assign) {
        // for now, assume vector
        token_t& open_bk = *titer++;
        if(open_bk.type != token_type_t::lbracket)
            throw_parse_error("Expecting '{', found " + lexer_token_desc(open_bk, p.src), p.filename, p.src, open_bk);

        shunting_stack_t sstack;
        process_shunting_yard(rtenv, modptr, p, titer, tend, sstack, { token_type_t::semicolon }, shunt_behavior_after);
    } else {
        INTERNAL_ERR();
    }
}

