#include <optional>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include<algorithm>

enum class TokenType {
    Lambda,
    Identifier,
    Dot,
    Left_Paren,
    Right_Paren,
    Invalid
};

struct Token {
    TokenType type;
    std::optional<std::string_view> name;
};


bool is_valid_identifier(std::string_view lexeme) {
    if (lexeme.empty()) return false;
    
    unsigned char first = static_cast<unsigned char>(lexeme[0]);
    if (!std::isalpha(first) && first != '_') return false;

    return std::ranges::all_of(lexeme, [](unsigned char c) {
        return std::isalnum(c) || c == '_';
    });
}

bool is_delimiter(char c) {
    return c == ' '  || c == '('  || c == ')' || 
           c == '.'  || c == '\n' || c == '\r'|| c == '\t';
}

std::vector<Token> tokenize(std::string_view source) {
    std::vector<Token> tokens;
    size_t cursor_end = 0;
    size_t cursor_start = 0;
    size_t length = source.size();

    while (cursor_end < length) {
        char current_char = source[cursor_end];

        if (is_delimiter(current_char)) {
            // Process any pending identifier/keyword before this delimiter
            if (cursor_end > cursor_start) {
                std::string_view lexeme = source.substr(cursor_start, cursor_end - cursor_start);
                if (lexeme == "lambda") {
                    tokens.push_back(Token{TokenType::Lambda, std::nullopt});
                } else if (is_valid_identifier(lexeme)) {
                    tokens.push_back(Token{TokenType::Identifier, lexeme});
                } else {
                    tokens.push_back(Token{TokenType::Invalid, lexeme});
                }
            }

            // Process the symbol itself
            if (current_char == '(') {
                tokens.push_back(Token{TokenType::Left_Paren, std::nullopt});
            } else if (current_char == ')') {
                tokens.push_back(Token{TokenType::Right_Paren, std::nullopt});
            } else if (current_char == '.') {
                tokens.push_back(Token{TokenType::Dot, std::nullopt});
            }

            cursor_start = cursor_end + 1;
        }

        cursor_end++;
    }

    // Handle trailing keyword/identifier
    if (cursor_start < length) {
        std::string_view lexeme = source.substr(cursor_start, length - cursor_start);
        if (lexeme == "lambda") {
            tokens.push_back(Token{TokenType::Lambda, std::nullopt});
        } else if (!lexeme.empty()) {
            if (is_valid_identifier(lexeme)) {
                tokens.push_back(Token{TokenType::Identifier, lexeme});
            } else {
                tokens.push_back(Token{TokenType::Invalid, lexeme});
            }
        }
    }
    
    return tokens;
}
