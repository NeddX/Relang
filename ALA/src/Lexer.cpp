#include "Lexer.h"

namespace rlang::rmc {
    void Token::Dump() const
    {
        std::printf("token { \"%s\" type: %s data: %d [%d, %d] }\n",
                    text.c_str(), GET_TOKEN_STR(type).c_str(), data, line, cur);
    }

    TokenList Lexer::Start(const std::string& src)
    {
        TokenList tokens;
        Token current_token;
        current_token.line = 1;

        int cur = 0;
        int i = 0;
        for (std::size_t i = 0; i < src.size(); ++i)
        {
            char c = src[i]; 
            switch (c)
            {
                case '#': // Number
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = TokenType::Number;
                    break;
                }
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
                /*
                case ',':
                {
                    if (current_token.type == TokenType::Comment)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    break;
                }*/
                /*case '%': // Register
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = TokenType::Register;
                    break;
                }*/
                /*case '@': // Label
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = TokenType::LabelCall;
                    break;
                }*/
                /*
                case ':':
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    if (current_token.type == TokenType::LabelCall)
                    {
                        current_token.type = TokenType::LabelDefinition;
                        EndToken(current_token, tokens);
                    }
                    break;
                }*/
                case '\n':
                case '\r':
                {
                    EndToken(current_token, tokens);
                    cur = 0;
                    current_token.line++;
                    current_token.type = TokenType::Whitespace;
                    break;
                }
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
                {
                    if (current_token.type == TokenType::Comment || current_token.type == TokenType::StringLiteral)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = TokenType::Operator;
                    current_token.text.append(1, c);
                    EndToken(current_token, tokens);
                    break;
                }
                case '\t':
                    break;
                default:
                {
                    if (current_token.type == TokenType::Whitespace)
                    {
                        EndToken(current_token, tokens);
                        if (cur == 0)
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
                }
            cur++;
            }
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
                case TokenType::Number:
                {
                    t.data = (std::int32_t)std::stoul(t.text);
                    break;
                }
            }
            tokens.push_back(t);
        }
        t.type = TokenType::Whitespace;
        t.data = 0;
        t.text.erase();
	}
}

/*
namespace amca
{
    TokenList Lexer::Lex(const string& src)
    {
        TokenList tokens;
        Token current_token;

        current_token.line = 1;

        int cur = 0;
        int i = 0;
        for (char c : src)
        {
            switch (c)
            {
                case '#': // Number
                {
                    if (current_token.type == COMMENT)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = NUM;
                    // We dont append the hashtag
                    break;
                }
                case ',':
                {
                    if (current_token.type == COMMENT)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    break;
                }
                case '%': // Register
                {
                    if (current_token.type == COMMENT)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = REG;
                    break;
                }
                case '@': // Label
                {
                    if (current_token.type == COMMENT)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = LABEL_CALL;
                    break;
                }
                case ':':
                {
                    if (current_token.type == COMMENT)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    if (current_token.type == LABEL_CALL)
                    {
                        current_token.type = LABEL_DEFINITION;
                        EndToken(current_token, tokens);
                    }
                    break;
                }
                case '\n':
                case '\r':
                {
                    EndToken(current_token, tokens);
                    cur = 0;
                    current_token.line++;
                    current_token.type = WHITESPACE;
                    break;
                }
                case ' ':
                {
                    if (current_token.type == COMMENT)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    break;
                }
                case ';':
                {
                    if (current_token.type == COMMENT)
                    {
                        current_token.text.append(1, c);
                        break;
                    }
                    EndToken(current_token, tokens);
                    current_token.type = COMMENT;
                    break;
                }
                default:
                {
                    if (current_token.type == WHITESPACE)
                    {
                        EndToken(current_token, tokens);
                        current_token.type = INST;
                        current_token.cur = cur;
                        current_token.text.append(1, c);
                    }
                    else
                    {
                        current_token.text.append(1, c);
                        current_token.cur = cur + 1;
                    }
                }
                cur++;
                i++;
            }
        }

        EndToken(current_token, tokens);
        return tokens;
    }

    void EndToken(Token& t, TokenList& tokens)
    {
        if (t.type != WHITESPACE)
        {
            switch (t.type)
            {
                case INST:
                {
                    t.data = GetInst(t.text);
                    break;
                }     
                case REG:
                {
                    t.data = GetReg(t.text);
                    break;
                }   
                case NUM:
                {
                    t.data = stoi(t.text);
                    break;
                }
            }
            tokens.push_back(t);
        }
        t.type = WHITESPACE;
        t.text.erase();
    }

    OpCode GetInst(string& inst)
    {
        transform(inst.begin(), inst.end(), inst.begin(), ::toupper);
        if (inst == "END") return END;
        if (inst == "PUSH") return PUSH;
        if (inst == "POP") return POP;
        if (inst == "ADD") return ADD;
        if (inst == "SUB") return SUB;
        if (inst == "MUL") return MUL;
        if (inst == "DIV") return DIV;
        if (inst == "PINT") return PRINT_INT;
        if (inst == "PRINT_STR") return PRINT_STR;
        if (inst == "CILT" ) return COMP_INT_LT;
        if (inst == "CIET") return COMP_INT_ET;
        if (inst == "CIGT" ) return COMP_INT_GT;
        if (inst == "CIET") return COMP_INT_NE;
        if (inst == "MOV") return MOV;
        if (inst == "MOVS") return MOVS;
        if (inst == "JMP") return JMP;
        if (inst == "CJMP") return CJMP;
        if (inst == "JRP") return JRP;
        if (inst == "CJRP") return CJRP;
        if (inst == "JRN") return JRN;
        if (inst == "JNE") return JNE;
        if (inst == "CALL") return CALL;
        if (inst == "RETURN") return RETURN;
        return NOP;
    }   

    Regs GetReg(string& reg)
    {
        transform(reg.begin(), reg.end(), reg.begin(), ::toupper);
        if (reg == "RAX") return RAX;
        if (reg == "RBX") return RBX;
        if (reg == "RCX") return RCX;
        if (reg == "RDX") return RDX;
        if (reg == "RBP") return RBP;
        if (reg == "RSP") return RSP;
        if (reg == "RSI") return RSI;
        if (reg == "RDI") return RDI;
        if (reg == "EAX") return EAX;
        if (reg == "EBX") return EBX;
        if (reg == "ECX") return ECX;
        if (reg == "EDX") return EDX;
        if (reg == "EBP") return EBP;
        if (reg == "ESP") return ESP;
        if (reg == "ESI") return ESI;
        if (reg == "EDI") return EDI;
        if (reg == "R8") return R8;
        if (reg == "R9") return R9;
        if (reg == "R10") return R10;
        if (reg == "R11") return R11;
        if (reg == "R12") return R12;
        if (reg == "R13") return R13;
        if (reg == "R14") return R14;
        if (reg == "R15") return R15;
        if (reg == "E8") return E8;
        if (reg == "E9") return E9;
        if (reg == "E10") return E10;
        if (reg == "E11") return E11;
        if (reg == "E12") return E12;
        if (reg == "E13") return E13;
        if (reg == "E14") return E14;
        if (reg == "E15") return E15;
        if (reg == "ZF") return ZF;
        if (reg == "FNR") return FNR;
        if (reg == "RFR") return RFR;
        if (reg == "EFR") return EFR;
        if (reg == "NUL") return NUL;
        return NUL;
    } 
}*/
