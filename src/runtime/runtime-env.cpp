#include "runtime-env.h"
#include <src/lexer.h>
#include <src/semantic-analysis/parser.h>
#include <src/error-util.h>

runtime_env_t::~runtime_env_t() {
    for(auto& p : this->modules) {
        delete p.second;
    }
}

module_desc_t* runtime_env_create_new_module(
        runtime_env_t* renv,
        const std::string& new_module_name,
        parse_info_t& p,
        token_t& tok) {

    auto iter = renv->modules.find(new_module_name);

    if(iter != renv->modules.end())
        throw_parse_error("module with name '" + new_module_name + "' already exists", p.filename, p.src, tok);

    module_desc_t* modptr = new module_desc_t;
    modptr->name = new_module_name;
    renv->modules.insert({ new_module_name, modptr }); // save pointer in runtime environment
    return modptr;
}
