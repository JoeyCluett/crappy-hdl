#include <src/semantic-analysis/module/expr/parse-local-decl.h>
#include <src/semantic-analysis/parser.h>
#include <src/semantic-analysis/syard.h>
#include <src/bytecode-data/opcodes.h>
#include <src/lexer.h>
#include <src/error-util.h>
#include <src/error-util.h>
#include <src/runtime/runtime-env.h>
#include <src/runtime/module-desc.h>

#include <vector>
#include <string>

static inline void parse_local_module_instance(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend,
        shunting_stack_t& shunt_stack);

void parse_local_or_ref_decl(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend,
        token_type_t first_type) {

    token_t& first_token = *(titer - 1);

    //
    // ref refname = something...
    // local localname : type;
    // local localname : type = something...
    // local moduleinst = module.modulename(...);
    //

    token_t local_name = *titer++; // local {vecstuff} :  vector = vector( 0, @1001, module.full_adder() );

    if(local_name.type != token_type_t::variable_name)
        throw_parse_error(
                "Expecting variable name, found " + lexer_token_desc(local_name, p.src),
                p.filename, p.src, local_name);

    shunting_stack_t shunt_stack;

    if(first_type == token_type_t::keyword_ref) { // ref ref_name = ...
        token_t& expect_assign = *titer;
        if(expect_assign.type != token_type_t::assign)
            throw_parse_error(
                    "Expecting `=', found " + lexer_token_desc(expect_assign, p.src),
                    p.filename, p.src, expect_assign);

        size_t local_idx = module_desc_add_string_constant(modptr, lexer_token_value(local_name, p.src));
        opc::push_new_local_ref(modptr, local_idx);
        shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);

    } else if(first_type == token_type_t::keyword_local) { // local varname : typespec = ...

        token_t expect_colon = *titer++; // local vecstuff {:} vector = vector( 0, @1001, module.full_adder() );

        if(expect_colon.type == token_type_t::assign) { // local moduleinst = module.modulename(...);
            // this MUST be a module instantiation

            size_t local_idx = module_desc_add_string_constant(modptr, lexer_token_value(local_name, p.src));
            opc::push_new_local(modptr, token_type_t::keyword_module, local_idx);



            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
            shunt_stack.eval_stack.push_back(eval_token_t::module_reference);
            shunt_stack.op_stack.push_back(expect_colon); // assign operator

            parse_local_module_instance(rtenv, modptr, p, titer, tend, shunt_stack);
            return;
        } else if(expect_colon.type != token_type_t::colon) {
            throw_parse_error(
                    "Expecting `:' or `=', found " + lexer_token_desc(expect_colon, p.src),
                    p.filename, p.src, expect_colon);
        }

        token_t local_type   = *titer++; // local vecstuff  : {vector} = vector( 0, @1001, module.full_adder() );
        token_t& after_type  = *titer; // get token but dont advance iterator

        if(!lexer_token_is_typespec(local_type))
            throw_parse_error(
                    "Expecting type specifier, found " + lexer_token_desc(local_type, p.src),
                    p.filename, p.src, local_type);

        size_t local_idx = module_desc_add_string_constant(modptr, lexer_token_value(local_name, p.src));
        opc::push_new_local(modptr, local_type.type, local_idx);

        if(after_type.type == token_type_t::semicolon) {
            titer++; // now we can advance the iterator
            opc::clear_stack(modptr);
            return;
        } else if(after_type.type != token_type_t::assign) {
            throw_parse_error(
                    "Expecting `;' or `=', found " + lexer_token_desc(after_type, p.src),
                    p.filename, p.src, after_type);
        } else {
            // assignment
            shunt_stack.eval_stack.push_back(eval_token_t::variable_reference);
        }
    } else {
        throw_parse_error(
                    "Expecting `local' or `ref', found " + lexer_token_desc(first_token, p.src),
                    p.filename, p.src, first_token);
    }

    process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::semicolon }, shunt_behavior_after);
}

static inline void parse_local_module_instance(
        runtime_env_t* rtenv,
        module_desc_t* modptr,
        parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend,
        shunting_stack_t& shunt_stack) {

    // expecting:
    //
    // module.modulename(arglist);
    //

    token_t& expect_kw_module = *titer++;

    if(expect_kw_module.type != token_type_t::keyword_module)
        throw_parse_error("Expecting `module', found " + lexer_token_desc(expect_kw_module, p.src), p.filename, p.src, expect_kw_module);

    token_t& expect_period = *titer++;

    if(expect_period.type != token_type_t::period)
        throw_parse_error("Expecting `.', found " + lexer_token_desc(expect_period, p.src), p.filename, p.src, expect_period);

    token_t module_name = *titer++;

    if(module_name.type != token_type_t::variable_name)
        throw_parse_error("Expecting module name, found " + lexer_token_desc(module_name, p.src), p.filename, p.src, module_name);

    token_t& expect_lparen = *titer++;

    if(expect_lparen.type != token_type_t::lparen)
        throw_parse_error("Expecting left-parentheses, found " + lexer_token_desc(expect_lparen, p.src), p.filename, p.src, expect_lparen);

    // prepare shunt stack for processing arguments to 
    opc::push_module_args_sentinal(modptr);

    module_name.type = token_type_t::module_ref;
    shunt_stack.op_stack.push_back(module_name);

    process_shunting_yard(rtenv, modptr, p, titer, tend, shunt_stack, { token_type_t::semicolon }, shunt_behavior_after);

}
