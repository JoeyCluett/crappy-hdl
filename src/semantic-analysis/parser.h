#pragma once

#include <src/lexer.h>
#include <src/runtime/runtime-env.h>

#include <vector>
#include <string>

struct runtime_env_t;

struct parse_info_t {

    parse_info_t(src_t& src, const std::string& filename, std::vector<token_t>& tkns);

    src_t& src;
    const std::string& filename;
    std::vector<token_t>& tkns;
};

//
// perform semantic analysis
// also the code gen stage
//
void parser_analyze(runtime_env_t* rtenv, src_t& src, const std::string& filename, std::vector<token_t>& tkns);



