#include <src/semantic-analysis/parser.h>
#include <src/semantic-analysis/syard.h>
#include <src/semantic-analysis/module/parse-module.h>
#include <src/error-util.h>

#include <iostream>
#include <string>

parse_info_t::parse_info_t(src_t& src, const std::string& filename, std::vector<token_t>& tkns)
        : src(src), filename(filename), tkns(tkns)
{
    ;
}

void parser_analyze(runtime_env_t* rtenv, src_t& src, const std::string& filename, std::vector<token_t>& tkns) {

    parse_info_t pinfo(src, filename, tkns);

    token_iterator_t tokeniter = tkns.begin();
    const token_iterator_t tokenend = tkns.end();

    while(tokeniter < tokenend) {
        const token_t& tok = *tokeniter++;
        const std::string str = lexer_token_value(tok, src);

        switch(tok.type) {
        case token_type_t::keyword_module:
            parse_module(rtenv, pinfo, tokeniter, tokenend);
            break;
        //case token_type_t::keyword_uses:
        default:
            throw_parse_error("expecting 'module', found '" + lexer_token_value(tok, src) + "' of type " + lexer_token_type(tok.type), filename, src, tok);
        }

        tokeniter++;
    }
}

