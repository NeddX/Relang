#include "Lexer.h"

namespace rlang::rmc {
    void Token::Dump() const
    {
        std::printf("token { \"%s\" type: %s data: %lu [%lu, %lu] }\n",
                    text.c_str(), GET_TOKEN_STR(type).c_str(), data, line, cur);
    }

    TokenList Lexer::Start(const std::string& src)
    {
        TokenList tokens;
        Token current_token;
        current_token.line = 1;

        std:;size_t cur = 0;
        for (std::size_t i = 0; i < src.size(); ++i)
        {
            char c = src[i];
            switch (c)
            {
                case '$': // Immediate value
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = TokenType::Immediate;
                    break;
                }
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (current_token.type == TokenType::Comment ||
                        current_token.type == TokenType::StringLiteral ||
                        current_token.type == TokenType::Displacement ||
                        current_token.type == TokenType::Immediate ||
                        current_token.type == TokenType::Identifier)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    else
                    {
                        EndToken(current_token, tokens);
                        current_token.text.append(1, c);
                        current_token.type = TokenType::Displacement;
                    }
                    break;
                case '\\':
                {
                    if (current_token.type == TokenType::StringLiteral)
                    {
                        switch (src[i + 1])
                        {
                            case '0':
                                current_token.text.append(1, '\0');
                                break;
                            case 'n':
                                current_token.text.append(1, '\n');
                                break;
                            case 'r':
                                current_token.text.append(1, '\r');
                                break;
                            case 't':
                                current_token.text.append(1, '\t');
                                break;
                            case '"':
                                current_token.text.append(1, '\"');
                                break;
                        }
                        i++;
                    }
                    break;
                }
                case '\"': // String
                {
                    if (current_token.type == TokenType::Comment)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    else if (current_token.type == TokenType::StringLiteral)
                    {
                        EndToken(current_token, tokens);
                    }
                    else
                    {
                        EndToken(current_token, tokens);
                        current_token.type = TokenType::StringLiteral;
                    }
                    break;
                }
                case '\n':
                case '\r':
                {
                    EndToken(current_token, tokens);
                    cur = 0;
                    current_token.cur = 0;
                    current_token.line++;
                    current_token.type = TokenType::Whitespace;
                    break;
                }
                case '\t':
                case ' ':
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    break;
                }
                case ';':
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = TokenType::Comment;
                    break;
                }
                case ':':
                case ',':
                case '@':
                case '%':
                case '/':
                case '*':
                case '-':
                case '+':
                case ']':
                case '[':
                case '(':
                case ')':
                case '.':
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    else if ((src[i] == '+' || src[i] == '-') && (src[i + 1] >= '0' && src[i + 1] <= '9'))
                    {
                        EndToken(current_token, tokens);
                        current_token.type = TokenType::Displacement;
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = TokenType::Operator;
                    current_token.text.append(1, c);
                    EndToken(current_token, tokens);
                    break;
                }
                default:
                {
                    if (current_token.type == TokenType::Whitespace)
                    {
                        EndToken(current_token, tokens);
                        if (tokens.empty() || tokens.back().line < current_token.line)
                        {
                            current_token.type = TokenType::Instruction;
                            current_token.cur = cur;
                            current_token.text.append(1, c);
                        }
                        else
                        {
                            current_token.type = TokenType::Identifier;
                            current_token.cur = cur;
                            current_token.text.append(1, c);
                        }
                    }
                    else
                    {
                        current_token.text.append(1, c);
                        current_token.cur = cur + 1;
                    }
                    break;
                }
            }
            cur++;
        }

        EndToken(current_token, tokens);
        return tokens;
    }

    void Lexer::EndToken(Token& t, TokenList& tokens)
    {
        if (t.type != TokenType::Whitespace && t.type != TokenType::Comment)
        {
            switch (t.type)
            {
                case TokenType::Immediate:
                {
                    t.data = (std::int32_t)std::stoul(t.text);
                    break;
                }
                default:
                    break;
            }
            tokens.push_back(t);
        }
        t.type = TokenType::Whitespace;
        t.data = 0;
        t.text.erase();
    }
}
