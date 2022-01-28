#pragma once

#include "lexical-token.h"

#include "hdl-runtime.h"
#include "hdl-module.h"
#include "hdl-variant.h"

#include <vector>

HDL_Runtime_t* parse_analyze(
        const std::vector<LexerToken_t>& tokens, 
        const std::vector<char>& src,
        const std::string& filename);



