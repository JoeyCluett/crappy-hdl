#include "src/lexer.h"
#include "src/file-reader.h"
#include "src/error-util.h"
#include "src/semantic-analysis/parser.h"

#include <vector>
#include <iostream>

using namespace std;

static std::string left_pad(const std::string& input_str, int len);
static std::string right_pad(const std::string& input_str, int len);

int main(int argc, char* argv[]) {

    std::string filename = "hdl/util/comparator.chdl";
//    std::string filename = "hdl/riscv-inst-unmarshall.chdl";
//    std::string filename = "hdl/riscv-decoder.chdl";
//    std::string filename = "hdl/adders.chdl";

    runtime_env_t* renv = NULL;

    try {
        auto src    = read_hdl_file_contents(filename);
        std::vector<token_t> tkns;
        lexical_analyze(src, filename, tkns);
//        print_lexer_tokens(tkns);

        renv = new runtime_env_t;

        parser_analyze(renv, src, filename, tkns);

        delete renv;
    }
    catch(ParserError_t& parse_error) {
        handle_parse_error(std::cout, parse_error);
        delete renv;
        return 1;
    }
    catch(LexerError_t& lexer_error) {
        handle_lexer_error(std::cout, lexer_error);
        delete renv;
        return 1;
    }

#   ifndef TRACE_ON_EXIT
    std::cout << "processing of '" << filename << "' successful" << std::endl;
#   endif

    return 0;
}

static std::string left_pad(const std::string& input_str, int len) {

    const int needed_space = len - input_str.size();

    if(needed_space < 0)
        return input_str;

    std::string ret = " ";
    while(ret.size() < needed_space)
        ret.push_back(' ');

    return ret + input_str;
}

static std::string right_pad(const std::string& input_str, int len) {

    std::string ret = input_str;

    while(ret.size() < len)
        ret.push_back(' ');

    return ret;
}
