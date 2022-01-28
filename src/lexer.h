#pragma once

#include <vector>
#include "lexical-token.h"

std::vector<LexerToken_t> lexical_analyze(const std::vector<char>& src, const std::string& filename);




