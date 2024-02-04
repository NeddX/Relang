#ifndef BLEND_BASM_LEXER_H
#define BLEND_BASM_LEXER_H

#include <BLEND.h>
#include <string>
#include <vector>

#define GET_TOKEN_STR(token) Token::TokenStr[(usize)token]

namespace relang::rmc {
    // Forward declerations
    struct Token;

    using TokenList = std::vector<Token>;

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

    public:
        TokenType type = TokenType::Whitespace;
        std::string text;
        u64 data = 0;
        usize line = 0;
        usize cur = 0;

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

    class Lexer
    {
    public:
        static TokenList Start(const std::string& src);

    private:
        static void EndToken(Token& t, TokenList& tokens);
    };
} // namespace relang::rmc

#endif // BLEND_BASM_LEXER_H
