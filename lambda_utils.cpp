#include "lambda_utils.hpp"
#include "lambda_term.hpp"
#include<string>
#include<unordered_set>
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
            return Abstraction{
                .parameter = abs.parameter,
                .body = std::make_unique<LambdaTerm>(clone(*abs.body))
            };
        },
        [](const Application& app) -> LambdaTerm {
            return Application{
                .function = std::make_unique<LambdaTerm>(clone(*app.function)),
                .argument = std::make_unique<LambdaTerm>(clone(*app.argument))
            };
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
            if (free_vars(replacement).contains(abs.parameter)) {
                std::string old_parameter = abs.parameter;
                abs.parameter = fresh_var_name(abs.parameter);
                *abs.body = substitute(std::move(*abs.body), old_parameter, Variable{abs.parameter});
            }
            *abs.body = substitute(std::move(*abs.body), target_var, replacement);
            return abs;
        },
        [&](Application app) -> LambdaTerm {
            *app.function = substitute(std::move(*app.function), target_var, replacement);
            *app.argument = substitute(std::move(*app.argument), target_var, replacement);
            return app;
        }
    }, std::move(term));
}