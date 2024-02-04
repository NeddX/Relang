#include "Parser.h"

// TODO: Improve and instead use exceptions.
#define CompileError(token, ...)                                                                                       \
    std::cerr << fmt::format("Compile Error @ line ({}, {}): ", (token).span.line, (token).span.cur)                   \
              << fmt::format(__VA_ARGS__) << std::endl;                                                                \
    std::exit(-1);

namespace relang::refront {
    using namespace ast;

    namespace ast {
        Type Type::Integer32 = Type{ .name = "Integer32", .ftype = FundamentalType::Integer32, .size = 32 };
        Type Type::Integer64 = Type{ .name = "Integer64", .ftype = FundamentalType::Integer64, .size = 64 };
        Type Type::Character = Type{ .name = "Character8", .ftype = FundamentalType::Character, .size = 8 };
        Type Type::Boolean   = Type{ .name = "Boolean", .ftype = FundamentalType::Boolean, .size = 8 };

        Type Type::String(const Token& string_token) noexcept
        {
            auto type = Type{ .name = "CString", .ftype = FundamentalType::String };
            type.size = string_token.span.text.size();
            return type;
        }

        std::optional<Token> Statement::GetToken(const TokenType& type) const noexcept
        {
            for (const auto& t : tokens)
            {
                if (t.type == type)
                    return t;
            }
            return std::nullopt;
        }
    } // namespace ast

    void SymbolTable::AddSymbol(Symbol symbol) noexcept
    {
        m_Symbols[symbol.name] = std::move(symbol);
    }

    bool SymbolTable::ContainsSymbol(const std::string& name) const noexcept
    {
        return m_Symbols.contains(name);
    }

    Symbol& SymbolTable::GetSymbol(const std::string& name) noexcept
    {
        return m_Symbols[name];
    }

    const Symbol& SymbolTable::GetSymbol(const std::string& name) const noexcept
    {
        return ((SymbolTable*)this)->GetSymbol(name);
    }

    std::optional<Type> Type::FromToken(const Token& token) noexcept
    {
        switch (token.type)
        {
            using enum TokenType;

            case KeywordI32: return Type::Integer32;
            case KeywordI64: return Type::Integer64;
            case KeywordString: return Type::String(token);
            case KeywordChar: return Type::Character;
            case KeywordBool: return Type::Boolean;
            case Identifier: return Type{ .name = token.span.text, .ftype = FundamentalType::UserDefined };
            default: break;
        }
        return std::nullopt;
    }

    Parser::Parser(const std::string_view source) noexcept : m_Source(source)
    {
    }

    std::vector<Statement> Parser::Parse()
    {
        m_Lexer        = Lexer(m_Source);
        m_CurrentToken = m_Lexer.NextToken();
        while (m_CurrentToken->IsValid())
        {
            if (auto c = ExpectFunctionDecl(); c.has_value())
                m_GlobalStatements.push_back(std::move(*c));
        }
        return m_GlobalStatements;
    }

    std::optional<Token> Parser::Consume() noexcept
    {
        auto current   = m_CurrentToken;
        m_CurrentToken = m_Lexer.NextToken();
        return current;
    }

    std::optional<Token> Parser::Peek() noexcept
    {
        return m_Lexer.PeekToken();
    }

    std::optional<Statement> Parser::GetStatement(const StatementKind kind) const noexcept
    {
        for (const auto& e : m_GlobalStatements)
        {
            if (e.kind == kind)
                return e;
        }
        return std::nullopt;
    }

    std::optional<Statement> Parser::ExpectFunctionDecl()
    {
        if (m_CurrentToken->type == TokenType::KeywordFn)
        {
            // Consume the Fn keyword.
            auto prev_token = *Consume();

            // Our function declaration statement.
            Statement func_stmt{};

            // If the following token is an identifier.
            if (m_CurrentToken->type == TokenType::Identifier)
            {
                // Consume the identifier.
                prev_token     = *Consume();
                func_stmt.name = prev_token.span.text;
                func_stmt.kind = StatementKind::FunctionDeclaration;
                func_stmt.tokens.push_back(std::move(prev_token));

                // The function's parameter list symbol table.
                m_SymbolTableStack.push_back(SymbolTable{});

                // Parse possible parameter list, if there's none then our parameter list statement will just be empty.
                auto param_list = ExpectFunctionParameterList();
                func_stmt.children.push_back(std::move(param_list));

                // Parse the possible return type or a function scope start.
                if (m_CurrentToken->IsValid())
                {
                    // Parse the possible arrow return type specifier.
                    if (m_CurrentToken->type == TokenType::Minus)
                    {
                        // Consume the dash.
                        Consume();

                        if (m_CurrentToken->IsValid() && m_CurrentToken->type == TokenType::RightAngleBracket)
                        {
                            // Consume the arrow.
                            Consume();

                            auto type_opt = Type::FromToken(*m_CurrentToken);
                            if (type_opt)
                            {
                                // Consume the type.
                                Consume();
                                func_stmt.type = std::move(*type_opt);
                            }
                            else
                            {
                                CompileError(*m_CurrentToken, "Unknown type '{}'.", m_CurrentToken->span.text);
                            }
                        }
                        else
                        {
                            CompileError(*m_CurrentToken, "Expected an arrow return type specifier.");
                        }
                    }

                    // Parse the function body.
                    auto body_stmt = ExpectLocalStatement();
                    if (body_stmt)
                        func_stmt.children.push_back(std::move(*body_stmt));
                    else
                    {
                        CompileError(*m_CurrentToken, "Expected a statement.");
                    }
                }
                else
                {
                    CompileError(*m_CurrentToken,
                                 "Expected a function return type specifier or a function scope start.");
                }

                // Pop the function's parameter list symbol table.
                m_SymbolTableStack.pop_back();

                return func_stmt;
            }
            else
            {
                CompileError(prev_token, "Expected an Identifier token but got an {} token.",
                             m_CurrentToken->ToString());
            }
        }
        return std::nullopt;
    }

    Statement Parser::ExpectFunctionParameterList()
    {
        Statement params{};
        if (m_CurrentToken->type == TokenType::LeftBrace)
        {
            // Consume the left brace.
            auto prev_token = *Consume();

            // Our possible parameter.
            Statement parameter{};

            // If our token is not eof.
            while (m_CurrentToken->IsValid())
            {
                // Possible parameter definition.
                if (m_CurrentToken->type == TokenType::Identifier)
                {
                    // Consume the identifier.
                    auto ident = *Consume();

                    parameter.name = ident.span.text;
                    parameter.kind = StatementKind::FunctionParameter;
                    parameter.tokens.push_back(std::move(ident));

                    // Next, we expect the token to be valid and a colon because
                    // types are defined in the following syntax: identifier: type, ...
                    if (m_CurrentToken->IsValid() && m_CurrentToken.value().type == TokenType::Colon)
                    {
                        // Consume the colon.
                        Consume();

                        // If the following token is valid and a keyword,
                        // hopefully a type.
                        if (m_CurrentToken->IsValid() && m_CurrentToken.value().IsKeyword())
                        {
                            // Consume the possible type token.
                            auto type_token = *Consume();

                            // Try and create a type from the token. If we get a nothing then it was not a type
                            // so throw a compile error and exit.
                            auto type_opt = Type::FromToken(type_token);
                            if (type_opt)
                                parameter.type = std::move(*type_opt);
                            else
                            {
                                CompileError(type_token, "Expected a type, instead got a {}", type_token.ToString());
                            }
                        }
                        else
                        {
                            CompileError(*m_CurrentToken, "Expected a type specifier for the parameter.");
                        }
                    }
                    else
                    {
                        CompileError(*m_CurrentToken, "Expected a type specifier for the parameter.");
                    }

                    // Append our parameter to the function's current symbol table.
                    m_SymbolTableStack.back().AddSymbol(Symbol{ .name = parameter.name, .statement = parameter });

                    // Finally, push our parameter statement to our parameter list.
                    params.children.push_back(std::move(parameter));
                }
                else if (m_CurrentToken->type == TokenType::Comma)
                {
                    // There are more parameters so just progress forward.
                    Consume();
                }
                else if (m_CurrentToken->type == TokenType::RightBrace)
                {
                    // We've reached the end so terminate.
                    break;
                }
                else
                {
                    CompileError(*m_CurrentToken, "Expected a function parameter.");
                }
            }

            if (!m_CurrentToken->IsValid())
            {
                CompileError(*m_CurrentToken, "Expected a closing brace after function parameter list declaration.");
            }
            else
            {
                // Consume the right brace.
                Consume();
                params.kind = StatementKind::FunctionParemeterList;
            }
        }
        else
        {
            CompileError(*m_CurrentToken, "Expected a parameter list.");
        }
        return params;
    }

    std::optional<Statement> Parser::ExpectLocalStatement()
    {
        // For keywords and block statements we do not want to check for a
        // semicolon because, well, block statements end with the closing curly
        // and keywords such as while and if can be a single statement or
        // be chained with a block statement which like I stated before, ends with a curly brace.

        // Check for a compound statement.
        auto result = ExpectBlockStatement();
        if (result)
            return result;

        // Else check for a keyword statement.
        if (!result)
        {
            result = ExpectKeyword();
            if (result)
                return result;
        }

        // Else check for a variable declaration statement.
        if (!result)
            result = ExpectVariableDeclaration();

        // Else just check for a possible expression statement.
        if (!result)
            result = ExpectExpression();

        // Check for the semicolon.
        if (m_CurrentToken->type == TokenType::SemiColon)
        {
            // Consume the semicolon.
            Consume();

            // If we don't have a result then it is a no-op statement.
            if (!result)
                return Statement{ .kind = StatementKind::NoOperationStatement };
        }
        else
        {
            CompileError(*m_CurrentToken, "Expected a semicolon but got {} instead.", m_CurrentToken->ToString());
        }

        return result;
    }

    std::optional<ast::Statement> Parser::ExpectBlockStatement()
    {
        // Check for a start of a block statement.
        if (m_CurrentToken->type == TokenType::LeftCurlyBrace)
        {
            // Create a new symbol table for our compound statement and push it onto the stack.
            m_SymbolTableStack.push_back(SymbolTable{});

            // Consume the left curly brace.
            auto brace_token = *Consume();

            // Our block statement.
            Statement block_stmt;
            block_stmt.kind = StatementKind::BlockStatement;
            block_stmt.tokens.push_back(std::move(brace_token));

            // Iterate through the tokens until we hit a closing curly brace.
            while (m_CurrentToken->type != TokenType::RightCurlyBrace)
            {
                // If we meet a EOF instead of a closing curly brace.
                if (!m_CurrentToken->IsValid())
                {
                    CompileError(*m_CurrentToken, "Expected a closing curly brace to end the block statement.");
                }
                else
                {
                    // Recursevly parse statements and append them to our block statement (if any).
                    auto stmt = ExpectLocalStatement();
                    if (stmt)
                        block_stmt.children.push_back(std::move(*stmt));
                }
            }

            // Consume the closing curly brace.
            brace_token = *Consume();
            block_stmt.tokens.push_back(std::move(brace_token));

            // Pop our compound statement's symbol table out and finally return our compound statement.
            m_SymbolTableStack.pop_back();
            return block_stmt;
        }
        return std::nullopt;
    }

    std::optional<ast::Statement> Parser::ExpectVariableDeclaration()
    {
        if (m_CurrentToken->IsValid() && m_CurrentToken.value().type == TokenType::KeywordLet)
        {
            // Consume our let token.
            Token let_token = *Consume();
            Token ident_token{};

            // Our variable declaration statement.
            Statement var_decl{};
            var_decl.kind = StatementKind::VariableDeclaration;
            var_decl.tokens.push_back(std::move(let_token));

            // The following token must be valid and an identifier.
            if (m_CurrentToken->IsValid() && m_CurrentToken.value().type == TokenType::Identifier)
            {
                ident_token   = *Consume();
                var_decl.name = ident_token.span.text;
                var_decl.tokens.push_back(ident_token);
            }
            else
            {
                CompileError(*m_CurrentToken, "Expected an identifier after the let keyword.");
            }

            // The token following the identifier must be a colon type specifier.
            if (auto token = Consume(); !token->IsValid() || token.value().type != TokenType::Colon)
            {
                CompileError(*m_CurrentToken, "Expected a colon type specifier.");
            }

            // The following token now must be a type.
            if (m_CurrentToken->IsValid())
            {
                // Consume our type token then try and create type from it.
                auto type_token = *Consume();
                auto type_opt   = Type::FromToken(type_token);

                if (type_opt)
                {
                    // Check if it is a possible array.
                    if (m_CurrentToken->type == TokenType::LeftSquareBracket)
                    {
                        // Consume the opening square bracket.
                        Consume();

                        // The following token must be a length specifier in the form of a number literal.
                        if (m_CurrentToken->type == TokenType::NumberLiteral)
                            type_opt->length = Consume().value().num;
                        else
                        {
                            CompileError(*m_CurrentToken,
                                         "Expected an array length specifier in the form of an integer literal.");
                        }

                        // The following token must be a closing square bracket.
                        if (auto rsq_bracket = *Consume(); rsq_bracket.type != TokenType::RightSquareBracket)
                        {
                            CompileError(rsq_bracket, "Expected a closing square bracket.");
                        }
                    }
                    var_decl.type = std::move(*type_opt);
                }
                else
                {
                    CompileError(*m_CurrentToken, "Unknown type {}.", type_token.span.text);
                }
            }
            else
            {
                CompileError(*m_CurrentToken, "Expected a type.");
            }

            // Now we either have a semicolon or initializer.
            if (m_CurrentToken->IsValid())
            {
                // It is an initializer.
                if (m_CurrentToken->type == TokenType::Equals)
                {
                    // Consume the equals.
                    auto equals_token = *Consume();

                    // Our initializer statement.
                    Statement init_stmt{};
                    init_stmt.tokens.push_back(std::move(equals_token));
                    init_stmt.kind = StatementKind::Initializer;

                    // Save the token before expression parsing.
                    auto pre_expr_token = *m_CurrentToken;

                    // The initializer expression.
                    auto init_expr = ExpectExpression();
                    if (init_expr)
                    {
                        // Check if our variable is an array.
                        if (var_decl.type.IsArray())
                        {
                            // If the lengths mismatch then it's an error.
                            if (init_expr->children.size() != var_decl.type.length)
                            {
                                CompileError(init_expr->tokens[0],
                                             "'{}' is an array of {} elements but is initialized with an initializer "
                                             "list of length {}.",
                                             var_decl.name, var_decl.type.length, init_expr->children.size());
                            }

                            // Check if there's a type mismatch.
                            for (const auto& e : init_expr->children)
                            {
                                // Compare the ELEMENT types.
                                if (e.type.ftype != var_decl.type.ftype)
                                {
                                    CompileError(
                                        e.tokens.at(0),
                                        "Type mistmatch. Cannot perform implicit conversion from '{}' to '{}'.",
                                        e.type.ToString(), var_decl.type.ToString());
                                }
                            }
                        }
                        else
                        {
                            // Check if there's a type mismatch between the initializer expression and the variable.
                            if (init_expr->type != var_decl.type)
                            {
                                CompileError(init_stmt.tokens[0],
                                             "Type mismatch. Cannot perform implicit conversion from '{}' to '{}'.",
                                             init_expr->type.ToString(), var_decl.type.ToString());
                            }
                        }

                        // If we reached here then everything is fine so just append our initializer and move on.
                        init_stmt.children.push_back(std::move(*init_expr));
                    }
                    else
                    {
                        CompileError(pre_expr_token, "Expected a valid expression for initialization.");
                    }

                    // Append our initializer statement.
                    var_decl.children.push_back(std::move(init_stmt));
                }

                // Check if the variable already exists in our block's symbol table.
                if (m_SymbolTableStack.back().ContainsSymbol(var_decl.name))
                {
                    auto& sym          = m_SymbolTableStack.back().GetSymbol(var_decl.name);
                    auto& redecl_token = sym.statement.tokens[0];
                    CompileError(var_decl.tokens[0],
                                 "Redeclaration of an already existing name '{}' in the same context previously "
                                 "defined @ line ({}, {}).",
                                 var_decl.name, redecl_token.span.line, redecl_token.span.cur);
                }
                else
                {
                    // Append our new variable to our symbol table and return it.
                    m_SymbolTableStack.back().AddSymbol(Symbol{ .name = ident_token.span.text, .statement = var_decl });
                }
                return var_decl;
            }
        }
        return std::nullopt;
    }

    std::optional<ast::Statement> Parser::ExpectKeyword()
    {
        // If our token is valid and an actual keyword (obviously).
        if (m_CurrentToken->IsValid() && m_CurrentToken.value().IsKeyword())
        {
            switch (m_CurrentToken->type)
            {
                using enum TokenType;

                // TODO: For else and else if statements you can use an if statement stack to determine which if
                // statement do they belong but for now I am not going to support else and else if statements.
                case KeywordIf: {
                    // Consume the if keyword.
                    auto if_keyword = *Consume();

                    // Our If statement.
                    Statement if_stmt{};
                    if_stmt.kind = StatementKind::IfStatement;
                    if_stmt.tokens.push_back(std::move(if_keyword));

                    // Save the token.
                    auto pre_cond_token = *m_CurrentToken;

                    // Else it's just a regular if statement.
                    auto condition = ExpectExpression();
                    if (condition)
                    {
                        // Check if the expression type is a boolean.
                        if (condition->type == Type::Boolean)
                        {
                            // Append our condition statement.
                            if_stmt.children.push_back(std::move(*condition));

                            // Save the token.
                            auto pre_body_token = *m_CurrentToken;

                            // The body for the if statement.
                            auto body_stmt = ExpectLocalStatement();
                            if (body_stmt)
                                if_stmt.children.push_back(std::move(*body_stmt));
                            else
                            {
                                CompileError(pre_body_token, "Expected a body for the if statement.");
                            }

                            // Finally return our if statement.
                            return if_stmt;
                        }
                        else
                        {
                            CompileError(pre_cond_token,
                                         "Type mismatch. Cannot perform implicit conversion from '{}' to '{}'.",
                                         condition->type.ToString(), Type::Boolean.ToString());
                        }
                    }
                    else
                    {
                        CompileError(pre_cond_token, "Expected an expression evaluating to bool.");
                    }
                    break;
                }
                case KeywordWhile: {
                    // Consume the while keyword.
                    auto while_keyword = *Consume();

                    // Our If statement.
                    Statement while_stmt{};
                    while_stmt.kind = StatementKind::WhileStatement;
                    while_stmt.tokens.push_back(std::move(while_keyword));

                    // Save the token.
                    auto pre_cond_token = *m_CurrentToken;

                    // Our while's condition statement.
                    auto condition = ExpectExpression();
                    if (condition)
                    {
                        // Check if the expression type is a boolean.
                        if (condition->type == Type::Boolean)
                        {
                            // Append our condition statement.
                            while_stmt.children.push_back(std::move(*condition));

                            // Save the token.
                            auto pre_body_token = *m_CurrentToken;

                            // The body for the if statement.
                            auto body_stmt = ExpectLocalStatement();
                            if (body_stmt)
                                while_stmt.children.push_back(std::move(*body_stmt));
                            else
                            {
                                CompileError(pre_body_token, "Expected a body for the while statement.");
                            }

                            // Finally return our while statement.
                            return while_stmt;
                        }
                        else
                        {
                            CompileError(pre_cond_token,
                                         "Type mismatch. Cannot perform implicit conversion from '{}' to '{}'.",
                                         condition->type.ToString(), Type::Boolean.ToString());
                        }
                    }
                    else
                    {
                        CompileError(pre_cond_token, "Expected an expression evaluating to bool.");
                    }
                    break;
                }
                case KeywordReturn: {
                    // Consume the return token.
                    Consume();

                    // The return statement.
                    Statement stmt{};
                    stmt.kind = StatementKind::ReturnStatement;

                    // If an expression follows our return statement.
                    auto exp = ExpectExpression();
                    if (exp)
                        stmt.children.push_back(std::move(*exp));

                    // Check for the semicolon of course.
                    if (m_CurrentToken->IsValid() && m_CurrentToken.value().type == TokenType::SemiColon)
                    {
                        // Consume the semicolon.
                        Consume();
                        return stmt;
                    }
                    else
                    {
                        CompileError(*m_CurrentToken, "Expected a semicolon after the return statement.");
                    }
                    break;
                }
                default: break;
            }
        }
        return std::nullopt;
    }

    std::optional<Statement> Parser::ExpectExpression()
    {
        return ExpectCondition();
    }

    std::optional<Statement> Parser::ExpectPrimaryExpression()
    {
        // Check if it's a function call expression.
        auto result = ExpectAssignment();
        if (result)
            return result;

        // Else check for a function call.
        result = ExpectFunctionCall();
        if (result)
            return result;

        // Else check for an initializer list expression.
        result = ExpectInitializerList();
        if (result)
            return result;

        // Check for a literal expression.
        result = ExpectLiteral();
        if (result)
            return result;

        // Else check for an identifier expression.
        result = ExpectIdentifierName();
        if (result)
            return result;

        return std::nullopt;
    }

    std::optional<Statement> Parser::ExpectLiteral()
    {
        // If our token is valid.
        if (m_CurrentToken->IsValid())
        {
            // Check for the type of the literal.
            switch (m_CurrentToken->type)
            {
                using enum TokenType;

                case NumberLiteral: {
                    auto      token = *Consume();
                    Statement stmt{};
                    stmt.kind = StatementKind::LiteralExpression;
                    stmt.type = Type::Integer64;
                    stmt.tokens.push_back(std::move(token));
                    return stmt;
                }
                case StringLiteral: {
                    auto      token = *Consume();
                    Statement stmt{};
                    stmt.kind = StatementKind::LiteralExpression;
                    stmt.type = Type::String(token);
                    stmt.tokens.push_back(std::move(token));
                    return stmt;
                }
                case CharacterLiteral: {
                    auto      token = *Consume();
                    Statement stmt{};
                    stmt.kind = StatementKind::LiteralExpression;
                    stmt.type = Type::Character;
                    stmt.tokens.push_back(std::move(token));
                    return stmt;
                }
                case KeywordTrue:
                case KeywordFalse: {
                    auto      token = *Consume();
                    Statement stmt{};
                    stmt.kind = StatementKind::LiteralExpression;
                    stmt.type = Type::Boolean;
                    stmt.tokens.push_back(std::move(token));
                    return stmt;
                }
                default: break;
            }
        }
        return std::nullopt;
    }

    std::optional<Statement> Parser::ExpectIdentifierName()
    {
        // If the current token is infact an identifier.
        if (m_CurrentToken->IsValid() && m_CurrentToken.value().type == TokenType::Identifier)
        {
            // Consume the identifier token.
            auto ident_token = *Consume();

            // Create our identifier statement.
            Statement name_stmt{};
            name_stmt.kind = StatementKind::IdentifierName;
            name_stmt.name = ident_token.span.text;

            // Perform a symbol table lookup.
            for (auto it = m_SymbolTableStack.rbegin(); it != m_SymbolTableStack.rend(); ++it)
            {
                const auto& e = *it;
                if (e.ContainsSymbol(name_stmt.name))
                    name_stmt.type = e.GetSymbol(name_stmt.name).statement.type;
            }

            // If the type is still void then the lookup most likely failed.
            if (name_stmt.type.IsVoid())
            {
                CompileError(ident_token, "The name '{}' does not exist in the current context.", name_stmt.name);
            }
            else
            {
                // Else everything is fine so return our identifier name reference statement.
                return name_stmt;
            }
        }
        return std::nullopt;
    }

    std::optional<Statement> Parser::ExpectInitializerList()
    {
        // Check if the token is infact an opening curly brace.
        if (m_CurrentToken->type == TokenType::LeftCurlyBrace)
        {
            // Consume the opening curly brace.
            auto left_curly = *Consume();

            // Our initializer list statement.
            Statement init_list{};
            init_list.kind = StatementKind::InitializerList;
            init_list.tokens.push_back(std::move(left_curly));

            // Parse the tokens until we hit a closing curly brace.
            while (m_CurrentToken->type != TokenType::RightCurlyBrace)
            {
                // Parse the expression element.
                auto expr = ExpectExpression();
                if (expr)
                {
                    // Either a comma must follow our parsed expression or the initializer list should end, otherwise
                    // it's a compile error.
                    if (m_CurrentToken->type == TokenType::Comma)
                        // Consume the comma and move on.
                        Consume();
                    else if (m_CurrentToken->type != TokenType::RightCurlyBrace)
                    {
                        CompileError(*m_CurrentToken, "Expected a closing curly brace.");
                    }
                    init_list.children.push_back(std::move(*expr));
                }
                else
                {
                    CompileError(*m_CurrentToken, "Invalid expression inside of an initializer list.");
                }
            }

            // Check if our initializer list was properly established.
            if (auto closing_curly = *Consume(); closing_curly.IsValid())
            {
                // Consume and append our closing curly brace to the initializer list statement.
                init_list.tokens.push_back(closing_curly);
                init_list.type        = init_list.children.back().type;
                init_list.type.length = init_list.children.size();
                return init_list;
            }
            else
            {
                CompileError(*m_CurrentToken, "Expected a closing curly brace.");
            }
        }
        return std::nullopt;
    }

    std::optional<ast::Statement> Parser::ExpectFunctionCall()
    {
        // Check for the identifier.
        if (m_CurrentToken->type == TokenType::Identifier)
        {
            // Check if the following token (without consuming casue we are unsure) is an opening brace.
            if (Peek()->type == TokenType::LeftBrace)
            {
                // Now we definitely know that it's a function call.
                auto ident_token = *Consume();

                // Try and find the function.
                Statement ref_fn{};
                for (const auto& fn : m_GlobalStatements)
                {
                    if (fn.kind == StatementKind::FunctionDeclaration)
                    {
                        if (fn.name == ident_token.span.text)
                            ref_fn = fn;
                    }
                }

                // If the function was not found.
                if (ref_fn.kind == StatementKind::None)
                {
                    CompileError(ident_token, "The name '{}' does not exist in the current context.",
                                 ident_token.span.text);
                }

                // Our function call statement.
                Statement func_call{};
                func_call.name = ident_token.span.text;
                func_call.kind = StatementKind::FunctionCallExpression;
                func_call.type = ref_fn.type;
                func_call.tokens.push_back(std::move(ident_token));

                auto arg_list = ExpectFunctionArgumentList();

                // Check for a type mismatch.
                for (usize i = 0; i < arg_list.children.size(); ++i)
                {
                    if (arg_list.children[i].type != ref_fn.children[0].children[i].type)
                    {
                        CompileError(ident_token,
                                     "Cannot perform implicit conversion from '{}' to '{}'. No matching function "
                                     "call to '{}'.",
                                     arg_list.children[i].type.ToString(),
                                     ref_fn.children[0].children[i].type.ToString(), func_call.name);
                    }
                }

                // Finally, return our function call statement.
                func_call.children.push_back(std::move(arg_list));
                return func_call;
            }
        }
        return std::nullopt;
    }

    ast::Statement Parser::ExpectFunctionArgumentList()
    {
        Statement args{};
        if (m_CurrentToken->type == TokenType::LeftBrace)
        {
            // Consume the left brace.
            auto prev_token = *Consume();

            // If our token is not eof.
            while (m_CurrentToken->IsValid())
            {
                if (m_CurrentToken->type == TokenType::Comma)
                {
                    // There are more parameters so just progress forward.
                    Consume();
                }
                else if (m_CurrentToken->type == TokenType::RightBrace)
                {
                    // We've reached the end so terminate.
                    break;
                }
                else
                {
                    auto expr = ExpectExpression();
                    if (expr)
                    {
                        args.children.push_back(std::move(*expr));
                    }
                    else
                    {
                        CompileError(*m_CurrentToken, "Expected a function argument.");
                    }
                }
            }

            if (!m_CurrentToken->IsValid())
            {
                CompileError(*m_CurrentToken, "Expected a closing brace after the function argument list.");
            }
            else
            {
                // Consume the right brace.
                Consume();
                args.kind = StatementKind::FunctionArgumentList;
            }
        }
        else
        {
            CompileError(*m_CurrentToken, "Expected an argument list.");
        }
        return args;
    }

    std::optional<ast::Statement> Parser::ExpectAssignment()
    {
        if (m_CurrentToken->type == TokenType::Identifier)
        {
            if (Peek()->type == TokenType::Equals)
            {
                // Parse our identifier.
                auto lhv = *ExpectIdentifierName();

                // Consuem the equals token.
                auto equals_token = *Consume();

                auto pre_rhv_token = *m_CurrentToken;

                auto rhv = ExpectExpression();
                if (rhv)
                {
                    if (rhv->type == lhv.type)
                    {
                        Statement assign_expr{};
                        assign_expr.type = lhv.type;
                        assign_expr.kind = StatementKind::AssignmentExpression;
                        assign_expr.children.push_back(std::move(lhv));
                        assign_expr.children.push_back(std::move(*rhv));
                        return assign_expr;
                    }
                    else
                    {
                        CompileError(pre_rhv_token,
                                     "Type mismatch. Cannot perform implicit conversion from '{}' to '{}'.",
                                     rhv->type.ToString(), lhv.type.ToString());
                    }
                }
                else
                {
                    CompileError(equals_token, "Expected a valid expression after the equals operator.");
                }
            }
        }
        return std::nullopt;
    }

    std::optional<ast::Statement> Parser::ExpectAddition()
    {
        auto result = ExpectMultiplication();
        while (m_CurrentToken->type == TokenType::Plus || m_CurrentToken->type == TokenType::Minus)
        {
            auto op_token = *Consume();
            auto rhv_expr = ExpectMultiplication();

            // Our multiplication expression.
            Statement binary_expr{};

            if (result)
            {
                binary_expr.kind = (op_token.type == TokenType::Plus) ? StatementKind::AdditionExpression
                                                                      : StatementKind::SubtractionExpression;
                switch (result->type.ftype)
                {
                    // We support fundamental types for now.
                    using enum FundamentalType;

                    case Integer32:
                    case Integer64: {
                        if (result->type == rhv_expr->type)
                        {
                            binary_expr.type = result->type;
                            binary_expr.tokens.push_back(std::move(op_token));
                            binary_expr.children.push_back(std::move(*result));
                            binary_expr.children.push_back(std::move(*rhv_expr));
                            result = std::move(binary_expr);
                        }
                        else
                        {
                            CompileError(*m_CurrentToken,
                                         "Type mismatch. Cannot perform implicit conversion from '{}' to '{}'.",
                                         result->type.ToString(), rhv_expr->type.ToString());
                        }
                        break;
                    }
                    default: {
                        CompileError(op_token, "Cannot perform '{}' on type {}.", op_token.span.text,
                                     result->type.ToString());
                        break;
                    }
                }
            }
            else
            {
                CompileError(op_token, "Expected an expression on the left hand side of the '{}' operator",
                             op_token.span.text);
            }
        }
        return result;
    }

    std::optional<ast::Statement> Parser::ExpectMultiplication()
    {
        auto result = ExpectPrimaryExpression();
        while (m_CurrentToken->type == TokenType::Asterisk || m_CurrentToken->type == TokenType::ForwardSlash)
        {
            auto op_token = *Consume();
            auto rhv_expr = ExpectPrimaryExpression();

            // Our multiplication expression.
            Statement binary_expr{};

            if (result)
            {
                binary_expr.kind = (op_token.type == TokenType::Asterisk) ? StatementKind::MultiplicationExpression
                                                                          : StatementKind::DivisionExpression;
                switch (result->type.ftype)
                {
                    // We support fundamental types for now.
                    using enum FundamentalType;

                    case Integer32:
                    case Integer64: {
                        if (result->type == rhv_expr->type)
                        {
                            binary_expr.type = result->type;
                            binary_expr.tokens.push_back(std::move(op_token));
                            binary_expr.children.push_back(std::move(*result));
                            binary_expr.children.push_back(std::move(*rhv_expr));
                            result = std::move(binary_expr);
                        }
                        else
                        {
                            CompileError(*m_CurrentToken,
                                         "Type mismatch. Cannot perform implicit conversion from '{}' to '{}'.",
                                         result->type.ToString(), rhv_expr->type.ToString());
                        }

                        break;
                    }
                    default: {
                        CompileError(op_token, "Cannot perform '{}' on type {}.", op_token.span.text,
                                     result->type.ToString());
                        break;
                    }
                }
            }
            else
            {
                CompileError(op_token, "Expected an expression on the left hand side of the '{}' operator",
                             op_token.span.text);
            }
        }
        return result;
    }

    std::optional<ast::Statement> Parser::ExpectCondition()
    {
        auto result = ExpectAddition();

        while (m_CurrentToken->type == TokenType::RightAngleBracket ||
               m_CurrentToken->type == TokenType::LeftAngleBracket || m_CurrentToken->type == TokenType::EqualsEquals ||
               m_CurrentToken->type == TokenType::GreaterThanEquals ||
               m_CurrentToken->type == TokenType::LesserThanEquals ||
               m_CurrentToken->type == TokenType::ExclamationEquals)
        {
            auto op_token = *Consume();
            auto rhv_expr = ExpectAddition();

            // Our multiplication expression.
            Statement binary_expr{};

            if (result)
            {
                switch (op_token.type)
                {
                    using enum TokenType;

                    case RightAngleBracket: binary_expr.kind = StatementKind::GreaterExpression; break;
                    case LeftAngleBracket: binary_expr.kind = StatementKind::LesserExpression; break;
                    case EqualsEquals: binary_expr.kind = StatementKind::EqualsExpression; break;
                    case GreaterThanEquals: binary_expr.kind = StatementKind::GreaterThanExpression; break;
                    case LesserThanEquals: binary_expr.kind = StatementKind::LesserThanExpression; break;
                    case ExclamationEquals: binary_expr.kind = StatementKind::NotEqualsExpression; break;
                    default: break;
                }

                // FIXME: This is buggy but whatever.
                if (result->type == rhv_expr->type)
                {
                    binary_expr.type = Type::Boolean;
                    binary_expr.tokens.push_back(std::move(op_token));
                    binary_expr.children.push_back(std::move(*result));
                    binary_expr.children.push_back(std::move(*rhv_expr));
                    result = std::move(binary_expr);
                }
                else
                {
                    CompileError(*m_CurrentToken,
                                 "Type mismatch. Cannot perform implicit conversion from '{}' to '{}'.",
                                 result->type.ToString(), rhv_expr->type.ToString());
                }
            }
            else
            {
                CompileError(op_token, "Expected an expression on the left hand side of the '{}' operator",
                             op_token.span.text);
            }
        }
        return result;
    }
} // namespace relang::refront

namespace std {
    std::string to_string(const relang::refront::ast::Type& type) noexcept
    {
        return type.ToString();
    }
} // namespace std
