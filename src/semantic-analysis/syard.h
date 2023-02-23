#pragma once

#include <src/lexer.h>
#include <src/error-util.h>
#include <src/runtime/module-desc.h>
#include <src/runtime/runtime-env.h>

#include <set>
#include <vector>
#include <utility>

enum eval_token_t {
    numeric_reference, // used for a variety of things, doesnt evaluate proper operations, only existence
    variable_reference,
    module_reference,
    left_paren,
    left_bracket,
    function_arg_sentinal,
    arr_access_sentinal,
};

struct shunting_stack_t {
    std::vector<token_t> op_stack;
    std::vector<eval_token_t> eval_stack;
};

enum shunt_behavior_t {
    shunt_behavior_before,
    shunt_behavior_after,
};

void process_shunting_yard(
        struct runtime_env_t* rtenv,
        struct module_desc_t* modptr,
        struct parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend,
        shunting_stack_t& shunt_stack,
        const std::set<token_type_t>& end_types,
        shunt_behavior_t shunt_behavior);

void shunting_yard_eval_operator(
        struct runtime_env_t* rtenv,
        struct module_desc_t* modptr,
        struct parse_info_t& p,
        token_iterator_t& titer,
        const token_iterator_t& tend,
        shunting_stack_t& shunt_stack,
        const token_t& t);

//void shunting_yard_print_eval_stack(shunting_stack_t& shunt_stack);
void shunting_yard_print_eval_stack(shunting_stack_t& shunt_stack, src_t& s);

const bool token_is_operator(token_type_t t);
