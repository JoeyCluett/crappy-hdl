#include "parse-global.h"

static const int state_expect_varname   = 0;
static const int state_expect_colon     = 1;
static const int state_expect_datatype  = 2;
static const int state_expect_assign    = 3;
static const int state_expect_number    = 4;
static const int state_expect_semicolon = 5;

static int state_current = state_expect_varname;

static LexerToken_t varname;
static LexerToken_t data_type;
static LexerToken_t value;

bool parse_global(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename) {

    // global CpuWidth : integer = 32;

    switch(state_current) {
    case state_expect_varname:
        // global CpuWidth : integer = 32;
        //        ^^^^^^^^

        if(token.type == LexerToken_VarName) {
            state_current = state_expect_colon;
            varname = token;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting token of type [" + 
                    std::string(lexer_token_name(LexerToken_VarName)) + "]. Found token of type [" + 
                    lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;
    
    case state_expect_colon:
        // global CpuWidth : integer = 32;
        //                 ^

        if(token.type == LexerToken_Syntax_Colon) {
            state_current = state_expect_datatype;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting ':'. Found token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_expect_datatype:
        // global CpuWidth : integer = 32;
        //                   ^^^^^^^

        if(token.type == LexerToken_KW_integer || token.type == LexerToken_KW_string) {
            data_type = token;
            state_current = state_expect_assign;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error("Expecting one of 'integer' or 'string'. Found token of type [" + 
            lexer_token_name_and_value(token, src) + "]", 
            filename, src, token.start, token);
        }
        break;

    case state_expect_assign:
        // global CpuWidth : integer = 32;
        //                           ^

        if(token.type == LexerToken_Syntax_Assign) {
            state_current = state_expect_number;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error("Expecting '='. Found token of type [" + 
            lexer_token_name_and_value(token, src) + "]", 
            filename, src, token.start, token);
        }
        break;

    case state_expect_number:
        // global CpuWidth : integer = 32;
        //                             ^^

        if(
                token.type == LexerToken_NumberBin || 
                token.type == LexerToken_NumberDec || 
                token.type == LexerToken_NumberHex) {

            if(data_type.type != LexerToken_KW_integer) {
                throw_parse_error(
                        "Expecting constant of type string. Found token of type [" + lexer_token_name_and_value(token, src) + "]",
                        filename, src, token.start, token);
            }

            value = token;
            state_current = state_expect_semicolon;
            token_iter++;
            return false;
        }
        else if(token.type == LexerToken_StringLiteral) {

            if(data_type.type != LexerToken_KW_string) {
                throw_parse_error(
                        "Expecting constant of type integer. Found token of type [" + lexer_token_name_and_value(token, src) + "]",
                        filename, src, token.start, token);
            }

            value = token;
            state_current = state_expect_semicolon;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                    "Expecting constant. Found token of type [" +
                    lexer_token_name_and_value(token, src) + "]", 
                    filename, src, token.start, token);
        }
        break;

    case state_expect_semicolon:
        // global CpuWidth : integer = 32;
        //                               ^

        if(token.type == LexerToken_Syntax_Semicolon) {

            const int retval = hdl_runtime_add_global_value(rt, lexer_token_value(varname, src), value, src);

            if(retval == HDL_ADD_GLOBAL_SUCCESS) {
                token_iter++;
                state_current = state_expect_varname;
                return true;
            }
            else if(retval == HDL_ADD_GLOBAL_ALREADY_EXISTS) {

                const std::string namestr = lexer_token_value(varname, src);
                auto global_iter = rt->globals.find(namestr);

                std::string global_type;

                switch(global_iter->second.which()) {
                case hdl_variant_long  : global_type = lexer_token_name(LexerToken_KW_integer); break;
                case hdl_variant_ulong : global_type = lexer_token_name(LexerToken_KW_integer); break;
                case hdl_variant_string: global_type = lexer_token_name(LexerToken_KW_string);  break;
                }

                throw_parse_error(
                        "Global '" + lexer_token_value(varname, src) + "' of type [" + global_type + 
                        "] already exists", 
                        filename, src, varname.start, varname);
            }
            else if(retval == HDL_ADD_GLOBAL_INVALID_TYPE) {

                throw_parse_error(
                    "Global '" + lexer_token_value(varname, src) + "' is of invalid type", 
                    filename, src, varname.start, varname);

            }
        }
        else {
            throw_parse_error("Expecting ';'. Found token of type [" +
            lexer_token_name_and_value(token, src) + "]", filename, src, token.start, token);
        }
        break;
    }


}
