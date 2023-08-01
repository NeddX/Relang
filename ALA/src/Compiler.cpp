#include "Compiler.h"
#include "Lexer.h"
#include "Utils.h"

#ifdef __clang__
#define DISABLE_ENUM_WARNING_BEGIN _Pragma("clang diagnostic push") \
                                   _Pragma("clang diagnostic ignored \"-Wswitch\"")
#define DISABLE_ENUM_WARNING_END   _Pragma("clang diagnostic pop")
#else
#define DISABLE_ENUM_WARNING_BEGIN
#define DISABLE_ENUM_WARNING_END
#endif

DISABLE_ENUM_WARNING_BEGIN

#define COMPILE_ERROR(token, msg)                                       \
    std::cerr << "Compile Error @ line (" << token.line << ", " << token.cur << "): " \
    << msg << "\n";                                                     \
    return { .status = CompilerStatus::Error }

static constexpr inline std::uint8_t SizeOfDataTypeB(rlang::rmc::DataType type)
{
    switch (type)
    {
        case rlang::rmc::Byte:
            return 1;
        case rlang::rmc::Word:
            return 2;
        case rlang::rmc::DWord:
            return 4;
        case rlang::rmc::QWord:
            return 8;
        default:
            return 0;
    }
}

static constexpr inline std::uint8_t SizeOfDataType(rlang::rmc::DataType type)
{
    switch (type)
    {
        case rlang::rmc::Byte:
            return 8;
        case rlang::rmc::Word:
            return 16;
        case rlang::rmc::DWord:
            return 32;
        case rlang::rmc::QWord:
            return 64;
        default:
            return 0;
    }
}

namespace rlang::rmc {
    std::unordered_map<std::string, DataInfo> Compiler::m_DataNameTable;
    std::unordered_map<std::string, std::pair<std::size_t, std::unordered_map<std::string, std::size_t>>> Compiler::m_LabelAddressMap;
    std::vector<alvm::Instruction> Compiler::m_CompiledCode;
    std::vector<alvm::Instruction> Compiler::m_InstEpilogue;
    std::vector<std::uint8_t> Compiler::m_DataSection;
    std::size_t Compiler::m_StackSize = 0;
    std::size_t Compiler::m_DataCount = 0;
    std::string Compiler::m_CurrentSection = "";

    void Compiler::Cleanup()
    {
        m_CompiledCode.clear();
        m_InstEpilogue.clear();
        m_DataNameTable.clear();
        m_LabelAddressMap.clear();
        m_DataSection.clear();
        m_CurrentSection = "";
    }

    void Compiler::Preproccess(const TokenList& tokens)
    {
        // Pre proccess the source code.
        std::size_t inst_count = 0;
        std::string last_label_definition;
        bool data_section = false;
        for (auto i = 0; i < tokens.size(); ++i)
        {
            if (!data_section)
            {
                if (tokens[i].type == TokenType::Operator &&
                    tokens[i].text == "." &&
                    tokens[i + 1].type == TokenType::Identifier &&
                    tokens[i + 1].text == "section" &&
                    tokens[i + 2].type == TokenType::Identifier &&
                    tokens[i + 2].text == "code" &&
                    tokens[i + 3].type == TokenType::Operator &&
                    tokens[i + 3].text == ":")
                {
                    data_section = true;
                    i += 4;
                }
                continue;
            }

            if (tokens[i].type == TokenType::Instruction)
                inst_count++;

            switch (tokens[i].type)
            {
                case TokenType::Operator:
                    if (tokens[i].text == "@" && tokens[i + 1].type == TokenType::Identifier)
                    {
                        // Label definition.
                        if (tokens[i + 2].type == TokenType::Operator && tokens[i + 2].text == ":")
                        {
                            m_LabelAddressMap[tokens[i + 1].text] = { inst_count + 1, { } };
                            last_label_definition = tokens[i + 1].text;
                            i += 2;
                        }
                    }
                    else if (tokens[i].text == "." && tokens[i + 1].type == TokenType::Identifier)
                    {
                        // Local label definition.
                        if (tokens[i + 2].type == TokenType::Operator && tokens[i + 2].text == ":")
                        {
                            if (auto it = m_LabelAddressMap[last_label_definition].second.find(tokens[i + 1].text);
                                it != m_LabelAddressMap[last_label_definition].second.end())
                            {
                                std::cerr << "Preproccess Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): "
                                          << "Unconsistent local label redefinition of '" << tokens[i + 1].text << "'."
                                          << std::endl;
                                std::exit(-2);
                            }
                            m_LabelAddressMap[last_label_definition].second[tokens[i + 1].text] = inst_count + 1;
                            i += 2;
                        }
                    }
                    break;
            }
        }
    }

    CompilerResult Compiler::Compile(const TokenList& tokens)
    {
        Cleanup();
        Preproccess(tokens);

        std::string current_label;
        int operand_count = -1;
        //int bit_size = 64;
        bool ptr = false;
        std::size_t inst_token_id = 0;
        alvm::Instruction current_instruction;
        for (auto i = 0; i < tokens.size(); ++i)
        {
            switch (tokens[i].type)
            {
                case TokenType::Instruction:
                {
                    if (!m_CurrentSection.empty())
                    {
                        if (m_CurrentSection == "data")
                        {
                            std::string inst = utils::string::ToLowerCopy(tokens[i].text);
                            inst_token_id = i;
                            if (inst == "byte" || inst == "word" || inst == "dword" || inst == "qword")
                            {
                                if (tokens[i + 1].type == TokenType::Identifier)
                                {
                                    if (m_DataNameTable.find(tokens[i + 1].text) != m_DataNameTable.end())
                                    {
                                        COMPILE_ERROR(tokens[i + 1],
                                                      "Attempted to redefine '" << tokens[i + 1].text << "'.");
                                    }

                                    // tokens[i + 2] should be the first element
                                    DataInfo inf =
                                    {
                                        .addr = m_DataSection.size(),
                                        .size = 0,
                                        .value = (std::uint64_t)m_DataSection.size(),
                                    };

                                    i += 2;
                                    bool run = true;
                                    if (inst == "byte")
                                    {
                                        inf.type = DataType::Byte;

                                        if (tokens[i].type != TokenType::Number &&
                                            tokens[i].type != TokenType::StringLiteral)
                                        {
                                            COMPILE_ERROR(tokens[i],
                                                          "Expected a NumberLiteral or a StringLiteral but got "
                                                          << Token::TokenStr[(std::size_t)tokens[i].type] << " instead.");
                                        }

                                        while (run)
                                        {
                                            switch (tokens[i].type)
                                            {
                                                case TokenType::StringLiteral:
                                                    m_DataSection.insert(m_DataSection.end(), tokens[i].text.begin(), tokens[i].text.end());
                                                    inf.size += tokens[i].text.size();
                                                        break;
                                                case TokenType::Number:
                                                    m_DataSection.resize(m_DataSection.size() + 1);
                                                    *(std::uint8_t*)(m_DataSection.data() + inf.addr + inf.size) = (std::uint8_t)std::stoul(tokens[i].text);
                                                    inf.size++;
                                                    break;
                                                case TokenType::Operator:
                                                    if (tokens[i].text == ",")
                                                    {
                                                        i++;
                                                        continue;
                                                    }
                                                    else if (tokens[i].text == ".")
                                                    {
                                                        goto byte_def_case;
                                                    }
                                                    else
                                                    {
                                                        // compile error: bad operator
                                                        COMPILE_ERROR(tokens[i], "Expected a comma (,) but got a " << tokens[i].text << " instead.");
                                                    }
                                                    break;
                                                default:
byte_def_case:
                                                    if (tokens[i].type == TokenType::Identifier &&
                                                        tokens[i++].text == "fill")
                                                    {
                                                        if (tokens[i].type == TokenType::Operator &&
                                                            tokens[i++].text == "(")
                                                        {
                                                            if (tokens[i].type == TokenType::Number)
                                                            {
                                                                if (tokens[++i].type != TokenType::Operator &&
                                                                    tokens[i].text != ")")
                                                                {
                                                                    COMPILE_ERROR(tokens[i], "Expected a ')'.");
                                                                }
                                                                std::size_t size = std::stoul(tokens[i - 1].text);
                                                                std::uint8_t data =
                                                                    *((std::uint8_t*)m_DataSection.data() + inf.size - 1);
                                                                m_DataSection.resize(inf.size + size);
                                                                for (auto x = 0; x < size; ++x)
                                                                    *((std::uint8_t*)m_DataSection.data() + inf.size + x) = data;
                                                                inf.size += size;
                                                            }
                                                            else
                                                            {
                                                                COMPILE_ERROR(tokens[i - 1], "fill function takes in a NumberLiteral as an argument.");
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        run = false;
                                                        m_DataNameTable[tokens[inst_token_id + 1].text] = inf;
                                                    }
                                                    break;
                                            }
                                            if (run) ++i;
                                            else --i;
                                        }
                                    }
                                    else if (inst == "word")
                                    {
                                        inf.type = DataType::Word;

                                        if (tokens[i].type != TokenType::Number &&
                                            tokens[i].type != TokenType::StringLiteral)
                                        {
                                            COMPILE_ERROR(tokens[i],
                                                          "Expected a NumberLiteral or a StringLiteral but got "
                                                          << Token::TokenStr[(std::size_t)tokens[i].type] << " instead.");
                                        }

                                        while (run)
                                        {
                                            switch (tokens[i].type)
                                            {
                                                case TokenType::StringLiteral:
                                                    COMPILE_ERROR(tokens[i], "Wide characters are not yet supported.");
                                                    break;
                                                case TokenType::Number:
                                                    m_DataSection.resize(m_DataSection.size() + 2);
                                                    *(std::uint16_t*)(m_DataSection.data() + inf.addr + inf.size) = (std::uint16_t)std::stoul(tokens[i].text);
                                                    inf.size += 2;
                                                    break;
                                                case TokenType::Operator:
                                                    if (tokens[i].text == ",")
                                                    {
                                                        i++;
                                                        continue;
                                                    }
                                                    else if (tokens[i].text == ".")
                                                    {
                                                        goto word_def_case;
                                                    }
                                                    else
                                                    {
                                                        // compile error: bad operator
                                                        COMPILE_ERROR(tokens[i], "Expected a comma (,) but got a " << tokens[i].text << " instead.");
                                                    }
                                                    break;
                                                default:
word_def_case:
                                                    if (tokens[i].type == TokenType::Identifier &&
                                                        tokens[i++].text == "fill")
                                                    {
                                                        if (tokens[i].type == TokenType::Operator &&
                                                            tokens[i++].text == "(")
                                                        {
                                                            if (tokens[i].type == TokenType::Number)
                                                            {
                                                                if (tokens[++i].type != TokenType::Operator &&
                                                                    tokens[i].text != ")")
                                                                {
                                                                    COMPILE_ERROR(tokens[i], "Expected a ')'.");
                                                                }
                                                                std::size_t size = std::stoul(tokens[i - 1].text);
                                                                std::uint16_t data =
                                                                    *((std::uint16_t*)(std::uintptr_t)(m_DataSection.data() + inf.size - 2));
                                                                m_DataSection.resize(inf.size + size * 2);
                                                                for (auto x = 0; x < size; ++x)
                                                                {
                                                                    *((std::uint16_t*)(std::uintptr_t)(m_DataSection.data() + inf.size + (x * 2)))
                                                                        = data;
                                                                }
                                                                inf.size += size * 2;                                                            }
                                                            else
                                                            {
                                                                COMPILE_ERROR(tokens[i - 1], "fill function takes in a NumberLiteral as an argument.");
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        run = false;
                                                        m_DataNameTable[tokens[inst_token_id + 1].text] = inf;
                                                    }
                                                    break;
                                            }
                                            if (run) ++i;
                                            else --i;
                                        }
                                    }
                                    else if (inst == "dword")
                                    {
                                        inf.type = DataType::DWord;

                                        if (tokens[i].type != TokenType::Number)
                                        {
                                            COMPILE_ERROR(tokens[i],
                                                          "Expected a NumberLiteral but got "
                                                          << Token::TokenStr[(std::size_t)tokens[i].type] << " instead.");
                                        }

                                        while (run)
                                        {
                                            switch (tokens[i].type)
                                            {
                                                case TokenType::StringLiteral:
                                                    COMPILE_ERROR(tokens[i], "Expected a NumberLiteral but got a StringLiteral instead.");
                                                    break;
                                                case TokenType::Number:
                                                    m_DataSection.resize(m_DataSection.size() + 4);
                                                    *(std::uint32_t*)(m_DataSection.data() + inf.addr + inf.size) = (std::uint32_t)std::stoul(tokens[i].text);
                                                    inf.size += 4;
                                                    break;
                                                case TokenType::Operator:
                                                    if (tokens[i].text == ",")
                                                    {
                                                        i++;
                                                        continue;
                                                    }
                                                    else if (tokens[i].text == ".")
                                                    {
                                                        goto dword_def_case;
                                                    }
                                                    else
                                                    {
                                                        // compile error: bad operator
                                                        COMPILE_ERROR(tokens[i], "Expected a comma (,) but got a " << tokens[i].text << " instead.");
                                                    }
                                                    break;
                                                default:
dword_def_case:
                                                    if (tokens[i].type == TokenType::Identifier &&
                                                        tokens[i++].text == "fill")
                                                    {
                                                        if (tokens[i].type == TokenType::Operator &&
                                                            tokens[i++].text == "(")
                                                        {
                                                            if (tokens[i].type == TokenType::Number)
                                                            {
                                                                if (tokens[++i].type != TokenType::Operator &&
                                                                    tokens[i].text != ")")
                                                                {
                                                                    COMPILE_ERROR(tokens[i], "Expected a ')'.");
                                                                }
                                                                std::size_t size = std::stoul(tokens[i - 1].text);
                                                                std::uint32_t data =
                                                                    *((std::uint32_t*)(std::uintptr_t)(m_DataSection.data() + inf.size - 4));
                                                                m_DataSection.resize(inf.size + size * 4);
                                                                for (auto x = 0; x < size; ++x)
                                                                {
                                                                    *((std::uint32_t*)(std::uintptr_t)(m_DataSection.data() + inf.size + (x * 4)))
                                                                        = data;
                                                                }
                                                                inf.size += size * 4;                                                            }
                                                            else
                                                            {
                                                                COMPILE_ERROR(tokens[i - 1], "fill function takes in a NumberLiteral as an argument.");
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        run = false;
                                                        m_DataNameTable[tokens[inst_token_id + 1].text] = inf;
                                                    }
                                                    break;
                                            }
                                            if (run) ++i;
                                            else --i;
                                        }
                                    }
                                    else if (inst == "qword")
                                    {
                                        inf.type = DataType::QWord;

                                        if (tokens[i].type != TokenType::Number)
                                        {
                                            COMPILE_ERROR(tokens[i],
                                                          "Expected a NumberLiteral but got "
                                                          << Token::TokenStr[(std::size_t)tokens[i].type] << " instead.");
                                        }

                                        while (run)
                                        {
                                            switch (tokens[i].type)
                                            {
                                                case TokenType::StringLiteral:
                                                    m_DataSection.insert(m_DataSection.end(), tokens[i].text.begin(), tokens[i].text.end());
                                                    inf.size += tokens[i].text.size();
                                                    break;
                                                case TokenType::Number:
                                                    m_DataSection.resize(m_DataSection.size() + 8);
                                                    *(std::uint64_t*)(m_DataSection.data() + inf.addr + inf.size) = std::stoul(tokens[i].text);
                                                    inf.size += 8;
                                                    break;
                                                case TokenType::Operator:
                                                    if (tokens[i].text == ",")
                                                    {
                                                        i++;
                                                        continue;
                                                    }
                                                    else if (tokens[i].text == ".")
                                                    {
                                                        goto qword_def_case;
                                                    }
                                                    else
                                                    {
                                                        // compile error: bad operator
                                                        COMPILE_ERROR(tokens[i], "Expected a comma (,) but got a " << tokens[i].text << " instead.");
                                                    }
                                                    break;
                                                default:
qword_def_case:
                                                    if (tokens[i].type == TokenType::Identifier &&
                                                        tokens[i++].text == "fill")
                                                    {
                                                        if (tokens[i].type == TokenType::Operator &&
                                                            tokens[i++].text == "(")
                                                        {
                                                            if (tokens[i].type == TokenType::Number)
                                                            {
                                                                if (tokens[++i].type != TokenType::Operator &&
                                                                    tokens[i].text != ")")
                                                                {
                                                                    COMPILE_ERROR(tokens[i], "Expected a ')'.");
                                                                }
                                                                std::size_t size = std::stoul(tokens[i - 1].text);
                                                                std::uint64_t data =
                                                                    *((std::uint64_t*)(std::uintptr_t)(m_DataSection.data() + inf.size - 8));
                                                                m_DataSection.resize(inf.size + size * 8);
                                                                for (auto x = 0; x < size; ++x)
                                                                {
                                                                    *((std::uint64_t*)(std::uintptr_t)(m_DataSection.data() + inf.size + (x * 8)))
                                                                        = data;
                                                                }
                                                                inf.size += size * 8;                                                            }
                                                            else
                                                            {
                                                                COMPILE_ERROR(tokens[i - 1], "fill function takes in a NumberLiteral as an argument.");
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        run = false;
                                                        m_DataNameTable[tokens[inst_token_id + 1].text] = inf;
                                                    }
                                                    break;
                                            }
                                            if (run) ++i;
                                            else --i;
                                        }
                                    }
                                }
                                else
                                {
                                    COMPILE_ERROR(tokens[i], "Expected an Identifier after '" << tokens[i].text << "'.");
                                }
                            }
                            else if (inst == "align")
                            {
                                if (tokens[i + 1].type == TokenType::Number)
                                {
                                    m_DataSection.resize(m_DataSection.size() + std::stoul(tokens[i + 1].text));
                                }
                                else
                                {
                                    COMPILE_ERROR(tokens[i + 2], "Expected a number literal after '" << tokens[i + 1].text << "'.");
                                }
                            }
                            else if (inst == "const")
                            {
                                if (tokens[i + 1].type == TokenType::Identifier)
                                {
                                    if (tokens[i + 2].type == TokenType::Number)
                                    {
                                        m_DataNameTable[tokens[i + 1].text] =
                                        {
                                            .size = 4,
                                            .value = std::stoul(tokens[i + 2].text),
                                            .constant = true,
                                            .type = DataType::QWord
                                        };
                                    }
                                    else
                                    {
                                        COMPILE_ERROR(tokens[i + 2], "Expected an Integer after '" << tokens[i + 1].text << "'.");
                                    }
                                }
                                else
                                {
                                   COMPILE_ERROR(tokens[i], "Expected an Identifier after const declaration.");
                                }
                                i += 2;
                            }
                            break;
                        }
                    }
                    else if (m_CurrentSection != "code")
                    {
                        COMPILE_ERROR(tokens[i], "No section defined.");
                    }

                    if (i > 0)
                    {
                        if (current_instruction.opcode != alvm::OpCode::Nop)
                        {
                            switch (current_instruction.opcode)
                            {
                                // Instructions that accept no operands.
                                case alvm::OpCode::Leave:
                                case alvm::OpCode::Return:
                                case alvm::OpCode::End:
                                case alvm::OpCode::Lrzf:
                                case alvm::OpCode::Srzf:
                                case alvm::OpCode::Nop:
                                case alvm::OpCode::DumpFlags:
                                    if (operand_count > -1)
                                    {
                                        COMPILE_ERROR(tokens[inst_token_id], "Instruction doesn't accept any operands.");
                                    }
                                    break;
                                // Instructions that accept a single operand.
                                case alvm::OpCode::Call:
                                case alvm::OpCode::Jump:
                                case alvm::OpCode::Jc:
                                case alvm::OpCode::Jcn:
                                case alvm::OpCode::Je:
                                case alvm::OpCode::Jl:
                                case alvm::OpCode::Jno:
                                case alvm::OpCode::Jns:
                                case alvm::OpCode::Jnz:
                                case alvm::OpCode::Jo:
                                case alvm::OpCode::Js:
                                case alvm::OpCode::Jug:
                                case alvm::OpCode::Juge:
                                case alvm::OpCode::Jul:
                                case alvm::OpCode::Jule:
                                case alvm::OpCode::Enter:
                                case alvm::OpCode::PInt:
                                case alvm::OpCode::PStr:
                                case alvm::OpCode::Push:
                                case alvm::OpCode::Pop:
                                case alvm::OpCode::Mul:
                                case alvm::OpCode::Div:
                                case alvm::OpCode::System:
                                    if (operand_count > 0)
                                    {
                                        COMPILE_ERROR(tokens[inst_token_id], "Instruction is unary.");
                                    }
                                    else if (operand_count == -1)
                                    {
                                        COMPILE_ERROR(tokens[inst_token_id], "Instruction accepts a single operand but no operands were passed. Refer to its encoding for correct usage.");
                                    }
                                    break;
                                // Instructions that acceot three operands.
                                // None...
                                // Instructions that accept two operands.
                                default:
                                    if (operand_count <= 0 || operand_count > 1)
                                    {
                                        COMPILE_ERROR(tokens[inst_token_id], "Invalid number of operands passed to Instruction, refer to its encoding for correct usage.");
                                    }
                                    break;
                            }
                            m_CompiledCode.push_back(current_instruction);
                        }
                        if (!m_InstEpilogue.empty())
                        {
                            m_CompiledCode.insert(m_CompiledCode.end(), m_InstEpilogue.begin(), m_InstEpilogue.end());
                            m_InstEpilogue.clear();
                        }
                    }
                    inst_token_id = i;
                    current_instruction = alvm::Instruction();
                    operand_count = -1;
                    //bit_size = current_instruction.size;

                    current_instruction.opcode = GetInst(tokens[i].text);
                    break;
                }
                case TokenType::Operator:
                {
                    if (tokens[i].text == "%")
                    {
                        alvm::RegType reg = GetReg(tokens[i + 1].text);
                        if (reg != alvm::RegType::Nul)
                        {
                            if (current_instruction.reg1.type == alvm::RegType::Nul)
                                current_instruction.reg1 = alvm::Register{ .type = reg, .ptr = ptr };
                            else
                                current_instruction.reg2 = alvm::Register{ .type = reg, .ptr = ptr };
                            i++;
                        }
                        else
                        {
                            COMPILE_ERROR(tokens[i + 1], "Undefined Identifier '" << tokens[i + 1].text << "'.");
                        }
                        if (operand_count <= -1) operand_count++;
                    }
                    else if (tokens[i].text == "[")
                    {
                        ptr = true;
                    }
                    else if (tokens[i].text == "]")
                    {
                        ptr = false;
                    }
                    else if (tokens[i].text == ",")
                    {
                        operand_count++;
                        ptr = false;
                    }
                    else if (tokens[i].text == "+")
                    {
                        if (tokens[i + 1].type != TokenType::Number)
                        {
                            COMPILE_ERROR(tokens[i + 1], "Expected a number literal after " << tokens[i].text << ".");
                        }
                        else if (ptr)
                        {
                            if (operand_count <= 0)
                            {
                                current_instruction.reg1.displacement = (std::int32_t)std::stoul(tokens[i + 1].text);
                            }
                            else
                            {
                                current_instruction.reg2.displacement = (std::int32_t)std::stoul(tokens[i + 1].text);
                            }
                            i++;
                        }
                        else
                        {
                            COMPILE_ERROR(tokens[i], "Single operation arithmetic expressions are only allowed inside Memory Access Brackets.");
                        }
                    }
                    else if (tokens[i].text == "-")
                    {
                        if (tokens[i + 1].type != TokenType::Number)
                        {
                            COMPILE_ERROR(tokens[i + 1], "Expected a number literal after " << tokens[i].text << ".");
                        }
                        else if (ptr)
                        {
                            if (operand_count == 0)
                            {
                                current_instruction.reg1.displacement = -(std::int32_t)std::stoul(tokens[i + 1].text);
                            }
                            else
                            {
                                current_instruction.reg2.displacement = -(std::int32_t)std::stoul(tokens[i + 1].text);
                            }
                            i++;
                        }
                        else
                        {
                            COMPILE_ERROR(tokens[i], "Single operqation arithmetic expressionsa re only allowed inside Memory Access Brackets.");
                        }
                    }
                    else if (tokens[i].text == "*")
                    {
                        COMPILE_ERROR(tokens[i], "Scaling not yet supported.");
                    }
                    else if (tokens[i].text == "@")
                    {
                        // Check for a label call
                        if (tokens[i + 1].type == TokenType::Identifier &&
                            tokens[i + 2].text != ":")
                        {
                            if (operand_count <= -1) operand_count++;

                            if (m_LabelAddressMap.find(tokens[i + 1].text) != m_LabelAddressMap.end())
                            {
                                switch (current_instruction.opcode)
                                {
                                    case alvm::OpCode::Call:
                                    case alvm::OpCode::Jump:
                                    case alvm::OpCode::Jc:
                                    case alvm::OpCode::Jcn:
                                    case alvm::OpCode::Je:
                                    case alvm::OpCode::Jl:
                                    case alvm::OpCode::Jno:
                                    case alvm::OpCode::Jns:
                                    case alvm::OpCode::Jnz:
                                    case alvm::OpCode::Jo:
                                    case alvm::OpCode::Js:
                                    case alvm::OpCode::Jug:
                                    case alvm::OpCode::Juge:
                                    case alvm::OpCode::Jul:
                                    case alvm::OpCode::Jule:
                                        break;
                                    default:
                                        COMPILE_ERROR(tokens[i], "Instruction doesn't accept a label as an operand.");
                                        break;
                                }
                                current_instruction.imm64 = (std::uint64_t)m_LabelAddressMap[tokens[i + 1].text].first;
                            }
                            else
                            {
                                std::cout << "Compiler Error @ line (" << tokens[i + 1].line << ", " << tokens[i + 1].cur << "): "
                                          << "Attempted to reference an undefined label '" << tokens[i + 1].text << "'.\n";
                                std::exit(-1);
                            }
                            i++;
                        }
                        else if (tokens[i + 2].type == TokenType::Operator && tokens[i + 2].text == ":")
                        {
                            // It's a label definition, ignore it.
                            current_label = tokens[i + 1].text;
                            i += 2;
                        }
                    }
                    else if (tokens[i].text == ".")
                    {
                        if (tokens[i + 1].type == TokenType::Identifier &&
                            tokens[i + 1].text == "section" &&
                            tokens[i + 2].type == TokenType::Identifier &&
                            tokens[i + 3].type == TokenType::Operator &&
                            tokens[i + 3].text == ":")
                        {

                            // Section definition
                            if (tokens[i + 2].text == "data" || tokens[i + 2].text == "code")
                            {
                                m_CurrentSection = tokens[i + 2].text;
                                i += 3;
                            }
                            else
                            {
                                std::cerr << "Compile Error @ line (" << tokens[i + 2].line << ", " << tokens[i + 2].cur
                                          << "): Unknown section '" << tokens[i + 2].text << "'.\n";
                                std::exit(-1);
                            }
                        }
                        else if (tokens[i + 1].type == TokenType::Identifier)
                        {
                            // Possible local label reference
                            if (tokens[i + 2].type != TokenType::Operator || tokens[i + 2].text != ":")
                            {
                                if  (m_LabelAddressMap.find(current_label) != m_LabelAddressMap.end() &&
                                     m_LabelAddressMap[current_label].second.find(tokens[i + 1].text) !=
                                     m_LabelAddressMap[current_label].second.end())
                                {
                                    operand_count++;
                                    current_instruction.imm64 = (std::uint64_t)m_LabelAddressMap[current_label].second[tokens[i + 1].text];
                                    i++;
                                    break;
                                }
                                else
                                {
                                    if (!current_label.empty())
                                    {
                                        COMPILE_ERROR(tokens[i], "Local label " << tokens[i + 1].text << " in parent label " << current_label << " is undefined.");
                                    }
                                    else
                                    {
                                        COMPILE_ERROR(tokens[i], "Attempted to reference a local label in an unexistent parent label.");
                                    }
                                }
                            }
                            else if (tokens[i + 2].type == TokenType::Operator || tokens[i + 2].text == ":")
                            {
                                i += 2;
                                // It's a local label definition, ignore it.
                                break;
                            }
                            else
                            {
                                COMPILE_ERROR(tokens[i], "Whatever you're trying to do bud.");
                            }
                        }
                    }
                    else if (tokens[i + 1].text != ":" && tokens[i - 1].type != TokenType::Identifier && tokens[i - 2].type != TokenType::Operator && tokens[i - 2].text != "@")
                    {
                        std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): "
                                  << "Invalid operator '" << tokens[i].text << "' in instruction operand context."
                                  << std::endl;
                        std::exit(-1);
                    }
                    break;
                }
                case TokenType::Number:
                {
                    if (operand_count <= -1) operand_count++;

                    switch (current_instruction.opcode)
                    {
                        case alvm::OpCode::Load:
                            if (operand_count > 0)
                            {
                                COMPILE_ERROR(tokens[inst_token_id], "Instruction doesn't take number literal as its second operand."
                                              << "\nRefer to its encoding for correct usage.");
                            }
                        case alvm::OpCode::Store:
                        case alvm::OpCode::Mov:
                        case alvm::OpCode::System:
                        case alvm::OpCode::Syscall:
                        case alvm::OpCode::Dec:
                        case alvm::OpCode::Inc:
                        case alvm::OpCode::Mul:
                        case alvm::OpCode::Div:
                        case alvm::OpCode::Add:
                        case alvm::OpCode::Sub:
                        case alvm::OpCode::PStr:
                        case alvm::OpCode::Cmp:
                        case alvm::OpCode::AND:
                        case alvm::OpCode::OR:
                        case alvm::OpCode::NOT:
                        case alvm::OpCode::XOR:
                            if (operand_count == 0)
                            {
                                COMPILE_ERROR(tokens[inst_token_id], "Instruction doesn't accept a number literal as its first operand."
                                              << "\nRefer to its encoding for correct usage.");
                            }
                            break;
                    }

                    current_instruction.imm64 = std::stoull(tokens[i].text);
                    break;
                }
                case TokenType::Identifier:
                {
                    if (operand_count <= -1) operand_count++;

                    // Check for keywords
                    if (tokens[i].text == "byte")
                    {
                        current_instruction.size = 8;
                    }
                    else if (tokens[i].text == "word")
                    {
                        current_instruction.size = 16;
                    }
                    else if (tokens[i].text == "dword")
                    {
                        current_instruction.size = 32;
                    }
                    else if (tokens[i].text == "qword")
                    {
                        current_instruction.size = 64;
                    }
                    else if (tokens[i].text == "sizeof")
                    {
                        if (tokens[i + 1].type == TokenType::Operator && tokens[i + 1].text == "(")
                        {
                            if (tokens[++i + 1].type != TokenType::Identifier)
                            {
                                COMPILE_ERROR(tokens[i + 1], "sizeof operator takes an identifier as an argument.");
                            }

                            auto it = m_DataNameTable.find(tokens[++i].text);
                            if (it == m_DataNameTable.end())
                            {
                                COMPILE_ERROR(tokens[i], "'" << tokens[i].text << "' doesn't exist in the current context.");
                            }

                            current_instruction.imm64 = it->second.size;
                        }
                    }
                    else if (tokens[i].text == "lengthof")
                    {
                        if (tokens[i + 1].type == TokenType::Operator && tokens[i + 1].text == "(")
                        {
                            if (tokens[++i + 1].type != TokenType::Identifier)
                            {
                                COMPILE_ERROR(tokens[i + 1], "lengthof operator takes an identifier as an argument.");
                            }

                            auto it = m_DataNameTable.find(tokens[++i].text);
                            if (it == m_DataNameTable.end())
                            {
                                COMPILE_ERROR(tokens[i], "'" << tokens[i].text << "' doesn't exist in the current context.");
                            }
                            current_instruction.imm64 = it->second.size / SizeOfDataTypeB(it->second.type);
                        }
                    }
                    else if (tokens[i].text == "offsetof")
                    {
                        if (tokens[i + 1].type == TokenType::Operator && tokens[i + 1].text == "(")
                        {
                            if (tokens[++i + 1].type != TokenType::Identifier)
                            {
                                COMPILE_ERROR(tokens[i + 1], "offsetof operator takes an identifier as an argument.");
                            }

                            auto it = m_DataNameTable.find(tokens[++i].text);
                            if (it == m_DataNameTable.end())
                            {
                                COMPILE_ERROR(tokens[i], "'" << tokens[i].text << "' doesn't exist in the current context.");
                            }

                            current_instruction.imm64 = it->second.addr;
                        }
                    }
                    else if (auto it = m_DataNameTable.find(tokens[i].text); it != m_DataNameTable.end())
                    {
                        switch (current_instruction.opcode)
                        {
                            case alvm::OpCode::System:
                            case alvm::OpCode::PStr:
                                if (operand_count == 0)
                                {
                                    if (it->second.type == DataType::Byte)
                                    {
                                        current_instruction.reg1 =
                                            { .type = alvm::RegType::DS, .ptr = true, .displacement = (std::int64_t)it->second.addr };
                                    }
                                    else
                                    {
                                        std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): Constant '"
                                                  << tokens[i].text << " is not a string literal.\n'"
                                                  << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode]
                                                  << "' Instruction Operand Encoding:\n"
                                                  << "\top1: [r, m, imm8/16/32] op2: [N/A]\n";
                                        std::exit(-1);
                                    }
                                }
                                else
                                {
                                    std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): Instruction '"
                                              << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode]
                                              << "' is a unary instruction.\n'"
                                              << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode]
                                              << "' Instruction Operand Encoding:\n"
                                              << "\top1: [r, m, imm8/16/32] op2: [N/A]\n";
                                    std::exit(-1);
                                }
                                break;
                            case alvm::OpCode::Mov:
                                if (it->second.type != DataType::Undefined)
                                {
                                    if (it->second.constant)
                                    {
                                        current_instruction.imm64 = it->second.value;
                                    }
                                    else
                                    {
                                        COMPILE_ERROR(tokens[i], "Instruction doesn't accept memory operands.\n"
                                                      << "MOV Instruction encdoing:\n"
                                                      << "\top1: [r] op2: [r, imm64]");
                                    }
                                }
                                else
                                {
                                    COMPILE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                }
                                break;
                            case alvm::OpCode::Load:
                                if (it->second.type != DataType::Undefined)
                                {
                                    if (it->second.constant)
                                    {
                                        COMPILE_ERROR(tokens[i], "Instruction doesn't accept an immediate value as an operand.\n"
                                                      << "LOAD Instruction encoding:\n"
                                                      << "\top1: [m] op2: [r]");
                                    }
                                    else
                                    {
                                        if (operand_count > 0)
                                        {
                                            current_instruction.reg2 = { .type = alvm::RegType::DS, .ptr = true, .displacement = (std::int64_t)it->second.addr };
                                            current_instruction.size = SizeOfDataType(it->second.type);
                                        }
                                        else
                                        {
                                            COMPILE_ERROR(tokens[i], "Instruction doesn't accept a memory as the first operand.\n"
                                                          << "LOAD Instruction encoding:\n"
                                                          << "\top1: [m] op2: [r]");
                                        }
                                    }
                                }
                                else
                                {
                                    COMPILE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                }
                                break;
                            case alvm::OpCode::Store:
                                if (it->second.type != DataType::Undefined)
                                {
                                    if (it->second.constant)
                                    {
                                        current_instruction.imm64 = it->second.value;
                                    }
                                    else
                                    {
                                        if (operand_count == 0)
                                        {
                                            current_instruction.reg1 = { .type = alvm::RegType::DS, .ptr = true, .displacement = (std::int64_t)it->second.addr };
                                            current_instruction.size = SizeOfDataType(it->second.type);
                                        }
                                        else
                                        {
                                            COMPILE_ERROR(tokens[i], "Instruction doesn't accept a memory as the second operand.\n"
                                                          << "STORE Instruction encoding:\n"
                                                          << "\top1: [m] op2: [r, imm64]");
                                        }
                                    }
                                }
                                else
                                {
                                    COMPILE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                }
                                break;
                            case alvm::OpCode::Enter:
                            case alvm::OpCode::Jump:
                            case alvm::OpCode::Push:
                                if (operand_count == 0)
                                {
                                    if (it->second.type != DataType::Undefined)
                                    {
                                        if (it->second.constant)
                                        {
                                            current_instruction.imm64 = it->second.value;
                                        }
                                        else
                                        {
                                            COMPILE_ERROR(tokens[i], "Instruction expects an immediate value.");
                                        }
                                    }
                                    else
                                    {
                                        COMPILE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                    }
                                }
                                else
                                {
                                    std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): Instruction '"
                                              << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode]
                                              << "' is a unary instruction.\n"
                                              << utils::string::ToUpperCopy(alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode])
                                              << " Instruction Operand Encoding:\n"
                                              << "\top1: [r, m, imm8/16/32] op2: [N/A]\n";
                                    std::exit(-1);
                                }
                                break;
                            case alvm::OpCode::PInt:
                                if (operand_count == 0)
                                {
                                    if (it->second.type != DataType::Undefined)
                                    {
                                        if (it->second.constant)
                                        {
                                            current_instruction.imm64 = it->second.value;
                                        }
                                        else
                                        {
                                            current_instruction.reg1 = { .type = alvm::RegType::DS, .ptr = true, .displacement = (std::int64_t)it->second.addr };
                                            current_instruction.size = SizeOfDataType(it->second.type);
                                        }
                                    }
                                    else
                                    {
                                        COMPILE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                    }
                                }
                                else
                                {
                                    COMPILE_ERROR(tokens[i], "Instruction "
                                                  << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode]
                                                  << " is a unary instruction.\n"
                                                  << utils::string::ToUpperCopy(alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode])
                                                  << " Instruction Operand Encoding:\n"
                                                  << "\top1: [r, m, imm8/16/32] op2: [N/A]\n");
                                }
                                break;
                        }
                    }
                    else
                    {
                        COMPILE_ERROR(tokens[i], "Undefined Identifier '" << tokens[i].text << "'.");
                    }
                    break;
                }
            }
        }

        m_CompiledCode.push_back(current_instruction);
        if (!m_InstEpilogue.empty())
        {
            m_CompiledCode.insert(m_CompiledCode.end(), m_InstEpilogue.begin(), m_InstEpilogue.end());
            m_InstEpilogue.clear();
        }

        m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::End });
        return { m_CompiledCode, m_DataSection, CompilerStatus::Ok };
    }

    alvm::OpCode Compiler::GetInst(std::string inst)
    {
        std::for_each(inst.begin(), inst.end(), [](char& c) { c = std::tolower(c); });
        auto it = std::find_if(alvm::Instruction::InstructionStr.begin(), alvm::Instruction::InstructionStr.end(),
            [&inst](std::string str)
            {
                std::for_each(str.begin(), str.end(), [](char& c) { c = std::tolower(c); });
                return str == inst;
            });

        return (it != alvm::Instruction::InstructionStr.end())
            ? (alvm::OpCode)std::distance(alvm::Instruction::InstructionStr.begin(), it)
            : alvm::OpCode::Nop;
    }

    alvm::RegType Compiler::GetReg(std::string reg)
    {
        std::for_each(reg.begin(), reg.end(), [](char& c) { c = std::tolower(c); });
        auto it = std::find_if(alvm::Register::RegisterStr.begin(), alvm::Register::RegisterStr.end(),
            [&reg](std::string str)
            {
                std::for_each(str.begin(), str.end(), [](char& c) { c = std::tolower(c); });
                return str == reg;
            });
        return 
            (it != alvm::Register::RegisterStr.end()) 
            ? (alvm::RegType)std::distance(alvm::Register::RegisterStr.begin(), it) 
            : alvm::RegType::Nul;
    }
}

DISABLE_ENUM_WARNING_END
