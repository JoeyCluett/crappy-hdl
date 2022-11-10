#pragma once

#include <src/semantic-analysis/parser.h>
#include <src/lexer.h>
#include <src/runtime/module-desc.h>

#include <map>
#include <string>

struct module_desc_t;
struct parse_info_t;

struct runtime_env_t {

    std::map<std::string, module_desc_t*> modules;

    ~runtime_env_t();

};

//
// create new empty module in runtime
//
module_desc_t* runtime_env_create_new_module(
        runtime_env_t* renv,
        const std::string& new_module_name,
        parse_info_t& p,
        token_t& tok);
