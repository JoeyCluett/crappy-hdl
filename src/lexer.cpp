#include "lexer.h"
#include "lexical-token.h"
#include "error-util.h"

#include <map>
#include <string>
#include <iostream>

static bool is_word_char(const char c);
static bool is_number_char(const char c);

static void handle_word_token(LexerToken_t& token, std::vector<LexerToken_t>& tkns, const std::vector<char>& src);

std::vector<LexerToken_t> 
        lexical_analyze(const std::vector<char>& src, const std::string& filename) {

    std::vector<LexerToken_t> tkns;
    LexerToken_t token;

    const static std::map<const char, std::pair<const int, bool>> single_char_token = {
        { ';', { LexerToken_Syntax_Semicolon, false }},
        { ':', { LexerToken_Syntax_Colon    , false }},
        { '=', { LexerToken_Syntax_Assign   , true  }}, // =    special
        { '.', { LexerToken_Syntax_Period   , false }},
        { '[', { LexerToken_Syntax_LBracket , false }},
        { ']', { LexerToken_Syntax_RBracket , false }},
        { ',', { LexerToken_Syntax_Comma    , false }},
        { '*', { LexerToken_Syntax_Star     , false }},
        { '$', { LexerToken_Syntax_Dollar   , false }},
        { '(', { LexerToken_Syntax_LParen   , false }},
        { ')', { LexerToken_Syntax_RParen   , false }},
        { '~', { LexerToken_Syntax_Invert   , false }},
        { '!', { LexerToken_Syntax_Not      , true  }}, // !    special
        { '>', { LexerToken_Syntax_GrThan   , true  }}, // >    special
        { '<', { LexerToken_Syntax_LsThan   , true  }}, // <    special
        { '{', { LexerToken_Syntax_LBrace   , false }},
        { '}', { LexerToken_Syntax_RBrace   , false }},
        //{ '@', { LexerToken_Syntax_At       , false }}, // this is syntax but it plays a much larger role than other syntax
    };

    const static std::map<std::string, const int> multi_char_syntax = {
        { ">=", LexerToken_Syntax_GrEq     },
        { "<=", LexerToken_Syntax_LsEq     },
        { "==", LexerToken_Syntax_Equiv    },
        { "!=", LexerToken_Syntax_NotEquiv },
    };

    std::string special_syntax_str = "";

    const auto begin_iter = src.begin();
    auto src_iter = src.begin();
    auto end_iter = src.end();

    const int state_default           =  0;
    const int state_bitliteral_start  =  1;
    const int state_bitliteral_first  =  2; // either '0' or '1'
    const int state_bitliteral_second =  3; // depends on what first char was
    const int state_bitliteral_body   =  4; // series of ones, zeros, and underscores
    const int state_syntax_first_char =  5;
    const int state_word              =  6;
    const int state_number_start_0    =  7;
    const int state_number_decimal    =  8;
    const int state_number_hex        =  9;
    const int state_number_bin        = 10;
    const int state_string            = 11;
    const int state_str_escape        = 12;

    int state_current = state_default;

    while(src_iter != end_iter) {
        const char c = *src_iter;

        switch(state_current) {
        case state_default:
        {
            token.start = src_iter - begin_iter;

            // syntax: ;:=.[],*$()~><
            // special: ><=

            auto find_iter = single_char_token.find(c);
            if(find_iter != single_char_token.end()) {

                const bool special_syntax = find_iter->second.second;

                if(special_syntax) {
                    state_current = state_syntax_first_char;
                    special_syntax_str.push_back(c);
                    src_iter++;
                }
                else {
                    token.type = find_iter->second.first;
                    token.end = token.start + 1;
                    tkns.push_back(token);
                    src_iter++;
                }
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
            else if(c == '@') {
                state_current = state_bitliteral_start;
                src_iter++;
            }
            else if(c == '"') {
                state_current = state_string;
                src_iter++;
            }
            else if(c == ' ' || c == '\n' || c == '\t') {
                src_iter++;
            }
            else {
                //src_iter++;
                throw_lexer_error(
                    "unknown character found while processing input: " + std::string(1, c), 
                    filename, src, token.start);
            }
            break;
        }

        case state_bitliteral_start:
            if(c == '\'') {
                state_current = state_bitliteral_first;
                src_iter++;
            }
            else {
                throw_lexer_error(
                    "expecting '\''. received character: " + std::string(1, c),
                    filename, src, token.start);
            }
            break;

        case state_bitliteral_first: // either '0' or '1'
            if(c == '0' || c == '1') {
                state_current = state_bitliteral_second;
                src_iter++;
            }
            else {
                throw_lexer_error(
                    "expecting one of '0' or '1'. received character: " + std::string(1, c),
                    filename, src, token.start);
            }
            break;

        case state_bitliteral_second:
            if(c == '\'') {
                // single bit literal
                token.type = LexerToken_BitLiteralSingle;
                token.end = src_iter - begin_iter + 1;
                tkns.push_back(token);
                state_current = state_default;
                src_iter++;
            }
            else if(c == 'b') {
                state_current = state_bitliteral_body;
                src_iter++;
            }
            else {
                throw_lexer_error(
                    "expecting one of '\\\'' or 'b'. received character: " + std::string(1, c),
                    filename, src, src_iter - begin_iter);
            }
            break;

        case state_bitliteral_body:
            if(c == '0' || c == '_' || c == '1') {
                src_iter++;
            }
            else if(c == '\'') {
                token.type = LexerToken_BitLiteralMulti;
                token.end = src_iter - begin_iter + 1;
                tkns.push_back(token);
                state_current = state_default;
                src_iter++;
            }
            else {
                throw_lexer_error(
                    "expecting one of '0', '1', '_', or '\\\''. received character: " + std::string(1, c),
                    filename, src, src_iter - begin_iter);
            }
            break;

        case state_syntax_first_char:
            special_syntax_str.push_back(c);
            {
                auto multi_char_syntax_iter = multi_char_syntax.find(special_syntax_str);
                if(multi_char_syntax_iter != multi_char_syntax.end()) {
                    token.type = multi_char_syntax_iter->second;
                    token.end = token.start + 2;
                    tkns.push_back(token);
                    special_syntax_str.clear();
                    src_iter++;
                }
                else {
                    auto single_char_syntax_iter = single_char_token.find(special_syntax_str[0]);
                    token.type = single_char_syntax_iter->second.first;
                    token.end = token.start + 1;
                    tkns.push_back(token);
                    special_syntax_str.clear();
                }
            }
            state_current = state_default;
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

    const static std::map<std::string, const int> keyword_list = {
        { "requires",  LexerToken_KW_requires  },
        { "global",    LexerToken_KW_global    },
        { "integer",   LexerToken_KW_integer   },
        { "uinteger",  LexerToken_KW_uinteger  },
        { "module",    LexerToken_KW_module    },
        { "out_ports", LexerToken_KW_out_ports },
        { "out",       LexerToken_KW_out_ports },
        { "in_ports",  LexerToken_KW_in_ports  },
        { "in",        LexerToken_KW_in_ports  },
        { "start",     LexerToken_KW_start     },
        { "end",       LexerToken_KW_end       },
        { "string",    LexerToken_KW_string    },
        { "header",    LexerToken_KW_header    },
        { "empty",     LexerToken_KW_empty     },
        { "void",      LexerToken_KW_void      },
        { "local",     LexerToken_KW_local     },
    };

    auto iter = keyword_list.find(word);
    if(iter != keyword_list.end()) {
        token.type = iter->second;
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
