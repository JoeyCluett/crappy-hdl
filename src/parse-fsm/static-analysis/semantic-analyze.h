#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stack>

#include <src/error-util.h>
#include <src/hdl-module.h>
#include <src/hdl-runtime.h>
#include <src/hdl-variant.h>
#include <src/lexical-token.h>
#include <src/ir/opcodes.h>
#include <src/parse-fsm/parse-util.h>
#include <src/parse-fsm/module/expr/parse-shunt.h>

void semantic_analyze(
        HDL_Runtime_t* rt,
        hdl_module_t* module_ptr,
        std::vector<shunting_token_t>& output_queue, 
        shunting_token_t& stok, 
        const std::vector<char>& src,
        const std::string& filename);

