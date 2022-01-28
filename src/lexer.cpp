#include "lexer.h"
#include "lexical-token.h"
#include "error-util.h"

#include <string>
#include <iostream>

static bool is_word_char(const char c);
static bool is_number_char(const char c);

static void handle_word_token(LexerToken_t& token, std::vector<LexerToken_t>& tkns, const std::vector<char>& src);

std::vector<LexerToken_t> 
        lexical_analyze(const std::vector<char>& src, const std::string& filename) {

    std::vector<LexerToken_t> tkns;
    LexerToken_t token;

    const auto begin_iter = src.begin();
    auto src_iter = src.begin();
    auto end_iter = src.end();

    const int state_default         = 0;
    const int state_word            = 1;
    const int state_number_start_0  = 2;
    const int state_number_decimal  = 3;
    const int state_number_hex      = 4;
    const int state_number_bin      = 5;
    const int state_string          = 6;
    const int state_str_escape      = 7;

    int state_current = state_default;

    while(src_iter != end_iter) {
        const char c = *src_iter;

        switch(state_current) {
        case state_default:
            token.start = src_iter - begin_iter;

            // syntax: ;:=.[],*$()

            if(c == ';') { // semicolon
                token.type = LexerToken_Syntax_Semicolon;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == ':') { // colon
                token.type = LexerToken_Syntax_Colon;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == '=') { // assign
                token.type = LexerToken_Syntax_Assign;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == '.') { // period
                token.type = LexerToken_Syntax_Period;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == '[') { // lbracket
                token.type = LexerToken_Syntax_LBracket;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == ']') { // rbracket
                token.type = LexerToken_Syntax_RBracket;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == ',') { // comma
                token.type = LexerToken_Syntax_Comma;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == '*') {
                token.type = LexerToken_Syntax_Star;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == '$') {
                token.type = LexerToken_Syntax_Dollar;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == '(') {
                token.type = LexerToken_Syntax_LParen;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(c == ')') {
                token.type = LexerToken_Syntax_RParen;
                token.end  = token.start + 1;
                tkns.push_back(token);
                src_iter++;
            }
            else if(is_word_char(c)) {
                state_current = state_word;
                src_iter++;
            }
            else if(is_number_char(c)) {
                if(c == '0') {
                    state_current = state_number_start_0;
                    src_iter++;
                }
                else {
                    state_current = state_number_decimal;
                    src_iter++;
                }
            }
            else if(c == '"') {
                state_current = state_string;
                src_iter++;
            }
            else if(c == ' ' || c == '\n' || c == '\t') {
                src_iter++;
            }
            else {
                src_iter++;
            }

            break;
        
        case state_word:
            if(is_word_char(c) || is_number_char(c)) {
                src_iter++;
            }
            else {
                token.end = src_iter - begin_iter;
                handle_word_token(token, tkns, src);
                state_current = state_default;
            }
            break;

        case state_number_start_0:

            if(c == 'b' || c == 'B') {
                state_current = state_number_bin;
                src_iter++;
            }
            else if(c == 'x' || c == 'X') {
                state_current = state_number_hex;
                src_iter++;
            }
            else if(is_number_char(c)) {
                state_current = state_number_decimal;
                src_iter++;
            }
            else {
                // single digit '0'
                token.end = token.start + 1;
                token.type = LexerToken_NumberDec;
                tkns.push_back(token);
                state_current = state_default;
            }
            break;

        case state_number_decimal:
            if(is_number_char(c) || c == '_') {
                src_iter++;
            }
            else {
                token.end  = src_iter - begin_iter;
                token.type = LexerToken_NumberDec;
                tkns.push_back(token);
                state_current = state_default;
            }
            break;

        case state_number_hex:
            if(is_number_char(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == '_') {
                src_iter++;
            }
            else {
                token.end = src_iter - begin_iter;
                token.type = LexerToken_NumberHex;
                tkns.push_back(token);
                state_current = state_default;
            }
            break;

        case state_number_bin:
            if(c == '0' || c == '1' || c == '_') {
                src_iter++;
            }
            else {
                if(is_number_char(c) || is_word_char(c)) {
                    throw_lexer_error(
                            "Expecting chars '0', '1', or '_'. Received '" + std::string(1,c) + "'", filename, src, src_iter - begin_iter);
                }
                else {
                    token.end = src_iter - begin_iter;
                    token.type = LexerToken_NumberBin;
                    tkns.push_back(token);
                    state_current = state_default;
                }
            }
            break;

        case state_string:
            if(c == '"') {
                token.end = src_iter - begin_iter;
                token.start++;
                token.type = LexerToken_StringLiteral;
                tkns.push_back(token);
                state_current = state_default;
                src_iter++;
            }
            else if(c == '\\') {
                state_current = state_str_escape;
                src_iter++;
            }
            else {
                src_iter++;
            }
            break;

        case state_str_escape:
            state_current = state_string;
            src_iter++;
            break;

        }
    }

    return tkns;
}

static void handle_word_token(LexerToken_t& token, std::vector<LexerToken_t>& tkns, const std::vector<char>& src) {

    const auto begin_iter = src.begin();
    const std::string word(begin_iter + token.start, begin_iter + token.end);

    if(word == "requires") {
        token.type = LexerToken_KW_requires;
        tkns.push_back(token);
    }
    else if(word == "global") {
        token.type = LexerToken_KW_global;
        tkns.push_back(token);
    }
    else if(word == "integer") {
        token.type = LexerToken_KW_integer;
        tkns.push_back(token);
    }
    else if(word == "module") {
        token.type = LexerToken_KW_module;
        tkns.push_back(token);
    }
    else if(word == "out_ports") {
        token.type = LexerToken_KW_out_ports;
        tkns.push_back(token);
    }
    else if(word == "in_ports") {
        token.type = LexerToken_KW_in_ports;
        tkns.push_back(token);
    }
    else if(word == "start") {
        token.type = LexerToken_KW_start;
        tkns.push_back(token);
    }
    else if(word == "end") {
        token.type = LexerToken_KW_end;
        tkns.push_back(token);
    }
    else if(word == "string") {
        token.type = LexerToken_KW_string;
        tkns.push_back(token);
    }
    else if(word == "header") {
        token.type = LexerToken_KW_header;
        tkns.push_back(token);
    }
    else if(word == "empty") {
        token.type = LexerToken_KW_empty;
        tkns.push_back(token);
    }
    else {

        //
        // if its not a keyword, its a variable name
        //

        token.type = LexerToken_VarName;
        tkns.push_back(token);
    }
}

static bool is_word_char(const char c) {
    return (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_number_char(const char c) {
    return c >= '0' && c <= '9';
}
