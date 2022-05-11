#pragma once

#include <vector>
#include <string>
#include <map>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>

bool parse_builtin_ref(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr,
        std::stack<shunting_token_t>& work_stack,
        std::vector<shunting_token_t>& output_queue);
