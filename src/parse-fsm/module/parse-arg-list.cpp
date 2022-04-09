#include "parse-arg-list.h"

static const int state_expect_lparen   = 0; // (
static const int state_varname_or_void = 1; // VarName or 'void'
static const int state_varname         = 2; // VarName
static const int state_colon           = 3; // :
static const int state_typename        = 4; // integer or uinteger or string
static const int state_after_typename  = 5; // , or )
static const int state_expect_rparen   = 6; // ) only used when arglist is void

static int state_current = state_expect_lparen;

size_t module_arg_index;

bool parse_arg_list(
        HDL_Runtime_t* rt,
        LexerToken_t token,
        std::vector<LexerToken_t>::const_iterator& token_iter,
        const std::vector<char>& src,
        const std::string& filename,
        hdl_module_t* module_ptr) {

    switch(state_current) {
    case state_expect_lparen:
        if(token.type == LexerToken_Syntax_LParen) {
            state_current = state_varname;
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting '('. Received token of type [" + lexer_token_name_and_value(token, src) + "].",
                filename, src, token.start, token);
        }
        break;

    case state_varname_or_void:
        if(token.type == LexerToken_VarName) {
            state_current = state_varname;
            // dont advance token iterator b/c state_varname expects this token to be a VarName
            return false;
        }
        else if(token.type == LexerToken_KW_void) {
            state_current = state_expect_rparen; // next token needs to be end of var list
            token_iter++;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting variable name or 'void'. Received token of type [" + lexer_token_name_and_value(token, src) + "].",
                filename, src, token.start, token);
        }
        break;

    case state_varname:
        if(token.type == LexerToken_VarName) {

            int error;
            const std::string arg_name = lexer_token_value(token, src);

            module_arg_index = module_arglist_add_argument_name(module_ptr, arg_name, &error);

            if(error == module_arg_error_argument_already_exists) {
                throw_parse_error(
                    "Argument '" + arg_name + "' already exists in module '" + module_ptr->name + "'.",
                    filename, src, token.start, token);
            }

            token_iter++;
            state_current = state_colon;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting token of type [" + lexer_token_name(LexerToken_VarName) + 
                "]. Received token of type [" + lexer_token_name_and_value(token, src) + "].",
                filename, src, token.start, token);
        }
        break;

    case state_colon:
        if(token.type == LexerToken_Syntax_Colon) {
            token_iter++;
            state_current = state_typename;
            return false;
        }
        else {
            throw_parse_error(
                "Expecting ':'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_typename:
        if(lexer_token_is_typename(token)) {

            switch(token.type) {
            case LexerToken_KW_integer:  module_arglist_set_argument_type(module_ptr, module_arg_index, hdl_variant_long);   break;
            case LexerToken_KW_uinteger: module_arglist_set_argument_type(module_ptr, module_arg_index, hdl_variant_ulong);  break;
            case LexerToken_KW_string:   module_arglist_set_argument_type(module_ptr, module_arg_index, hdl_variant_string); break;
            default:
                throw_parse_error(
                    "Unknown typename '" + lexer_token_name_and_value(token, src) + "'.",
                    filename, src, token.type, token);
                break;
            }

            state_current = state_after_typename;
            token_iter++;

            parse_arg_list_sanity_check(module_ptr);

            return false;
        }
        else {
            throw_parse_error(
                "Invalid typename [" + lexer_token_name_and_value(token, src) + 
                "] for argument '" + module_arglist_get_argument_name(module_ptr, module_arg_index) + 
                "' in module '" + module_ptr->name + "'.",
                filename, src, token.start, token);
        }
        break;

    case state_after_typename:
        if(token.type == LexerToken_Syntax_Comma) { // list contains another argument
            state_current = state_varname; // look for next argument name
            token_iter++;
            return false;
        }
        else if(token.type == LexerToken_Syntax_RParen) { // end of argument list
            state_current = state_expect_lparen; // reset state
            token_iter++;

            parse_arg_list_sanity_check(module_ptr);

            return true;
        }
        else {
            throw_parse_error(
                "Expecting one of ',' or ')'. Received token of type [" + lexer_token_name_and_value(token, src) + "]",
                filename, src, token.start, token);
        }
        break;

    case state_expect_rparen:
        if(token.type == LexerToken_Syntax_RParen) {
            state_current = state_expect_lparen; // reset state
            token_iter++;

            parse_arg_list_sanity_check(module_ptr);

            return true;
        }
        else {
            throw_parse_error(
                "Expecting ')'. Received token of type [" + lexer_token_name_and_value(token, src) + "].",
                filename, src, token.start, token);
        }
        break;
    }

    return false;
}

void parse_arg_list_sanity_check(hdl_module_t* module_ptr) {

    for(auto& p : module_ptr->argument_vector) {

        const std::string& arg_name = p.first;
        const int arg_type          = p.second;

        auto arg_map_iter = module_ptr->argument_map.find(arg_name);
        if(arg_map_iter == module_ptr->argument_map.end()) {
            // argument exists in arg vector but not arg map
            throw std::runtime_error(
                    "parse_arg_list_sanity_check : argument '" + 
                    arg_name + "' exists in argument vector but not argument map"
            );
        }

        if(arg_type != arg_map_iter->second.first) {
            throw std::runtime_error(
                    "parse_arg_list_sanity_check : type of argument '" + arg_name + 
                    "' in argument vector does not match corresponding entry in argument map"
            );
        }
    }

    for(auto& p : module_ptr->argument_map) {
        
        const std::string& arg_name = p.first;
        const int arg_type          = p.second.first;
        const size_t arg_idx        = p.second.second;

        if(arg_idx >= module_ptr->argument_vector.size()) {
            throw std::runtime_error(
                    "parse_arg_list_sanity_check : index of argument '" + 
                    arg_name + "' exceeds argument vector size"
            );
        }

        if(arg_name != module_ptr->argument_vector[arg_idx].first) {
            throw std::runtime_error(
                    "parse_arg_list_sanity_check : argument '" + arg_name + 
                    "' does not match entry [" + std::to_string(arg_idx) + 
                    "] in argument vector (found '" + module_ptr->argument_vector[arg_idx].first + "')"
            );
        }

        if(arg_type != module_ptr->argument_vector[arg_idx].second) {
            throw std::runtime_error(
                    "parse_arg_list_sanity_check : type of argument '" + arg_name + 
                    "' in argument map does not match corresponding entry in argument vector"
            );
        }
    }
}
