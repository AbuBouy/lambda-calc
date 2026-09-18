#include "lambda_utils.hpp"
#include "lambda_term.hpp"
#include<string>
#include<unordered_set>
#include<utility>
using namespace lambda;


std::unordered_set<std::string> free_vars(const LambdaTerm& term) {
    return std::visit(overloaded{
        [](const Variable& var) {
            return std::unordered_set<std::string>{var.name};
        },
        [](const Abstraction& abs) {
            auto fv = free_vars(*abs.body);
            fv.erase(abs.parameter);
            return fv;
        },
        [](const Application& app) {
            auto function_fv = free_vars(*app.function);
            auto argument_fv = free_vars(*app.argument);
            function_fv.insert(argument_fv.begin(), argument_fv.end());
            return function_fv;
        }
    }, term);
}

std::string fresh_var_name(const std::string& var_name) {
    static int counter = 0;
    return var_name + std::to_string(counter++);
}

LambdaTerm clone(const LambdaTerm& term) {
    return std::visit(overloaded{
        [](const Variable& var) -> LambdaTerm {
            return var;
        },
        [](const Abstraction& abs) -> LambdaTerm {
            return abstraction(abs.parameter, clone(*abs.body));
        },
        [](const Application& app) -> LambdaTerm {
           return application(clone(*app.function), clone(*app.argument));
        }
    }, term);
}

LambdaTerm substitute(LambdaTerm term, const std::string& target_var, const LambdaTerm& replacement) {
    return std::visit(overloaded{
        [&](Variable var) -> LambdaTerm {
            if (var.name == target_var) {
                return clone(replacement);
            }   
            return var;
        },
        [&](Abstraction abs) -> LambdaTerm {
            if (abs.parameter == target_var) {
                return abs;
            }

            LambdaTerm body_term = std::move(*abs.body);
            std::string current_parameter = std::move(abs.parameter);
            
            if (free_vars(replacement).contains(current_parameter)) {
                std::string old_parameter = current_parameter;
                current_parameter = fresh_var_name(old_parameter);
                body_term = substitute(std::move(body_term), old_parameter, variable(current_parameter));
            }

            body_term = substitute(std::move(body_term), target_var, replacement);
            return abstraction(std::move(current_parameter), std::move(body_term));
        },
        [&](Application app) -> LambdaTerm {
            LambdaTerm new_function = substitute(std::move(*app.function), target_var, replacement);
            LambdaTerm new_argument = substitute(std::move(*app.argument), target_var, replacement);
            return application(std::move(new_function), std::move(new_argument));
        }
    }, std::move(term));
}

std::pair<LambdaTerm, bool> applicative_order_step(LambdaTerm term) {
    return std::visit(overloaded{
        [&](Variable var) -> std::pair<LambdaTerm, bool> {
            return {var, false};
        },
        [&](Abstraction abs) -> std::pair<LambdaTerm, bool> {
            auto [new_body, body_reduced] = applicative_order_step(std::move(*abs.body));
            return {abstraction(std::move(abs.parameter), std::move(new_body)), body_reduced};
        },
        [&](Application app) -> std::pair<LambdaTerm, bool> {
            auto [new_function, function_reduced] = applicative_order_step(std::move(*app.function));
            if (function_reduced) {
                return {application(std::move(new_function), std::move(*app.argument)), true};
            }
            auto [new_argument, argument_reduced] = applicative_order_step(std::move(*app.argument));
            if (argument_reduced) {
                return {application(std::move(new_function), std::move(new_argument)), true};
            }

            if (auto abs_ptr = std::get_if<Abstraction>(&new_function)) {
                LambdaTerm new_term = substitute(std::move(*abs_ptr->body), abs_ptr->parameter, new_argument);
                return {std::move(new_term), true};
            }
            
            return {application(std::move(new_function), std::move(new_argument)), false};
        },
    }, std::move(term));
}