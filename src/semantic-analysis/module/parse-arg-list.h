#pragma once

#include <src/lexer.h>
#include <src/semantic-analysis/parser.h>
#include <src/error-util.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>

#include <vector>
#include <string>

//
// top-level function for parsing modules
// expects first iter to be the module name following 'module' keyword
//
void parse_arg_list(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend);

