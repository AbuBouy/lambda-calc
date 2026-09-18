#pragma once

#include<memory>
#include<string>
#include<variant>

namespace lambda {

struct LambdaTerm;

struct Variable {
    std::string name;
};

struct Abstraction {
    std::string parameter;
    std::unique_ptr<LambdaTerm> body;
};

struct Application {
    std::unique_ptr<LambdaTerm> function;
    std::unique_ptr<LambdaTerm> argument;
};

struct LambdaTerm : std::variant<Variable, Abstraction, Application> {
    using variant::variant;
};


// Helper functions to enable clean construction of ASTs
inline LambdaTerm variable(std::string name) {
    return Variable{std::move(name)};
}

inline LambdaTerm abstraction(std::string parameter, LambdaTerm body) {
    return Abstraction{
        std::move(parameter),
        std::make_unique<LambdaTerm>(std::move(body))
    };
}

inline LambdaTerm application(LambdaTerm function, LambdaTerm argument) {
    return Application{
        std::make_unique<LambdaTerm>(std::move(function)),
        std::make_unique<LambdaTerm>(std::move(argument))
    };
}

}
