#ifndef ALVM_ALASM_LEXER_H
#define ALVM_ALASM_LEXER_H

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
        Immediate,
        Register,
        Instruction,
        BitSizeIndicator,
        Operator,
        StringLiteral,
        Displacement
    };

    struct Token
    {
        TokenType type = TokenType::Whitespace;
        std::string text;
        std::uint64_t data = 0;
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
            "StringLiteral",
            "Displacement"
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

#endif // ALVM_ALASM_LEXER_H
