#include <iostream>

#include "src/file-reader.h"
#include "src/lexical-token.h"
#include "src/lexer.h"
#include "src/parser.h"
#include "src/error-util.h"
#include "src/hdl-runtime.h"

using namespace std;

static std::string left_pad(const std::string& input_str, int len);
static std::string right_pad(const std::string& input_str, int len);

int main(int argc, char* argv[]) {

    std::string filename = "hdl/toplevel.nhdl";

    try {
        auto src    = read_hdl_file_contents(filename);
        auto tokens = lexical_analyze(src, filename);
        auto* rt    = parse_analyze(tokens, src, "hdl/toplevel.nhdl");

        hdl_runtime_print(std::cout, rt);
    }
    catch(ParserError_t& parse_error) {
        handle_parse_error(std::cout, parse_error);
        return 1;
    }
    catch(LexerError_t& lexer_error) {
        handle_lexer_error(std::cout, lexer_error);
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
