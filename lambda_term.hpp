#pragma once

#include<memory>
#include<string>
#include<variant>

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
