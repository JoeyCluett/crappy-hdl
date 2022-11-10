#include <src/error-util.h>

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
        const token_t& token) {

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

void throw_parse_error(
        const std::string& error_desc,
        const std::string& filename,
        const std::vector<char>& src,
        const token_t& token) {
    throw_parse_error(error_desc, filename, src, token.start, token);
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

    token_t t = parse_error.token;
    const int t_len = t.end - t.start;

    if(parse_error.token.type == token_type_t::string_literal) {
        print_error_source(os, parse_error.error_location-1, parse_error.src_ref, t_len + 2);
    }
    else {
        print_error_source(os, parse_error.error_location, parse_error.src_ref, t_len);
    }
}

void handle_lexer_error(std::ostream& os, LexerError_t& lexer_error) {

    os << "\nLexerError in file '" << lexer_error.filename << "':\n";
    os << lexer_error.error_desc << "\n\n";

    print_error_source(os, lexer_error.error_location, lexer_error.src_ref, 1);
}

void print_error_source(std::ostream& os, int src_idx, const std::vector<char>& src, const int error_len) {

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

            for(int i = 0; i < (error_len - 1); i++)
                os << '~';
        }
        else {
            os << ' ';
        }

        start_iter++;
    }


    os << "\n\n";
}


