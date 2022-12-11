#pragma once

#include <src/semantic-analysis/parser.h>
#include <src/lexer.h>
#include <src/runtime/module-desc.h>

#include <map>
#include <string>
#include <utility>

struct runtime_env_t {

    std::map<std::string, struct module_desc_t*> modules;

    ~runtime_env_t();
};

//
// create new empty module in runtime
//
module_desc_t* runtime_env_create_new_module(
        runtime_env_t* renv,
        const std::string& new_module_name,
        struct parse_info_t& p,
        token_t& tok);

void runtime_env_print_module(std::ostream& os, runtime_env_t* rtenv, module_desc_t* modptr);
