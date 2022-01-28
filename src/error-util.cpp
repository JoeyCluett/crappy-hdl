#include "error-util.h"

#include <iostream>
#include <vector>

#include <execinfo.h>

ParserError_t::ParserError_t(const std::vector<char>& src) : src_ref(src) {
}

void throw_parse_error(
        const std::string& error_desc, 
        const std::string& filename, 
        const std::vector<char>& src, 
        int src_idx, 
        const LexerToken_t& token) {

    ParserError_t parse_error(src);
    parse_error.error_desc     = error_desc;
    parse_error.filename       = filename;
    parse_error.error_location = src_idx;
    parse_error.token          = token;


    // when using Valgrind, TRACE_ON_EXIT is defined

#   ifdef TRACE_ON_EXIT
    handle_parse_error(std::cerr, parse_error);
    exit(EXIT_FAILURE);
#   else
    throw parse_error;
#   endif
}

LexerError_t::LexerError_t(const std::vector<char>& src) : src_ref(src) {
}

void throw_lexer_error(
        const std::string& error_desc,
        const std::string& filename,
        const std::vector<char>& src,
        int src_idx) {

    LexerError_t lexer_error(src);
    lexer_error.error_desc = error_desc;
    lexer_error.filename = filename;
    lexer_error.error_location = src_idx;

    throw lexer_error;
}

void handle_parse_error(std::ostream& os, ParserError_t& parse_error) {

    os << "\nParseError in file '" << parse_error.filename << "':\n";
    os << parse_error.error_desc << "\n\n";

    if(parse_error.token.type == LexerToken_StringLiteral) {
        print_error_source(os, parse_error.error_location-1, parse_error.src_ref);
    }
    else {
        print_error_source(os, parse_error.error_location, parse_error.src_ref);
    }
}

void handle_lexer_error(std::ostream& os, LexerError_t& lexer_error) {

    os << "\nLexerError in file '" << lexer_error.filename << "':\n";
    os << lexer_error.error_desc << "\n\n";

    print_error_source(os, lexer_error.error_location, lexer_error.src_ref);
}

void print_error_source(std::ostream& os, int src_idx, const std::vector<char>& src) {

    //os << std::string(src.begin(), src.end()) << std::endl;
    //os << "Error index: " << src_idx << std::endl;

    const auto src_iter = src.begin();
    auto error_iter = src_iter + src_idx;

    while(error_iter != src_iter && *error_iter != '\n') {
        error_iter--; // iterate backwards
    }

    if(*error_iter == '\n')
        error_iter++;

    auto tmp_iter = src_iter + src_idx;
    auto start_iter = error_iter;

    int lines = 1;
    for(auto iter = src.begin(); iter < src.end(); iter++) {
        if(*iter == '\n') {
            if(iter < tmp_iter) {
                lines++;
            }
        }
    }

    int column = tmp_iter - error_iter;

    os << "ln:" << lines << ",col:" << column+1 << "\n\n";

    // print the line with the error on it
    while(error_iter != src.end() && *error_iter != '\n') {
        os << *error_iter;
        error_iter++;
    }

    os << "\n";

    while(start_iter < error_iter) {
        if(start_iter == tmp_iter) {
            os << '^';
        }
        else {
            os << ' ';
        }

        start_iter++;
    }

    os << "\n\n";
}

/*
extern "C" void stack_trace_on_exit(void) {
    void* arr[256]; // up to 256 deep stack trace

    int size = backtrace(arr, 256);
    char** strs = backtrace_symbols(arr, size);

    if(strs != NULL) {
        for(int i = 0; i < size; i++) {
            std::cout << strs[i] << std::endl;
        }
    }
}
*/

