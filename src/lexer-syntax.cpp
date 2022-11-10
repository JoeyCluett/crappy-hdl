#include <src/lexer-syntax.h>
#include <src/lexer.h>
#include <src/error-util.h>

#include <vector>
#include <string>

static const bool lexer_syntax_multichar(const char c) {
    return c == ':' || c == '>' || c == '<' || c == '!' || c == '=';
}

void lexer_consume_syntax(src_iter_t& iter, src_t& src, const std::string& filename, std::vector<token_t>& tkns) {

    auto src_end = src.end();
    const char c0 = *iter;
    const char c1 = *(iter + 1);

    token_t tok;
    tok.start = iter - src.begin();
    tok.end = tok.start + 1;

    switch(c0) {
    // always one character
    case ';': tok.type = token_type_t::semicolon; break;
    case '.': tok.type = token_type_t::period; break;
    case ',': tok.type = token_type_t::comma; break;
    case '|': tok.type = token_type_t::pipe; break;
    case '&': tok.type = token_type_t::ampersand; break;
    case '^': tok.type = token_type_t::caret; break;
    case '+': tok.type = token_type_t::plus; break;
    case '/': tok.type = token_type_t::divide; break;
    case '*': tok.type = token_type_t::star; break;
    case '~': tok.type = token_type_t::invert; break;
    case '[': tok.type = token_type_t::lbracket; break;
    case ']': tok.type = token_type_t::rbracket; break;
    case '(': tok.type = token_type_t::lparen; break;
    case ')': tok.type = token_type_t::rparen; break;
    case '{': tok.type = token_type_t::lbrace; break;
    case '}': tok.type = token_type_t::rbrace; break;
    case '$': tok.type = token_type_t::dollar; break;
    case '-': tok.type = token_type_t::minus; break;

    // can be two chars
    case ':': tok.type = token_type_t::colon; break;
    case '>': tok.type = token_type_t::greater_than; break;
    case '<': tok.type = token_type_t::less_than; break;
    case '!': tok.type = token_type_t::not_; break;
    case '=': tok.type = token_type_t::assign; break;

    // unknown
    default:
        throw_lexer_error("unrecognized syntax/operator", filename, src, iter - src.begin());
    }

    if(!lexer_syntax_multichar(c0) || c1 != '=') {
        tkns.push_back(tok);
        iter++;
        return;
    }

    switch(c0) {
    case ':': tok.type = token_type_t::bit_assign; break;
    case '>': tok.type = token_type_t::greater_eq; break;
    case '<': tok.type = token_type_t::less_eq; break;
    case '!': tok.type = token_type_t::not_equiv; break;
    case '=': tok.type = token_type_t::equiv; break;
    default:
        throw_lexer_error("unrecognized multi-char operator", filename, src, iter - src.begin());
    }

    tok.end++;
    tkns.push_back(tok);
    iter += 2;
    return;
}

