#include <src/lexer.h>
#include <src/lexer-syntax.h>
#include <src/error-util.h>
#include <src/semantic-analysis/parser.h>

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <algorithm>

static std::string filename;
static const std::vector<char>* srcptr;
static src_iter_t srcbegin;
static src_iter_t srcend;

static const bool is_word_char(const char c);
static const bool is_number_char(const char c);
static const bool is_whitespace(const char c);
static const bool is_hex_digit(const char c);

static const bool lexer_seek(src_iter_t& iter);

static void lexer_consume_string(src_iter_t& iter, std::vector<token_t>& tkns);
static void lexer_consume_bitliteral(src_iter_t& iter, std::vector<token_t>& tkns);

static void lexer_consume_number(src_iter_t& iter, std::vector<token_t>& tkns);
static void lexer_consume_binary_number(src_iter_t& iter, std::vector<token_t>& tkns);
static void lexer_consume_hex_number(src_iter_t& iter, std::vector<token_t>& tkns);
static void lexer_consume_decimal_number(src_iter_t& iter, std::vector<token_t>& tkns);

static void lexer_consume_word(src_iter_t& iter, std::vector<token_t>& tkns);
static void lexer_word_eval(token_t& token, std::vector<token_t>& tkns);

static const bool lexer_is_var_char(const char c);

void lexical_analyze(
        src_t& src, 
        const std::string& filename, 
        std::vector<token_t>& tkns) {

    ::filename = filename;
    srcptr     = &src;
    srcbegin   = srcptr->begin();
    srcend     = srcptr->end();

    tkns.clear();
    token_t token;

    src_iter_t iter = srcbegin;

    lexer_seek(iter);

    const std::string syntax_chars = "><$^*&+-=/:;.,()[]{}|~!";

    while(iter < srcend) {
        const char c = *iter;

        if(is_word_char(c)) { // either keyword or variable name
            lexer_consume_word(iter, tkns);
        }
        else if(is_number_char(c)) { // some kind of number
            lexer_consume_number(iter, tkns);
        }
        else if(c == '"') { // string
            lexer_consume_string(iter, tkns);
        }
        else if(c == '@') { // bit literal
            lexer_consume_bitliteral(iter, tkns);
        }
        else if(is_whitespace(c)) {
            lexer_seek(iter);
        }
        else {
            auto syntax_begin = syntax_chars.begin();
            auto syntax_end   = syntax_chars.end();
            auto find_iter = std::find(syntax_begin, syntax_end, c);

            if(find_iter == syntax_end)
                throw_lexer_error("unknown character", filename, src, iter - srcbegin);

            lexer_consume_syntax(iter, src, filename, tkns);
        }
    }
}

static void lexer_consume_number(src_iter_t& iter, std::vector<token_t>& tkns) {

    auto second_char_iter = iter + 1;

    if(second_char_iter >= srcend)
        throw_lexer_error("malformed number", filename, *srcptr, iter - srcbegin);

    //
    // 0 - just zero
    // 0bnn - binary number
    // 0xnn - hex number
    // default - decimal number
    //

    const char c0 = *iter;
    const char c1 = *second_char_iter;

    if(c0 == '0' && !is_number_char(c1) && c1 != 'b' && c1 != 'x') { // just zero
        token_t tok;
        tok.type  = token_type_t::number_dec;
        tok.start = iter - srcptr->begin();
        tok.end   = second_char_iter - srcptr->begin();
        tkns.push_back(tok);
        iter++;
        return;
    }

    if(c0 == '0') {
        switch(c1) {
        case 'b': return lexer_consume_binary_number(iter, tkns);
        case 'x': return lexer_consume_hex_number(iter, tkns);
        default:
            throw_lexer_error("malformed '0'. note : multiple zeroes is invalid", filename, *srcptr, iter - srcbegin);
        }
    }

    lexer_consume_decimal_number(iter, tkns);
}

static void lexer_consume_binary_number(src_iter_t& iter, std::vector<token_t>& tkns) {
    // first two chars are "0b"

    token_t tok;
    tok.type = token_type_t::number_bin;
    tok.start = iter - srcbegin;

    iter += 2;

    while(iter < srcend) {
        const char c = *iter;

        if(c == '0' || c == '1' || c == '_') {
            iter++;
        } else {
            tok.end = iter - srcbegin;
            tkns.push_back(tok);
            return;
        }
    }

    throw_lexer_error("malformed binary number", filename, *srcptr, tok.start);
}

static void lexer_consume_hex_number(src_iter_t& iter, std::vector<token_t>& tkns) {
    // first two chars are "0x"

    token_t tok;
    tok.type = token_type_t::number_hex;
    tok.start = iter - srcbegin;
    iter += 2;

    while(iter < srcend) {
        const char c = *iter;

        if(is_hex_digit(c) || c == '_') {
            iter++;
        } else {
            tok.end = iter - srcbegin;
            tkns.push_back(tok);
            return;
        }
    }

    throw_lexer_error("malformed hexadecimal number", filename, *srcptr, tok.start);
}

static void lexer_consume_decimal_number(src_iter_t& iter, std::vector<token_t>& tkns) {
    token_t tok;
    tok.type = token_type_t::number_dec;
    tok.start = iter - srcbegin;

    while(iter < srcend) {
        const char c = *iter;

        if(is_number_char(c) || c == '_') {
            iter++;
        } else {
            tok.end = iter - srcbegin;
            tkns.push_back(tok);
            return;
        }
    }

    throw_lexer_error("malformed decimal number", filename, *srcptr, tok.start);
}

static void lexer_consume_word(src_iter_t& iter, std::vector<token_t>& tkns) {
    token_t token;
    token.start = iter - srcptr->begin();
    iter++;

    while(iter < srcend) {
        const char c = *iter;
        if(is_word_char(c) || is_number_char(c)) {
            iter++;
        } else {
            token.end = iter - srcptr->begin();
            return lexer_word_eval(token, tkns);
        }
    }
    
    throw_lexer_error("malformed source file", filename, *srcptr, 0);
}

static void lexer_word_eval(token_t& token, std::vector<token_t>& tkns) {
    auto srcbeg = srcptr->begin();
    const std::string word(srcbeg + token.start, srcbeg + token.end);

    auto kw_tup = lexer_token_is_keyword(word);
    if(std::get<0>(kw_tup)) {
        token.type = std::get<2>(kw_tup);
    } else {
        auto fn_tup = lexer_token_is_function(word);
        if(std::get<0>(fn_tup)) {
            token.type = token_type_t::function;
        } else {
            token.type = token_type_t::variable_name;
        }
    }

    tkns.push_back(token);
}

static void lexer_consume_bitliteral(src_iter_t& iter, std::vector<token_t>& tkns) {
    token_t token;
    src_iter_t bit_start = iter;
    iter++;

    while(iter < srcend) {
        const char c = *iter;
        if(c == '0' || c == '1' || c == '_') {
            iter++;
        } else if(lexer_is_var_char(c)) {
            throw_lexer_error("malformed bit-literal", filename, *srcptr, iter - srcbegin);
        } else {
            token.type  = token_type_t::bit_literal;
            token.start = bit_start - srcbegin;
            token.end   = iter - srcbegin;
            tkns.push_back(token);
            return;
        }
    }

    throw_lexer_error("malformed bit-literal at end of source", filename, *srcptr, bit_start - srcbegin - 1);
}

static void lexer_consume_string(src_iter_t& iter, std::vector<token_t>& tkns) {
    token_t token;

    src_iter_t string_start = iter + 1; // advance past opening "
    src_iter_t string_end;

    iter++;

    while(iter < srcend) {
        std::cout << "string char : " << *iter << std::endl;
        if(*iter == '\\') {
            iter += 2;
        } else if(*iter == '"') {
            string_end  = iter;
            token.type  = token_type_t::string_literal;
            token.start = string_start - srcbegin;
            token.end   = string_end - srcbegin;
            iter++; // advance past closing "

            tkns.push_back(token);
            return;
        } else {
            iter++;
        }
    }

    throw_lexer_error("malformed string", filename, *srcptr, string_start - srcbegin - 1);
}

static const bool lexer_seek(src_iter_t& iter) {
    while(iter < srcend) {
        if(is_whitespace(*iter)) {
            iter++;
        } else {
            return true;
        }
    }
    return false;
}

static const bool is_whitespace(const char c) {
    switch(c) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
        return true;
    default:
        return false;
    }
}

static const bool is_word_char(const char c) {
    return (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static const bool is_number_char(const char c) {
    return c >= '0' && c <= '9';
}

static const bool is_hex_digit(const char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

const string_t lexer_token_type(token_type_t token_type) {

    switch(token_type) {
    case token_type_t::keyword_integer:  return "typename:int";
    case token_type_t::keyword_uinteger: return "typename:uint";
    case token_type_t::keyword_string:   return "typename:string";
    case token_type_t::keyword_bit:      return "typename:bit";
    case token_type_t::keyword_module:   return "keyword:module";
    case token_type_t::keyword_out:      return "keyword:out";
    case token_type_t::keyword_in:       return "keyword:in";
    case token_type_t::keyword_start:    return "keyword:start";
    case token_type_t::keyword_end:      return "keyword:end";
    case token_type_t::keyword_void:     return "keyword:void";
    case token_type_t::keyword_local:    return "keyword:local";
    case token_type_t::keyword_ref:      return "keyword:ref";
    case token_type_t::keyword_builtin:  return "keyword:builtin";
    case token_type_t::keyword_true_:    return "keyword:true";
    case token_type_t::keyword_false_:   return "keyword:false";

    case token_type_t::variable_name:    return "variablename";
    case token_type_t::bit_literal:      return "bit-literal";
    case token_type_t::semicolon:        return "semicolon";     // ;
    case token_type_t::colon:            return "colon";         // :
    case token_type_t::assign:           return "assignment";    // =
    case token_type_t::period:           return "period";        // .
    case token_type_t::lbracket:         return "leftbracket";   // [
    case token_type_t::rbracket:         return "rightbracket";  // ]
    case token_type_t::comma:            return "comma";         // ,
    case token_type_t::star:             return "star";          // *
    case token_type_t::dollar:           return "dollar";        // $
    case token_type_t::lparen:           return "leftparen";     // (
    case token_type_t::rparen:           return "rightparen";    // )
    case token_type_t::not_:             return "not";           // !
    case token_type_t::lbrace:           return "leftbrace";     // {
    case token_type_t::rbrace:           return "rightbrace";    // }
    case token_type_t::divide:           return "divide";        // /
    case token_type_t::invert:           return "invert";        // ~
    case token_type_t::greater_than:     return "greaterthan";   // >
    case token_type_t::less_than:        return "lessthan";      // <
    case token_type_t::greater_eq:       return "greaterthaneq"; // >=
    case token_type_t::less_eq:          return "lessthaneq";    // <=
    case token_type_t::equiv:            return "equiv";         // ==
    case token_type_t::not_equiv:        return "notequiv";      // !=
    case token_type_t::pipe:             return "pipe";          // | OR
    case token_type_t::ampersand:        return "amp";           // & AND
    case token_type_t::caret:            return "caret";         // ^ XOR
    case token_type_t::plus:             return "plus";          // +
    case token_type_t::minus:            return "minus";         // - also a unary but the lexer doesnt know that
    case token_type_t::unary_negative:   return "negate";        // - (parser changes the type in-place)
    case token_type_t::number_dec:       return "decimal_number";
    case token_type_t::number_hex:       return "hex_number";
    case token_type_t::number_bin:       return "binary_number";
    case token_type_t::string_literal:   return "string";
    case token_type_t::bit_assign:       return "bit_assign";     // :=
    case token_type_t::function:         return "function";   // one of a number of native builtin functions
    default: return "UNKNOWN_TOKEN_TYPE";
    }
}

static const bool lexer_is_var_char(const char c) {
    return
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9');
}

const string_t lexer_token_value(const token_t& tok, src_t& src) {
    return string_t( src.begin() + tok.start, src.begin() + tok.end );
}

const string_t lexer_token_desc(const token_t& tok, src_t& src) {
    return "`" + lexer_token_value(tok, src) + "' [type=" + lexer_token_type(tok.type) + "]";
}

const bool lexer_token_is_typespec(const token_t& tok) {
    switch(tok.type) {
    case token_type_t::keyword_integer:
    case token_type_t::keyword_uinteger:
    case token_type_t::keyword_string:
    case token_type_t::keyword_vector:
        return true;
    default:
        return false;
    }
}

size_t lexer_token_to_uinteger(const token_t& tok, parse_info_t& p) {
    size_t v = 0ul;

    if(tok.type == token_type_t::number_bin) {
        std::string s = lexer_token_value(tok, p.src);
        for(auto iter = s.begin() + 2; iter < s.end(); iter++) {
            const char c = *iter;
            switch(c) {
            case '0':
            case '1':
                v <<= 1;
                v += (c - '0');
                break;
            case '_':
                break;
            default:
                throw_parse_error("Invalid character in binary constant " + lexer_token_desc(tok, p.src), p.filename, p.src, tok);
            }
        }

    } else if(tok.type == token_type_t::number_dec) {
        std::string s = lexer_token_value(tok, p.src);
        for(auto iter = s.begin(); iter < s.end(); iter++) {
            const char c = *iter;
            if(is_number_char(c)) {
                v *= 10ul;
                v += (c - '0');
            } else if(c != '_') {
                throw_parse_error("Invalid character in decimal constant " + lexer_token_desc(tok, p.src), p.filename, p.src, tok);
            }
        }
    } else if(tok.type == token_type_t::number_hex) {
        std::string s = lexer_token_value(tok, p.src);
        for(auto iter = s.begin() + 2; iter < s.end(); iter++) {
            const char c = *iter;
            if(is_hex_digit(c)) {
                v *= 16ul;
                if(is_number_char(c)) {           v += (c - '0');
                } else if(c >= 'a' && c <= 'f') { v += (10 + c - 'a');
                } else if(c >= 'A' && c <= 'F') { v += (10 + c - 'A');
                } else {                          INTERNAL_ERR();
                }
            } else if(c != '_') {
                throw_parse_error("Invalid character in hexadecimal constant " + lexer_token_desc(tok, p.src), p.filename, p.src, tok);
            }
        }
    } else {
        INTERNAL_ERR();
    }

    return v;
}

void print_lexer_tokens(std::vector<token_t>& tkns) {

    const int padding = 20;

    for(auto& t : tkns) {
        const string_t tok_type  = lexer_token_type(t.type);
        const string_t tok_value = lexer_token_value(t, *srcptr);

        std::cout << tok_type;
        for(int i = 0; i < (padding - tok_type.size()); i++)
            std::cout << ' ';
        std::cout << tok_value << std::endl;
    }

}

const bool operator==(const token_t& tok, token_type_t tt) {
    return tok.type == tt;
}

static const std::map<std::string, token_type_t> keywords = {
    { "integer",  token_type_t::keyword_integer  },
    { "uinteger", token_type_t::keyword_uinteger },
    { "string",   token_type_t::keyword_string   },
    { "bit",      token_type_t::keyword_bit      },
    { "module",   token_type_t::keyword_module   },
    { "out",      token_type_t::keyword_out      },
    { "in",       token_type_t::keyword_in       },
    { "start",    token_type_t::keyword_start    },
    { "end",      token_type_t::keyword_end      },
    { "void",     token_type_t::keyword_void     },
    { "local",    token_type_t::keyword_local    },
    { "ref",      token_type_t::keyword_ref      },
    { "builtin",  token_type_t::keyword_builtin  },
    { "true",     token_type_t::keyword_true_    },
    { "false",    token_type_t::keyword_false_   },
    { "vector",   token_type_t::keyword_vector   },
};

static const std::map<std::string, function_type_t> functions = {
    { "push",  function_type_t::push  },
    { "last",  function_type_t::last  },
    { "print", function_type_t::print },
    { "cast",  function_type_t::cast  },
};

std::tuple<bool, string_t, token_type_t> lexer_token_is_keyword(const std::string& s) {
    auto iter = keywords.find(s);
    if(iter == keywords.end()) {
        return { false, "", token_type_t::UNKNOWN };
    } else {
        return { true, iter->first, iter->second };
    }
}

std::tuple<bool, string_t, function_type_t> lexer_token_is_function(const std::string& s) {
    auto iter = functions.find(s);
    if(iter == functions.end()) {
        return { false, "", function_type_t::UNKNOWN };
    } else {
        return { true, iter->first, iter->second };
    }
}


