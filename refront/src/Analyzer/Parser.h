#ifndef CMC_ANALYZER_PARSER_H
#define CMC_ANALYZER_PARSER_H

#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <stack>
#include <vector>

#include "Lexer.h"

namespace relang::refront {
    namespace ast {
        enum class FundamentalType : u8
        {
            Void,
            Integer32,
            Integer64,
            Boolean,
            Character,
            String,
            UserDefined
        };

        enum class StatementKind : u8
        {
            None,
            VariableDeclaration,
            FunctionDeclaration,
            FunctionParameter,
            FunctionParemeterList,
            Initializer,
            FunctionCallExpression,
            FunctionArgument,
            FunctionArgumentList,
            ImportDirective,
            NoOperationStatement,

            ArrayLengthSpecifier,
            InitializerList,

            EqualsExpression,
            NotEqualsExpression,
            GreaterExpression,
            GreaterThanExpression,
            LesserExpression,
            LesserThanExpression,
            GreaterThanOrEqualExpression,
            LesserThanOrEqualExpression,

            AssignmentExpression,
            AdditionExpression,
            SubtractionExpression,
            MultiplicationExpression,
            DivisionExpression,

            IfStatement,
            ElseIfStatement,
            ElseStatement,
            WhileStatement,
            ReturnStatement,
            BlockStatement,

            IdentifierName,
            LiteralExpression
        };

        struct Type
        {
        public:
            std::string       name = "Void";
            FundamentalType   ftype{};
            std::vector<Type> fields{}; // For user defined types.
            usize             length{}; // In element count (for arrays).
            i8                size{};   // In bits.

        public:
            static Type Integer32;
            static Type Integer64;
            static Type Character;
            static Type Boolean;

        public:
            static Type String(const Token& string_token) noexcept;

        public:
            static std::optional<Type> FromToken(const Token& token) noexcept;

        public:
            inline bool        IsArray() const noexcept { return length > 0; }
            inline bool        IsVoid() const noexcept { return ftype == FundamentalType::Void; }
            inline std::string ToString() const noexcept
            {
                std::string str{};
                if (IsArray())
                    str = name + "[" + std::to_string(length) + "]";
                else
                    str = name;
                return str;
            }

        public:
            inline bool operator==(const Type& type) const noexcept
            {
                return this->ftype == type.ftype && this->name == type.name && this->length == type.length;
            }
        };

        struct Statement
        {
        public:
            std::string            name{};
            StatementKind          kind{};
            std::vector<Statement> children{};
            Type                   type{};
            std::vector<Token>     tokens{};

        public:
            std::optional<Token> GetToken(const TokenType& type) const noexcept;
        };

        using SyntaxTree = std::vector<ast::Statement>;

        struct Symbol
        {
            std::string    name{};
            ast::Statement statement;
        };

        struct SymbolTable
        {
        private:
            std::unordered_map<std::string, Symbol> m_Symbols{};

        public:
            void          AddSymbol(Symbol symbol) noexcept;
            bool          ContainsSymbol(const std::string& name) const noexcept;
            Symbol&       GetSymbol(const std::string& name) noexcept;
            const Symbol& GetSymbol(const std::string& name) const noexcept;
        };
    } // namespace ast

    class Parser
    {
    private:
        std::string_view              m_Source{};
        Lexer                         m_Lexer{};
        std::optional<Token>          m_CurrentToken{};
        std::vector<ast::Statement>   m_GlobalStatements{};
        std::vector<ast::SymbolTable> m_SymbolTableStack{};

    public:
        explicit Parser(const std::string_view source) noexcept;

    public:
        std::vector<ast::Statement> Parse();

    private:
        std::optional<Token>          Consume() noexcept;
        std::optional<Token>          Peek() noexcept;
        std::optional<ast::Statement> GetStatement(const ast::StatementKind kind) const noexcept;
        std::optional<ast::Statement> ExpectFunctionDecl();
        std::optional<ast::Statement> ExpectImportDirective();
        ast::Statement                ExpectFunctionParameterList();
        std::optional<ast::Statement> ExpectLocalStatement();
        std::optional<ast::Statement> ExpectBlockStatement();
        std::optional<ast::Statement> ExpectVariableDeclaration();
        std::optional<ast::Statement> ExpectKeyword();
        std::optional<ast::Statement> ExpectExpression();
        std::optional<ast::Statement> ExpectPrimaryExpression();
        std::optional<ast::Statement> ExpectLiteral();
        std::optional<ast::Statement> ExpectIdentifierName();
        std::optional<ast::Statement> ExpectInitializerList();
        std::optional<ast::Statement> ExpectFunctionCall();
        ast::Statement                ExpectFunctionArgumentList();
        std::optional<ast::Statement> ExpectAssignment();
        std::optional<ast::Statement> ExpectAddition();
        std::optional<ast::Statement> ExpectCondition();
        std::optional<ast::Statement> ExpectMultiplication();
    };
} // namespace relang::refront

namespace nlohmann {
    template <>
    struct adl_serializer<relang::refront::ast::StatementKind>
    {
        static void to_json(ordered_json& j, const relang::refront::ast::StatementKind& e)
        {
            switch (e)
            {
                using enum relang::refront::ast::StatementKind;

                case None: j = "None"; break;
                case VariableDeclaration: j = "VariableDeclaration"; break;
                case FunctionDeclaration: j = "FunctionDeclaration"; break;
                case FunctionParameter: j = "FunctionParameter"; break;
                case FunctionParemeterList: j = "FunctionParameterList"; break;
                case ImportDirective: j = "ImportDirective"; break;
                case NoOperationStatement: j = "NoOperationStatement"; break;
                case Initializer: j = "Initializer"; break;
                case FunctionCallExpression: j = "FunctionCallExpression"; break;
                case FunctionArgument: j = "FunctionArgument"; break;
                case FunctionArgumentList: j = "FunctionArgumentList"; break;
                case ArrayLengthSpecifier: j = "ArrayLengthSpecifier"; break;
                case InitializerList: j = "InitializerList"; break;
                case EqualsExpression: j = "EqualsExpression"; break;
                case NotEqualsExpression: j = "NotEqualsExpresion"; break;
                case GreaterThanExpression: j = "GreaterThanExpression"; break;
                case LesserThanExpression: j = "LesserThanExpression"; break;
                case LesserExpression: j = "LesserExpression"; break;
                case GreaterThanOrEqualExpression: j = "GreaterThanOrEqualExpression"; break;
                case GreaterExpression: j = "GreaterEpxression"; break;
                case LesserThanOrEqualExpression: j = "LesserThanOrEqualExpression"; break;
                case AssignmentExpression: j = "AssignmentExpression"; break;
                case AdditionExpression: j = "AdditionExpression"; break;
                case SubtractionExpression: j = "SubtractionExpression"; break;
                case MultiplicationExpression: j = "MultiplicationExpression"; break;
                case DivisionExpression: j = "DivisionExpression"; break;
                case IfStatement: j = "IfStatement"; break;
                case ElseIfStatement: j = "ElseIfStatement"; break;
                case ElseStatement: j = "ElseStatement"; break;
                case WhileStatement: j = "WhileStatement"; break;
                case ReturnStatement: j = "ReturnStatement"; break;
                case BlockStatement: j = "BlockStatement"; break;
                case IdentifierName: j = "IdentifierName"; break;
                case LiteralExpression: j = "LiteralExpression"; break;
                default: j = "Unknown"; break;
            }
        }

        // Do not need a from_json() for now.
    };

    template <>
    struct adl_serializer<relang::refront::ast::FundamentalType>
    {
        static void to_json(ordered_json& j, const relang::refront::ast::FundamentalType& e)
        {
            switch (e)
            {
                using enum relang::refront::ast::FundamentalType;

                case Void: j = "Void"; break;
                case Integer32: j = "Integer32"; break;
                case Integer64: j = "Integer64"; break;
                case Boolean: j = "Boolean"; break;
                case Character: j = "Character"; break;
                case String: j = "String"; break;
                case UserDefined: j = "UserDefined"; break;
                default: j = "Unknown"; break;
            }
        }
    };

    template <>
    struct adl_serializer<relang::refront::ast::Type>
    {
        static void to_json(ordered_json& j, const relang::refront::ast::Type& type)
        {
            j["name"]   = type.name;
            j["ftype"]  = type.ftype;
            j["fields"] = type.fields;
            j["length"] = type.length;
            j["size"]   = type.size;
        }
    };

    template <>
    struct adl_serializer<relang::refront::ast::Statement>
    {
        static void to_json(ordered_json& j, const relang::refront::ast::Statement& s)
        {
            j["name"]     = s.name;
            j["kind"]     = s.kind;
            j["children"] = s.children;
            j["type"]     = s.type;
            j["tokens"]   = s.tokens;
        }
    };
} // namespace nlohmann

namespace std {
    std::string to_string(const relang::refront::ast::Type& type) noexcept;
} // namespace std

#endif // CMC_ANALYZER_PARSER_H
