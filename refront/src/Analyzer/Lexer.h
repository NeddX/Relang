#ifndef CMC_ANALYZER_LEXER_H
#define CMC_ANALYZER_LEXER_H

#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>
#include <vector>

#include <CommonDef.h>

namespace relang::refront {
    enum class TokenType : u32
    {
        None,

        // Identifier
        Identifier,

        // Literals
        NumberLiteral,
        StringLiteral,
        CharacterLiteral,

        // Operators
        Colon,
        SemiColon,
        Equals,
        LeftBrace,
        RightBrace,
        LeftCurlyBrace,
        RightCurlyBrace,
        Plus,
        Minus,
        Asterisk,
        ForwardSlash,
        LeftAngleBracket,
        RightAngleBracket,
        LeftSquareBracket,
        RightSquareBracket,
        DoubleQuote,
        Quote,
        Comma,
        Exclamation,
        Bar,

        // Conditionals
        EqualsEquals,
        GreaterThanEquals,
        LesserThanEquals,
        ExclamationEquals,
        AmpersasndAmpersand,
        BarBar,

        // Unary
        Increment,
        Decrement,

        // Shorthand
        PlusEquals,
        MinusEquals,
        AsteriskEquals,
        ForwardSlashEquals,

        // Keywords
        KeywordLet,
        KeywordFn,
        KeywordImport,
        KeywordIf,
        KeywordElse,
        KeywordElseIf,
        KeywordI32,
        KeywordI64,
        KeywordString,
        KeywordBool,
        KeywordChar,
        KeywordWhile,
        KeywordReturn,
        KeywordTrue,
        KeywordFalse,

        // Eof
        Eof
    };

    std::string_view TokenTypeToString(const TokenType type) noexcept;

    struct TextSpan
    {
        usize       line{};
        usize       cur{};
        std::string text{};
    };

    struct Token
    {
    public:
        TokenType type = TokenType::None;
        TextSpan  span{};
        i64       num{};

    public:
        // Few handy methods to make parsing easier.
        bool IsValid() const noexcept { return type != TokenType::None && type != TokenType::Eof; }
        bool IsOperator() const noexcept
        {
            switch (type)
            {
                using enum TokenType;

                case Colon:
                case SemiColon:
                case Equals:
                case LeftBrace:
                case RightBrace:
                case LeftCurlyBrace:
                case RightCurlyBrace:
                case Plus:
                case Minus:
                case Asterisk:
                case ForwardSlash:
                case LeftAngleBracket:
                case RightAngleBracket:
                case LeftSquareBracket:
                case RightSquareBracket:
                case DoubleQuote:
                case Quote:
                case Comma:
                case Exclamation:
                case Bar:
                case EqualsEquals:
                case GreaterThanEquals:
                case LesserThanEquals:
                case ExclamationEquals:
                case AmpersasndAmpersand:
                case BarBar:
                case Increment:
                case Decrement:
                case PlusEquals:
                case MinusEquals:
                case AsteriskEquals:
                case ForwardSlashEquals: return true;
                default: break;
            }
            return false;
        }
        bool IsKeyword() const noexcept
        {
            switch (type)
            {
                using enum TokenType;

                case KeywordLet:
                case KeywordFn:
                case KeywordImport:
                case KeywordIf:
                case KeywordElse:
                case KeywordElseIf:
                case KeywordI32:
                case KeywordI64:
                case KeywordString:
                case KeywordBool:
                case KeywordChar:
                case KeywordWhile:
                case KeywordReturn:
                case KeywordTrue:
                case KeywordFalse: return true;
                default: break;
            }
            return false;
        }
        std::string_view ToString() const noexcept { return TokenTypeToString(type); }
    };

    using TokenList = std::vector<Token>;

    class Lexer
    {
    private:
        std::string_view m_Source{};
        usize            m_CurrentPos{};
        usize            m_TokenCount{};
        usize            m_LineCount = 1;

    public:
        Lexer() = default;
        explicit Lexer(const std::string_view source);

    public:
        std::optional<Token> NextToken();
        std::optional<Token> PeekToken();

    private:
        std::optional<char>        CurrentChar() const noexcept;
        std::optional<char>        Consume() noexcept;
        i64                        ConsumeNumber() noexcept;
        std::string                ConsumeIdentifier() noexcept;
        TokenType                  ConsumeOperator() noexcept;
        std::optional<std::string> ConsumeString() noexcept;
        bool                       IsIdentifierStart(const char c) const noexcept;
    };

} // namespace relang::refront

namespace nlohmann {
    template <>
    struct adl_serializer<relang::refront::TokenType>
    {
        static void to_json(ordered_json& j, const relang::refront::TokenType& e)
        {
            switch (e)
            {
                using enum relang::refront::TokenType;

                case None: j = "None"; break;
                case Identifier: j = "Identifier"; break;
                case NumberLiteral: j = "NumberLiteral"; break;
                case StringLiteral: j = "StringLiteral"; break;
                case CharacterLiteral: j = "CharacterLiteral"; break;
                case Colon: j = "Colon"; break;
                case SemiColon: j = "SemiColon"; break;
                case Equals: j = "Equals"; break;
                case LeftBrace: j = "LeftBrace"; break;
                case RightBrace: j = "RightBrace"; break;
                case LeftCurlyBrace: j = "LeftCurlyBrace"; break;
                case RightCurlyBrace: j = "RightCurlyBrace"; break;
                case Plus: j = "Plus"; break;
                case Minus: j = "Minus"; break;
                case Asterisk: j = "Asterisk"; break;
                case ForwardSlash: j = "ForwardSlash"; break;
                case LeftAngleBracket: j = "LeftAngleBracket"; break;
                case RightAngleBracket: j = "RightAngleBracket"; break;
                case LeftSquareBracket: j = "LeftSquareBracket"; break;
                case RightSquareBracket: j = "RightSquareBracket"; break;
                case DoubleQuote: j = "DoubleQuote"; break;
                case Quote: j = "Quote"; break;
                case Comma: j = "Comma"; break;
                case Exclamation: j = "Exclamation"; break;
                case Bar: j = "Bar"; break;
                case EqualsEquals: j = "EqualsEquals"; break;
                case GreaterThanEquals: j = "GreaterThanEquals"; break;
                case LesserThanEquals: j = "LesserThanEquals"; break;
                case ExclamationEquals: j = "ExclamationEquals"; break;
                case AmpersasndAmpersand: j = "AmpersandAmpersand"; break;
                case BarBar: j = "BarBar"; break;
                case Increment: j = "Increment"; break;
                case Decrement: j = "Decrement"; break;
                case PlusEquals: j = "PlusEquals"; break;
                case MinusEquals: j = "MinusEquals"; break;
                case AsteriskEquals: j = "AsteriskEquals"; break;
                case ForwardSlashEquals: j = "ForwardSlashEquals"; break;
                case KeywordLet: j = "KeywordLet"; break;
                case KeywordFn: j = "KeywordFn"; break;
                case KeywordImport: j = "KeywordImport"; break;
                case KeywordIf: j = "KeywordIf"; break;
                case KeywordElse: j = "KeywordElse"; break;
                case KeywordElseIf: j = "KeywordElseIf"; break;
                case KeywordI32: j = "KeywordI32"; break;
                case KeywordI64: j = "KeywordI64"; break;
                case KeywordString: j = "KeywordString"; break;
                case KeywordBool: j = "KeywordBool"; break;
                case KeywordChar: j = "KeywordChar"; break;
                case KeywordWhile: j = "KeywordWhile"; break;
                case KeywordReturn: j = "KeywordReturn"; break;
                case KeywordTrue: j = "KeywordTrue"; break;
                case KeywordFalse: j = "KeywordFalse"; break;
                case Eof: j = "Eof"; break;
                default: j = "Unknown"; break;
            }
        }

        // Do not need a from_json() for now.
    };

    template <>
    struct adl_serializer<relang::refront::TextSpan>
    {
        static void to_json(ordered_json& j, const relang::refront::TextSpan& t)
        {
            j["line"] = t.line;
            j["cur"]  = t.cur;
            j["text"] = t.text;
        }
    };

    template <>
    struct adl_serializer<relang::refront::Token>
    {
        static void to_json(ordered_json& j, const relang::refront::Token& t)
        {
            j["type"] = t.type;
            j["span"] = t.span;
            j["num"]  = t.num;
        }
    };

} // namespace nlohmann

std::ostream& operator<<(std::ostream& stream, const relang::refront::TextSpan& span) noexcept;
std::ostream& operator<<(std::ostream& stream, const relang::refront::Token& token) noexcept;

#endif // CMC_ANALYZER_LEXER_H
