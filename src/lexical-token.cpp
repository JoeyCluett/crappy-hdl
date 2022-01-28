#include "lexical-token.h"

#include <string>
#include <vector>

const std::string lexer_token_name(int token_type) {

    switch(token_type) {
    case LexerToken_KW_requires:  return "Keyword:requires";
    case LexerToken_KW_global:    return "Keyword:global";
    case LexerToken_KW_integer:   return "Keyword:integer";
    case LexerToken_KW_module:    return "Keyword:module";
    case LexerToken_KW_out_ports: return "Keyword:out_ports";
    case LexerToken_KW_in_ports:  return "Keyword:in_ports";
    case LexerToken_KW_start:     return "Keyword:start";
    case LexerToken_KW_end:       return "Keyword:end";
    case LexerToken_KW_string:    return "Keyword:string";
    case LexerToken_KW_header:    return "Keyword:header";
    case LexerToken_KW_empty:     return "Keyword:empty";

    case LexerToken_VarName: return "VariableName";

    case LexerToken_Syntax_Semicolon: return "Syntax:semicolon";
    case LexerToken_Syntax_Colon:     return "Syntax:colon";
    case LexerToken_Syntax_Assign:    return "Syntax:assignment";
    case LexerToken_Syntax_Period:    return "Syntax:period";
    case LexerToken_Syntax_LBracket:  return "Syntax:leftbracket";
    case LexerToken_Syntax_RBracket:  return "Syntax:rightbracket";
    case LexerToken_Syntax_Comma:     return "Syntax:comma";
    case LexerToken_Syntax_Star:      return "Syntax:star";
    case LexerToken_Syntax_Dollar:    return "Syntax:dollar";
    case LexerToken_Syntax_LParen:    return "Syntax:leftparen";
    case LexerToken_Syntax_RParen:    return "Syntax:rightparen";

    case LexerToken_NumberDec: return "Number:decimal";
    case LexerToken_NumberHex: return "Number:hexadecimal";
    case LexerToken_NumberBin: return "Number:binary";

    case LexerToken_StringLiteral: return "StringLiteral";

    default: return "TokenTypeUnknown";
    }
}

const bool lexer_token_is_number_type(const LexerToken_t& token) {
    return 
        token.type == LexerToken_NumberDec ||
        token.type == LexerToken_NumberHex ||
        token.type == LexerToken_NumberBin;
}

const std::string lexer_token_string_eval(LexerToken_t token, const std::vector<char>& src) {

    const std::string str(src.begin() + token.start, src.begin() + token.end);
    std::string eval;

    const int state_default = 0;
    const int state_escape  = 1;
    int state_current = state_default;

    for(char c : str) {
        switch(state_current) {
        case state_default:
            if(c == '\\') {
                state_current = state_escape;
            }
            else {
                eval.push_back(c);
            }
            break;
        case state_escape:
            if(c == 'n') {
                eval.push_back('\n');
            }
            else if(c == 't') {
                eval.push_back('\t');
            }
            else if(c == '\\') {
                eval.push_back('\\');
            }
            else if(c == 's') {
                eval.push_back(' ');
            }
            else if(c == '"') {
                eval.push_back('"');
            }
            else {
                throw 1;
            }
            state_current = state_default;
            break;
        }
    }

    return eval;
}

const std::string lexer_token_value(const LexerToken_t& token, const std::vector<char>& src) {
    const auto src_begin = src.begin();
    return std::string{ src_begin + token.start, src_begin + token.end };
}

const std::string lexer_token_name_and_value(const LexerToken_t& token, const std::vector<char>& src) {

    if(
            token.type == LexerToken_NumberBin || 
            token.type == LexerToken_NumberDec || 
            token.type == LexerToken_NumberHex) {
        
        return lexer_token_name(token.type) + "='" + lexer_token_value(token, src) + "',value=" + 
        std::to_string(lexer_token_number_to_ulong(token, src));
    }
    else if(token.type == LexerToken_StringLiteral) {
        return lexer_token_name(token.type) + "='" + lexer_token_value(token, src) + "',value='" + 
        lexer_token_string_eval(token, src) + "'";
    }
    else {
        return lexer_token_name(token.type) + "='" + lexer_token_value(token, src) + "'";
    }
}

size_t lexer_token_number_to_ulong(LexerToken_t token, const std::vector<char>& src) {
    const auto src_beg = src.begin();
    std::string string_val(src_beg + token.start, src_beg + token.end);

    size_t sz = 0ul;

    if(token.type == LexerToken_NumberBin) {
        string_val = string_val.substr(2);

        for(char c : string_val) {
            if(c == '1') {
                sz = sz << 1;
                sz |= 0x01;
            }
            else if(c == '0') {
                sz = sz << 1;
            }
            else {
                // underscore. do nothing
            }
        }
    }
    else if(token.type == LexerToken_NumberDec) {
        const std::string digit_lut = "0123456789";

        for(char c : string_val) {
            if(c >= '0' && c <= '9') {
                sz *= 10;
                int d = digit_lut.find_first_of(c);
                sz += d;
            }
        }
    }
    else if(token.type == LexerToken_NumberHex) {
        string_val = string_val.substr(2);
        for(char c : string_val) {

            if(c == '_') {
                continue;
            }

            sz = sz << 4;

            switch(c) {
            case '0': sz += 0; break;
            case '1': sz += 1; break;
            case '2': sz += 2; break;
            case '3': sz += 3; break;
            case '4': sz += 4; break;
            case '5': sz += 5; break;
            case '6': sz += 6; break;
            case '7': sz += 7; break;
            case '8': sz += 8; break;
            case '9': sz += 9; break;
            case 'a': case 'A': sz += 0xA; break;
            case 'b': case 'B': sz += 0xB; break;
            case 'c': case 'C': sz += 0xC; break;
            case 'd': case 'D': sz += 0xD; break;
            case 'e': case 'E': sz += 0xE; break;
            case 'f': case 'F': sz += 0xF; break;
            }
        }
    }
    else {
        // TODO
    }

    return sz;
}

long lexer_token_number_to_long(LexerToken_t token, const std::vector<char>& src) {

    union {
        size_t sz;
        long lg;
    };

    sz = lexer_token_number_to_ulong(token, src);
    return lg;
}
