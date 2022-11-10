#pragma once

#include <src/lexer.h>

#include <vector>
#include <string>

void lexer_consume_syntax(src_iter_t& iter, src_t& src, const std::string& filename, std::vector<token_t>& tkns);
