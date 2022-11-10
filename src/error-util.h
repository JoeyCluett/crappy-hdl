#pragma once

#include <src/lexer.h>

#include <vector>
#include <string>

#define STR(s) std::to_string(s)

void print_error_source(std::ostream& os, int src_idx, const std::vector<char>& src, const int error_len);

struct ParserError_t {

    ParserError_t(const std::vector<char>& src);

    const std::vector<char>& src_ref;
    std::string error_desc;
    std::string filename;
    int error_location;
    token_t token;
};

void throw_parse_error(
        const std::string& error_desc, 
        const std::string& filename, 
        const std::vector<char>& src, 
        int src_idx, 
        const token_t& token);

void throw_parse_error(
        const std::string& error_desc,
        const std::string& filename,
        const std::vector<char>& src,
        const token_t& token);

struct LexerError_t {

    LexerError_t(const std::vector<char>& src);

    const std::vector<char> src_ref;
    std::string error_desc;
    std::string filename;
    int error_location; 
};

void handle_lexer_error(std::ostream& os, LexerError_t& lexer_error);

void throw_lexer_error(
        const std::string& error_desc,
        const std::string& filename,
        const std::vector<char>& src,
        int src_idx);

void handle_parse_error(std::ostream& os, ParserError_t& parse_error);
