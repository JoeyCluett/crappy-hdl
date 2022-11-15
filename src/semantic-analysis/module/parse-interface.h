#pragma once

#include <src/lexer.h>
#include <src/semantic-analysis/parser.h>
#include <src/error-util.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>

#include <vector>
#include <string>

//
// top-level function for parsing interface (in/out) section following argument list
//
void parse_interface(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend);

