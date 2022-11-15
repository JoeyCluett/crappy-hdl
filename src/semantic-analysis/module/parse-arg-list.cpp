#include <src/semantic-analysis/module/parse-arg-list.h>
#include <src/semantic-analysis/module/parse-module.h>
#include <src/semantic-analysis/parser.h>
#include <src/lexer.h>
#include <src/error-util.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>

#include <iostream>

static void parse_expect_rparen(runtime_env_t* rtenv, parse_info_t& p, token_iterator_t& titer, const token_iterator_t& tend);

void parse_arg_list(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend) {

    token_t& tok = *titer;
    if(tok.type == token_type_t::keyword_void) {
        titer++;
        // titer now points to what should be closing paren
        return parse_expect_rparen(rtenv, p, titer, tend);
    }

    // arguments are always arranged as:
    // <argument_name> : <argument_type> (,)

    while(true) {

        token_t& arg_name = *titer++;
        token_t& colon    = *titer++;
        token_t& arg_type = *titer++;

        if(arg_name.type != token_type_t::variable_name)
            throw_parse_error("Expected variable name, found " + lexer_token_desc(arg_name, p.src), p.filename, p.src, arg_name);

        if(colon.type != token_type_t::colon)
            throw_parse_error("Expected colon ':', found " + lexer_token_desc(colon, p.src), p.filename, p.src, colon);

        if(!lexer_token_is_typespec(arg_type))
            throw_parse_error("Expected type specifier, found " + lexer_token_desc(arg_type, p.src), p.filename, p.src, arg_type);

        // valid argument (possibly)
        module_desc_add_argument_desc(modptr, p, arg_name, arg_type);

        token_t& after_arg = *titer++;
        if(after_arg.type == token_type_t::comma) {
            continue;
        } else if(after_arg.type == token_type_t::rparen) {
            return;
        } else {
            throw_parse_error("Expected ',' or ')', found " + lexer_token_desc(after_arg, p.src), p.filename, p.src, after_arg);
        }
    }
}

static void parse_expect_rparen(runtime_env_t* rtenv, parse_info_t& p, token_iterator_t& titer, const token_iterator_t& tend) {
    token_t& tok = *titer++;
    if(tok.type != token_type_t::rparen)
        throw_parse_error("Expected closing paren, found " + lexer_token_desc(tok, p.src), p.filename, p.src, tok);
}

