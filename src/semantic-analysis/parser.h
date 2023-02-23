#pragma once

#include <src/lexer.h>
#include <src/runtime/runtime-env.h>

#include <vector>
#include <string>
#include <map>

enum class parse_scope_type_t : int {
    for_loop,
    while_loop,
    if_statement,
};

struct parse_scope_info_t {
    parse_scope_type_t type;

    union {
        //
        // 3 parts of for loop expression:
        //  - initialization
        //  - condition
        //  - afterthought
        //
        // example :     for i = 0; i < width; i = i + 1 start
        //
        struct {
            size_t condition_tag;
            size_t afterthought_tag;
        } for_type;

        struct {

        } if_type;

        struct {

        } while_type;
    };
};

struct parse_info_t {

    parse_info_t(src_t& src, const std::string& filename, std::vector<token_t>& tkns);

    src_t& src;
    const std::string& filename;
    std::vector<token_t>& tkns;

    std::map<size_t, long int> branch_targets; // target .second is negative if it hasnt been evaluated yet
    std::vector<parse_scope_info_t> scope;
};

//
// perform semantic analysis
// also the code gen stage
//
void parser_analyze(struct runtime_env_t* rtenv, src_t& src, const std::string& filename, std::vector<token_t>& tkns);
