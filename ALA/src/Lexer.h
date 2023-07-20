#ifndef ALVM_ALA_LEXER_H
#define ALVM_ALA_LEXER_H

#include <vector>
#include <string>
#include <ALVM.h>

#define GET_TOKEN_STR(token) Token::TokenStr[(std::size_t)token]

namespace rlang::rmc {
    enum class TokenType
    {
        Whitespace,
        Comment,
        Identifier,
        Number,
        Register,
        Instruction,
        BitSizeIndicator,
        Operator,
        StringLiteral
    };

    struct Token
    {
        TokenType type = TokenType::Whitespace;
        std::string text;
        std::int32_t data = 0;
        std::size_t line = 0;
        std::size_t cur = 0;
    
    public:
        inline static const std::vector<std::string> TokenStr =
        {
            "Whitespace",
            "Comment",
            "Identifier",
            "Number",
            "Register",
            "Instruction",
            "BitSizeIndicator",
            "Operator",
            "StringLiteral"
        };

    public:
        void Dump() const;
    };
    using TokenList = std::vector<Token>;

    class Lexer
    {
    public:
        static TokenList Start(const std::string& src);

    private:
        static void EndToken(Token& t, TokenList& tokens);
    };
}

#endif // ALVM_ALA_LEXER_H
