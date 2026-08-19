#pragma once

#include "lambda_term.hpp"
#include<string>
#include<unordered_set>
using namespace lambda;

// Define "overloaded"
template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template<typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::unordered_set<std::string> free_vars(const LambdaTerm& term);
std::string fresh_var_name(const std::string& var_name);
LambdaTerm clone(const LambdaTerm& term);
LambdaTerm substitute(LambdaTerm term, const std::string& target_var, const LambdaTerm& replacement);
