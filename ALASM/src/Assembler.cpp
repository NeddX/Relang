#include "Assembler.h"
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

#define ASSEMBLE_ERROR(token, msg)                                       \
    std::cerr << "Compile Error @ line (" << token.line << ", " << token.cur << "): " \
    << msg << "\n";                                                     \
    return AssemblerStatus::AssembleError

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
    std::unordered_map<std::string, DataInfo> Assembler::m_SymbolTable;
    std::unordered_map<std::string, std::pair<std::size_t, std::unordered_map<std::string, std::size_t>>> Assembler::m_LabelAddressMap;
    std::vector<alvm::Instruction> Assembler::m_AssembledCode;
    std::vector<alvm::Instruction> Assembler::m_InstEpilogue;
    std::vector<std::uint8_t> Assembler::m_DataSection;
    std::size_t Assembler::m_BssSize = 0;
    std::string Assembler::m_CurrentSection = "";

    void Assembler::Cleanup()
    {
        m_AssembledCode.clear();
        m_InstEpilogue.clear();
        m_SymbolTable.clear();
        m_LabelAddressMap.clear();
        m_DataSection.clear();
        m_CurrentSection = "";
        m_BssSize = 0;
    }

    void Assembler::LabelProcessor(const TokenList& tokens)
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

    AssemblerStatus Assembler::WriteToBinary(const std::string& path)
    {
        std::ofstream fs(path);
        if (fs.is_open())
        {
            fs.seekp(0, fs.beg);

            std::uint8_t indic = alvm::DATA_SECTION_INDIC;
            std::size_t data_section_size = m_DataSection.size() * sizeof(std::uint8_t);
            std::size_t code_section_size = m_AssembledCode.size() * sizeof(alvm::Instruction);

            fs.write((const char*)&indic, sizeof(std::uint8_t));
            fs.write((const char*)&data_section_size, sizeof(std::size_t));
            fs.write((const char*)m_DataSection.data(), data_section_size);

            indic = alvm::BSS_SECTION_INDIC;

            fs.write((const char*)&indic, sizeof(std::uint8_t));
            fs.write((const char*)&m_BssSize, sizeof(std::size_t));

            indic = alvm::CODE_SECTION_INDIC;

            fs.write((const char*)&indic, sizeof(std::uint8_t));
            fs.write((const char*)&code_section_size, sizeof(std::size_t));
            fs.write((const char*)m_AssembledCode.data(), code_section_size);

            fs.close();
        }
        else
        {
            return AssemblerStatus::WriteError;
        }
        return AssemblerStatus::Ok;
    }

    AssemblerResult Assembler::Assemble(const AssemblerOptions& opt)
    {
        AssemblerResult res;
        res.status = CodeGen(opt.tokens);
        if (res.status == AssemblerStatus::Ok)
        {
            res.assembledCode = m_AssembledCode;
            res.dataSection = m_DataSection;
            switch (opt.type)
            {
                case OutputType::Lib:
                    break;
                case OutputType::DLib:
                    break;
                case OutputType::XBin:
                    res.status = WriteToBinary(opt.path);
                    break;
            }
        }
        return res;
    }

    AssemblerStatus Assembler::CodeGen(const TokenList& tokens)
    {
        Cleanup();
        LabelProcessor(tokens);

        std::string current_label;
        int operand_count = -1;
        alvm::RegType ptr = alvm::RegType::NUL;
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
                                    if (m_SymbolTable.find(tokens[i + 1].text) != m_SymbolTable.end())
                                    {
                                        ASSEMBLE_ERROR(tokens[i + 1],
                                                      "Attempted to redefine '" << tokens[i + 1].text << "'.");
                                    }

                                    DataInfo inf =
                                    {
                                        .addr = m_DataSection.size(),
                                        .size = 0,
                                        .value = (std::uint64_t)m_DataSection.size(),
                                        .initialized = true,
                                    };

                                    i += 2;
                                    bool run = true;
                                    if (inst == "byte")
                                    {
                                        inf.type = DataType::Byte;
                                        while (run)
                                        {
                                            switch (tokens[i].type)
                                            {
                                                case TokenType::StringLiteral:
                                                    m_DataSection.insert(m_DataSection.end(), tokens[i].text.begin(), tokens[i].text.end());
                                                    inf.size += tokens[i].text.size();
                                                        break;
                                                case TokenType::Immediate:
                                                case TokenType::Displacement:
                                                    m_DataSection.resize(m_DataSection.size() + 1);
                                                    *(std::uint8_t*)(m_DataSection.data() + inf.addr + inf.size) = (std::uint8_t)std::stoull(tokens[i].text);
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
                                                        ASSEMBLE_ERROR(tokens[i], "Expected a comma (,) but got a " << tokens[i].text << " instead.");
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
                                                            if ((tokens[i].type != TokenType::Immediate &&
                                                                tokens[i].type != TokenType::Displacement) ||
                                                                (tokens[i + 2].type != TokenType::Immediate &&
                                                                tokens[i + 2].type != TokenType::Displacement))
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "fill function expects two number literals.");
                                                            }

                                                            if (tokens[i + 1].type != TokenType::Operator &&
                                                                tokens[i + 1].text != ",")
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "Expecyed a ','.");
                                                            }

                                                            if (tokens[i + 3].type != TokenType::Operator &&
                                                                tokens[i + 3].text != ")")
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "Expected a ')'.");
                                                            }

                                                            std::uint8_t value = (std::uint8_t)std::stoull(tokens[i].text);
                                                            std::size_t size = std::stoull(tokens[i + 2].text);
                                                            m_DataSection.resize(inf.addr + size);
                                                            for (auto x = 0; x < size; ++x)
                                                                *((std::uint8_t*)m_DataSection.data() + inf.addr + x) = value;
                                                            inf.size += size;
                                                            i += 3;
                                                        }
                                                        else
                                                        {
                                                            ASSEMBLE_ERROR(tokens[i], "Expected a '('");
                                                        }
                                                    }
                                                    else
                                                    {
                                                        run = false;
                                                        m_SymbolTable[tokens[inst_token_id + 1].text] = inf;
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
                                        while (run)
                                        {
                                            switch (tokens[i].type)
                                            {
                                                case TokenType::StringLiteral:
                                                    ASSEMBLE_ERROR(tokens[i], "Wide characters are not yet supported.");
                                                    break;
                                                case TokenType::Immediate:
                                                case TokenType::Displacement:
                                                    m_DataSection.resize(m_DataSection.size() + 2);
                                                    *(std::uint16_t*)(m_DataSection.data() + inf.addr + inf.size) = (std::uint16_t)std::stoull(tokens[i].text);
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
                                                        ASSEMBLE_ERROR(tokens[i], "Expected a comma (,) but got a " << tokens[i].text << " instead.");
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
                                                            if ((tokens[i].type != TokenType::Immediate &&
                                                                tokens[i].type != TokenType::Displacement) ||
                                                                (tokens[i + 2].type != TokenType::Immediate &&
                                                                tokens[i + 2].type != TokenType::Displacement))
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "fill function expects two number literals.");
                                                            }

                                                            if (tokens[i + 1].type != TokenType::Operator &&
                                                                tokens[i + 1].text != ",")
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "Expecyed a ','.");
                                                            }

                                                            if (tokens[i + 3].type != TokenType::Operator &&
                                                                tokens[i + 3].text != ")")
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "Expected a ')'.");
                                                            }

                                                            std::uint16_t value = (std::uint16_t)std::stoull(tokens[i].text);
                                                            std::size_t size = std::stoull(tokens[i + 1].text);
                                                            m_DataSection.resize(inf.addr + size * 2);
                                                            for (auto x = 0; x < size; ++x)
                                                            {
                                                                *((std::uint16_t*)(std::uintptr_t)(m_DataSection.data() + inf.addr + (x * 2)))
                                                                    = value;
                                                            }
                                                            inf.size += size * 2;
                                                            i += 3;
                                                        }
                                                        else
                                                        {
                                                            ASSEMBLE_ERROR(tokens[i], "Expected a '('");
                                                        }
                                                    }
                                                    else
                                                    {
                                                        run = false;
                                                        m_SymbolTable[tokens[inst_token_id + 1].text] = inf;
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
                                        while (run)
                                        {
                                            switch (tokens[i].type)
                                            {
                                                case TokenType::StringLiteral:
                                                    ASSEMBLE_ERROR(tokens[i], "Expected a NumberLiteral but got a StringLiteral instead.");
                                                    break;
                                                case TokenType::Immediate:
                                                case TokenType::Displacement:
                                                    m_DataSection.resize(m_DataSection.size() + 4);
                                                    *(std::uint32_t*)(m_DataSection.data() + inf.addr + inf.size) = (std::uint32_t)std::stoull(tokens[i].text);
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
                                                        ASSEMBLE_ERROR(tokens[i], "Expected a comma (,) but got a " << tokens[i].text << " instead.");
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
                                                            if ((tokens[i].type != TokenType::Immediate &&
                                                                tokens[i].type != TokenType::Displacement) ||
                                                                (tokens[i + 2].type != TokenType::Immediate &&
                                                                tokens[i + 2].type != TokenType::Displacement))
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "fill function expects two number literals.");
                                                            }

                                                            if (tokens[i + 1].type != TokenType::Operator &&
                                                                tokens[i + 1].text != ",")
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "Expecyed a ','.");
                                                            }

                                                            if (tokens[i + 3].type != TokenType::Operator &&
                                                                tokens[i + 3].text != ")")
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "Expected a ')'.");
                                                            }

                                                            std::uint32_t value = (std::uint32_t)std::stoull(tokens[i].text);
                                                            std::size_t size = std::stoull(tokens[i + 1].text);
                                                            m_DataSection.resize(inf.addr + size * 4);
                                                            for (auto x = 0; x < size; ++x)
                                                            {
                                                                *((std::uint32_t*)(std::uintptr_t)(m_DataSection.data() + inf.addr + (x * 4)))
                                                                    = value;
                                                            }
                                                            inf.size += size * 4;
                                                            i += 3;
                                                        }
                                                        else
                                                        {
                                                            ASSEMBLE_ERROR(tokens[i], "Expected a '('");
                                                        }
                                                    }
                                                    else
                                                    {
                                                        run = false;
                                                        m_SymbolTable[tokens[inst_token_id + 1].text] = inf;
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
                                        while (run)
                                        {
                                            switch (tokens[i].type)
                                            {
                                                case TokenType::StringLiteral:
                                                    m_DataSection.insert(m_DataSection.end(), tokens[i].text.begin(), tokens[i].text.end());
                                                    inf.size += tokens[i].text.size();
                                                    break;
                                                case TokenType::Immediate:
                                                case TokenType::Displacement:
                                                    m_DataSection.resize(m_DataSection.size() + 8);
                                                    *(std::uint64_t*)(m_DataSection.data() + inf.addr + inf.size) = std::stoull(tokens[i].text);
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
                                                        ASSEMBLE_ERROR(tokens[i], "Expected a comma (,) but got a " << tokens[i].text << " instead.");
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
                                                            if ((tokens[i].type != TokenType::Immediate &&
                                                                tokens[i].type != TokenType::Displacement) ||
                                                                (tokens[i + 2].type != TokenType::Immediate &&
                                                                tokens[i + 2].type != TokenType::Displacement))
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "fill function expects two number literals.");
                                                            }

                                                            if (tokens[i + 1].type != TokenType::Operator &&
                                                                tokens[i + 1].text != ",")
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "Expecyed a ','.");
                                                            }

                                                            if (tokens[i + 3].type != TokenType::Operator &&
                                                                tokens[i + 3].text != ")")
                                                            {
                                                                ASSEMBLE_ERROR(tokens[i], "Expected a ')'.");
                                                            }

                                                            std::uint64_t value = std::stoull(tokens[i].text);
                                                            std::size_t size = std::stoull(tokens[i + 1].text);
                                                            m_DataSection.resize(inf.addr + size * 8);
                                                            for (auto x = 0; x < size; ++x)
                                                            {
                                                                *((std::uint64_t*)(std::uintptr_t)(m_DataSection.data() + inf.addr + (x * 8)))
                                                                    = value;
                                                            }
                                                            inf.size += size * 8;
                                                            i += 3;
                                                        }
                                                        else
                                                        {
                                                            ASSEMBLE_ERROR(tokens[i], "Expected a '('");
                                                        }
                                                    }
                                                    else
                                                    {
                                                        run = false;
                                                        m_SymbolTable[tokens[inst_token_id + 1].text] = inf;
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
                                    ASSEMBLE_ERROR(tokens[i], "Expected an Identifier after '" << tokens[i].text << "'.");
                                }
                            }
                            else if (inst == "align")
                            {
                                if (tokens[i + 1].type == TokenType::Immediate ||
                                    tokens[i + 1].type == TokenType::Displacement)
                                {
                                    m_DataSection.resize(m_DataSection.size() + std::stoull(tokens[i + 1].text));
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i + 2], "Expected a number literal after '" << tokens[i + 1].text << "'.");
                                }
                            }
                            else if (inst == "const")
                            {
                                if (tokens[i + 1].type == TokenType::Identifier)
                                {
                                    if (tokens[i + 2].type == TokenType::Immediate ||
                                        tokens[i + 2].type == TokenType::Displacement)
                                    {
                                        m_SymbolTable[tokens[i + 1].text] =
                                        {
                                            .size = 4,
                                            .value = std::stoull(tokens[i + 2].text),
                                            .constant = true,
                                            .type = DataType::QWord
                                        };
                                    }
                                    else
                                    {
                                        ASSEMBLE_ERROR(tokens[i + 2], "Expected an Integer after '" << tokens[i + 1].text << "'.");
                                    }
                                }
                                else
                                {
                                   ASSEMBLE_ERROR(tokens[i], "Expected an Identifier after const declaration.");
                                }
                                i += 2;
                            }
                            break;
                        }
                        else if (m_CurrentSection == "bss")
                        {
                            // bssara
                            std::string inst = utils::string::ToLowerCopy(tokens[i].text);
                            inst_token_id = i;
                            if (inst == "byte" || inst == "word" || inst == "dword" || inst == "qword")
                            {
                                if (tokens[i + 1].type == TokenType::Identifier)
                                {
                                   if (m_SymbolTable.find(tokens[i + 1].text) != m_SymbolTable.end())
                                   {
                                        ASSEMBLE_ERROR(tokens[i + 1], "Attempted to redefine '" << tokens[i + 1].text << "'.");
                                   }

                                   DataInfo inf =
                                   {
                                        .addr = m_DataSection.size() + m_BssSize,
                                        .size = 0,
                                        .value = 0,
                                        .initialized = false,
                                   };

                                   i += 2;
                                   if (tokens[i].type != TokenType::Displacement &&
                                       tokens[i].type != TokenType::Immediate)
                                   {
                                        ASSEMBLE_ERROR(tokens[i], "Expected a number.");
                                   }

                                   if (inst == "byte")
                                   {
                                        inf.type = DataType::Byte;
                                   }
                                   else if (inst == "word")
                                   {
                                        inf.type = DataType::Word;
                                   }
                                   else if (inst == "dword")
                                   {
                                        inf.type = DataType::DWord;
                                   }
                                   else if (inst == "qword")
                                   {
                                        inf.type = DataType::QWord;
                                   }
                                   inf.size = SizeOfDataTypeB(inf.type) * std::stoull(tokens[i].text);
                                   m_BssSize += inf.size;
                                   m_SymbolTable[tokens[inst_token_id + 1].text] = inf;
                                   break;
                                }
                                else
                                {
                                   ASSEMBLE_ERROR(tokens[i + 1], "Expected an identifier.");
                                }
                            }
                        }
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
                                case alvm::OpCode::Pushar:
                                case alvm::OpCode::Popar:
                                case alvm::OpCode::DumpFlags:
                                    if (operand_count > -1)
                                    {
                                        ASSEMBLE_ERROR(tokens[inst_token_id], "Instruction doesn't accept any operands.");
                                    }
                                    break;
                                // Instructions that accept a single operand.
                                case alvm::OpCode::Call:
                                case alvm::OpCode::Jump:
                                case alvm::OpCode::Jc:
                                case alvm::OpCode::Jcn:
                                case alvm::OpCode::Jue:
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
                                case alvm::OpCode::June:
                                case alvm::OpCode::Enter:
                                case alvm::OpCode::PInt:
                                case alvm::OpCode::PStr:
                                case alvm::OpCode::PChr:
                                case alvm::OpCode::Push:
                                case alvm::OpCode::Pop:
                                case alvm::OpCode::Mul:
                                case alvm::OpCode::Div:
                                case alvm::OpCode::Neg:
                                case alvm::OpCode::System:
                                case alvm::OpCode::GetChar:
                                case alvm::OpCode::Inc:
                                case alvm::OpCode::Dec:
                                case alvm::OpCode::Malloc:
                                case alvm::OpCode::Free:
                                case alvm::OpCode::SConio:
                                    // Instructions that accept both no operands or a single operand.
                                    switch (current_instruction.opcode)
                                    {
                                        case alvm::OpCode::Pop:
                                            goto ok_instruction_case;
                                            break;
                                    }

                                    if (operand_count > 0)
                                    {
                                        ASSEMBLE_ERROR(tokens[inst_token_id], "Instruction is unary.");
                                    }
                                    else if (operand_count == -1)
                                    {
                                        ASSEMBLE_ERROR(tokens[inst_token_id], "Instruction accepts a single operand but no operands were passed. Refer to its encoding for correct usage.");
                                    }
                                    break;
                                // Instructions that acceot three operands.
                                // None...
                                // Instructions that accept two operands.
                                default:
                                    // Instructions that accept both two operands or a single operand.
                                    switch (current_instruction.opcode)
                                    {
                                        case alvm::OpCode::Printf:
                                            goto ok_instruction_case;
                                            break;
                                    }

                                    if (operand_count <= 0 || operand_count > 1)
                                    {
                                        ASSEMBLE_ERROR(tokens[inst_token_id], "Invalid number of operands passed to Instruction.\nRefer to its encoding for correct usage.");
                                    }
                                    break;
                            }
ok_instruction_case:
                            m_AssembledCode.push_back(current_instruction);
                        }
                        if (!m_InstEpilogue.empty())
                        {
                            m_AssembledCode.insert(m_AssembledCode.end(), m_InstEpilogue.begin(), m_InstEpilogue.end());
                            m_InstEpilogue.clear();
                        }
                    }
                    inst_token_id = i;
                    current_instruction = alvm::Instruction {  };
                    operand_count = -1;
                    ptr = alvm::RegType::NUL;

                    current_instruction.opcode = GetInst(tokens[i].text);
                    current_instruction.size = 64;
                    if (current_instruction.opcode == alvm::OpCode::Nop && utils::string::ToLowerCopy(tokens[i].text) != "nop")
                    {
                        current_instruction.opcode = GetInst(tokens[i].text.substr(0, tokens[i].text.length() - 1));
                        if (current_instruction.opcode == alvm::OpCode::Nop && utils::string::ToLowerCopy(tokens[i].text) != "nop")
                        {
                            ASSEMBLE_ERROR(tokens[i], "Unknown Instruction " << tokens[i].text << ".");
                        }

                        switch (tokens[i].text[tokens[i].text.length() - 1])
                        {
                            case 'B':
                            case 'b':
                                current_instruction.size = 8;
                                break;
                            case 'W':
                            case 'w':
                                current_instruction.size = 16;
                                break;
                            case 'D':
                            case 'd':
                                current_instruction.size = 32;
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                }
                case TokenType::Displacement:
                {
                    /*
                    if (tokens[i + 1].type != TokenType::Operator && tokens[i + 1].text != "(")
                    {
                        ASSEMBLE_ERROR(tokens[i], "Bad displacement.");
                    }
                    */
                    if (ptr != alvm::RegType::NUL)
                    {
                        // Scaler...
                    }
                    else
                    {
                        // Displacer...
                        ptr = alvm::RegType::PTR;
                        current_instruction.displacement = (std::int32_t)std::stoull(tokens[i].text);
                    }
                    break;
                }
                case TokenType::Operator:
                {
                    if (tokens[i].text == "%")
                    {
                        alvm::RegType reg = GetReg(tokens[i + 1].text);
                        if (reg != alvm::RegType::NUL)
                        {
                            if (operand_count <= 0)
                            {
                                current_instruction.sreg = reg;
                                if (ptr != alvm::RegType::NUL)
                                    current_instruction.sreg = (alvm::RegType)(current_instruction.sreg | ptr);
                            }
                            else if (operand_count > 0)
                            {
                                current_instruction.dreg = reg;
                                if (ptr != alvm::RegType::NUL)
                                    current_instruction.dreg = (alvm::RegType)(current_instruction.dreg | ptr);
                            }
                            i++;
                        }
                        else
                        {
                            ASSEMBLE_ERROR(tokens[i + 1], "Undefined Identifier '" << tokens[i + 1].text << "'.");
                        }
                        if (operand_count <= -1) operand_count++;
                    }
                    else if (tokens[i].text == "+")
                    {
                        if (tokens[i - 1].type == TokenType::Identifier)
                        {
                            if (tokens[i + 1].type == TokenType::Immediate ||
                                tokens[i + 1].type == TokenType::Displacement)
                            {
                                auto it = m_SymbolTable.find(tokens[i - 1].text);
                                if (it != m_SymbolTable.end())
                                {
                                    current_instruction.displacement += (std::int32_t)std::stoull(tokens[i + 1].text);
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i - 1], "Unknown Identifier " << tokens[i - 1].text << ".");
                                }
                            }
                            else
                            {
                                ASSEMBLE_ERROR(tokens[i], "Expected a Number at the right hand side of the + operator.");
                            }
                        }
                        else
                        {
                            ASSEMBLE_ERROR(tokens[i], "Expected an Identifier at the left hand side of the + operator.");
                        }
                    }
                    else if (tokens[i].text == "-")
                    {
                        if (tokens[i - 1].type == TokenType::Identifier)
                        {
                            if (tokens[i + 1].type == TokenType::Immediate ||
                                tokens[i + 1].type == TokenType::Displacement)
                            {
                                auto it = m_SymbolTable.find(tokens[i - 1].text);
                                if (it != m_SymbolTable.end())
                                {
                                    current_instruction.displacement -= (std::int32_t)std::stoull(tokens[i + 1].text);
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i - 1], "Unknown Identifier " << tokens[i - 1].text << ".");
                                }
                            }
                            else
                            {
                                ASSEMBLE_ERROR(tokens[i], "Expected a Number at the right hand side of the + operator.");
                            }
                        }
                        else
                        {
                            ASSEMBLE_ERROR(tokens[i], "Expected an Identifier at the left hand side of the + operator.");
                        }
                    }
                    else if (tokens[i].text == "(")
                    {
                        if (operand_count <= -1) operand_count++;

                        i++;
                        ptr = alvm::RegType::PTR;
                        auto base_reg = GetReg(tokens[++i].text);
                        if (base_reg == alvm::RegType::NUL)
                        {
                            auto it = m_SymbolTable.find(tokens[--i].text);
                            if (it == m_SymbolTable.end())
                            {
                                ASSEMBLE_ERROR(tokens[i], "Expected a register or a symbol.");
                            }
                            i--;
                            break;
                        }

                        base_reg = (alvm::RegType)(base_reg | alvm::RegType::PTR);
                        if (operand_count <= 0)
                        {
                            current_instruction.sreg = base_reg;
                        }
                        else
                        {
                            current_instruction.dreg = base_reg;
                        }

                        int sub_op_counter = 1;
                        while (tokens[++i].text == "," && tokens[i].type == TokenType::Operator)
                        {
                            if (sub_op_counter == 1)
                            {
                                // Source register disp(base, source, scaler)
                                i++;
                                auto src_reg = GetReg(tokens[++i].text);
                                if (src_reg != alvm::RegType::NUL)
                                {
                                    current_instruction.src_reg = src_reg;
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i], "Expected a register.");
                                }
                            }
                            else if (sub_op_counter == 2)
                            {
                                // Scaler disp(base, source, scaler)
                                // TODO: CONTINUE
                            }
                            sub_op_counter++;

                        }
                        if (tokens[i].text != ")" && tokens[i++].type != TokenType::Operator)
                        {
                            ASSEMBLE_ERROR(tokens[i], "Expected a ).");
                        }
                    }
                    else if (tokens[i].text == ")")
                    {
                        ptr = alvm::RegType::NUL;
                    }
                    else if (tokens[i].text == ",")
                    {
                        operand_count++;
                        ptr = alvm::RegType::NUL;

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
                                    case alvm::OpCode::Jue:
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
                                    case alvm::OpCode::June:
                                        break;
                                    default:
                                        ASSEMBLE_ERROR(tokens[i], "Instruction doesn't accept a label as an operand.");
                                        break;
                                }
                                current_instruction.imm64 = (std::uint64_t)m_LabelAddressMap[tokens[i + 1].text].first;
                            }
                            else
                            {
                                std::cout << "Compiler Error @ line (" << tokens[i + 1].line << ", " << tokens[i + 1].cur << "): "
                                          << "Attempted to reference an undefined label '" << tokens[i + 1].text << "'.\n";
                                return AssemblerStatus::AssembleError;
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
                            if (tokens[i + 2].text == "data")
                            {
                                if (m_CurrentSection == "bss" || m_CurrentSection == "code")
                                {
                                    ASSEMBLE_ERROR(tokens[i + 2], "data section must be defined before " << m_CurrentSection << " section.");
                                }
                            }
                            else if (tokens[i + 2].text == "bss")
                            {
                                if (m_CurrentSection == "code")
                                {
                                    ASSEMBLE_ERROR(tokens[i + 2], "bss section must be defined before code section.");
                                }
                            }
                            else if (tokens[i + 2].text != "code")
                            {
                                ASSEMBLE_ERROR(tokens[i + 2], "Unknown section " << tokens[i + 2].text << ".");
                            }
                            m_CurrentSection = tokens[i + 2].text;
                            i += 3;
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
                                        ASSEMBLE_ERROR(tokens[i], "Local label " << tokens[i + 1].text << " in parent label " << current_label << " is undefined.");
                                    }
                                    else
                                    {
                                        ASSEMBLE_ERROR(tokens[i], "Attempted to reference a local label in an unexistent parent label.");
                                    }
                                }
                            }
                            else if (tokens[i + 2].type == TokenType::Operator || tokens[i + 2].text == ":")
                            {
                                // It's a local label definition, ignore it.
                                i += 2;
                                break;
                            }
                            else
                            {
                                ASSEMBLE_ERROR(tokens[i], "Whatever you're trying to do bud.");
                            }
                        }
                    }
                    else if (tokens[i + 1].text != ":" && tokens[i - 1].type != TokenType::Identifier && tokens[i - 2].type != TokenType::Operator && tokens[i - 2].text != "@")
                    {
                        std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): "
                                  << "Invalid operator '" << tokens[i].text << "' in instruction operand context."
                                  << std::endl;
                        return AssemblerStatus::AssembleError;
                    }
                    break;
                }
                case TokenType::Immediate:
                {
                    if (operand_count <= -1) operand_count++;

                    switch (current_instruction.opcode)
                    {
                        case alvm::OpCode::GetChar:
                            ASSEMBLE_ERROR(tokens[inst_token_id], "Instruction doesn't accept a number literal as an operand."
                                          << "\nRefer to its encoding for correct usage.");
                            break;
                        case alvm::OpCode::Load:
                            if (operand_count == 0)
                            {
                                ASSEMBLE_ERROR(tokens[inst_token_id], "Instruction doesn't accept a number literal as its source operand."
                                              << "\nRefer to its encoding for correct usage.");
                            }
                            break;
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
                            if (operand_count > 0)
                            {
                                ASSEMBLE_ERROR(tokens[inst_token_id], "Instruction doesn't accept a number literal as its destination operand."
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

                    if (tokens[i].text == "typeof")
                    {
                        if (tokens[i + 1].type == TokenType::Operator && tokens[i + 1].text == "(")
                        {
                            if (tokens[++i + 1].type != TokenType::Identifier)
                            {
                                ASSEMBLE_ERROR(tokens[i + 1], "typeof operator takes an identifier as an argument.");
                            }

                            auto it = m_SymbolTable.find(tokens[++i].text);
                            if (it == m_SymbolTable.end())
                            {
                                ASSEMBLE_ERROR(tokens[i], "'" << tokens[i].text << "' doesn't exist in the current context.");
                            }

                            current_instruction.imm64 = SizeOfDataTypeB(it->second.type);
                        }
                    }
                    else if (tokens[i].text == "sizeof")
                    {
                        if (tokens[i + 1].type == TokenType::Operator && tokens[i + 1].text == "(")
                        {
                            if (tokens[++i + 1].type != TokenType::Identifier)
                            {
                                ASSEMBLE_ERROR(tokens[i + 1], "sizeof operator takes an identifier as an argument.");
                            }

                            auto it = m_SymbolTable.find(tokens[++i].text);
                            if (it == m_SymbolTable.end())
                            {
                                ASSEMBLE_ERROR(tokens[i], "'" << tokens[i].text << "' doesn't exist in the current context.");
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
                                ASSEMBLE_ERROR(tokens[i + 1], "lengthof operator takes an identifier as an argument.");
                            }

                            auto it = m_SymbolTable.find(tokens[++i].text);
                            if (it == m_SymbolTable.end())
                            {
                                ASSEMBLE_ERROR(tokens[i], "'" << tokens[i].text << "' doesn't exist in the current context.");
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
                                ASSEMBLE_ERROR(tokens[i + 1], "offsetof operator takes an identifier as an argument.");
                            }

                            auto it = m_SymbolTable.find(tokens[++i].text);
                            if (it == m_SymbolTable.end())
                            {
                                ASSEMBLE_ERROR(tokens[i], tokens[i].text << " doesn't exist in the current context.");
                            }

                            current_instruction.imm64 = it->second.addr;
                        }
                    }
                    else if (auto it = m_SymbolTable.find(tokens[i].text); it != m_SymbolTable.end())
                    {
                        switch (current_instruction.opcode)
                        {
                            // Instructions that accept registers in their both operands
                            default:
                                if (it->second.constant)
                                {
                                    current_instruction.imm64 = it->second.value;
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i], "Instruction doesn't accept memory operands.");
                                }
                                break;
                            case alvm::OpCode::System:
                            case alvm::OpCode::PStr:
                                if (operand_count == 0)
                                {
                                    if (it->second.type == DataType::Byte)
                                    {
                                        current_instruction.sreg = (alvm::RegType)(alvm::RegType::DS | 0x80);
                                        current_instruction.displacement = (std::int64_t)it->second.addr;
                                    }
                                    else
                                    {
                                        std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): Variable "
                                                  << tokens[i].text << " is not a string literal.\n"
                                                  << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode]
                                                  << " Instruction Operand Encoding:\n"
                                                  << "\top1: [r, m, imm8/16/32] op2: [N/A]\n";
                                        return AssemblerStatus::AssembleError;
                                    }
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i], "Instruction is unary.");
                                }
                                break;
                            case alvm::OpCode::Mov:
                            case alvm::OpCode::Cmp:
                            case alvm::OpCode::Sub:
                            case alvm::OpCode::Add:
                                if (it->second.type != DataType::Undefined)
                                {
                                    if (!it->second.constant)
                                    {
                                        ASSEMBLE_ERROR(tokens[i], "Instruction doesn't accept memory operands.");
                                    }
                                    else if (operand_count > 0)
                                    {
                                        ASSEMBLE_ERROR(tokens[i], "Instruction doesn't accept an immediate value as its destination operand.");
                                    }
                                    current_instruction.imm64 = it->second.value;
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                }
                                break;
                            case alvm::OpCode::Load:
                            case alvm::OpCode::Lea:
                                if (it->second.type != DataType::Undefined)
                                {
                                    if (it->second.constant)
                                    {
                                        ASSEMBLE_ERROR(tokens[i], "Instruction doesn't accept an immediate value as an operand.");
                                    }
                                    else
                                    {
                                        if (operand_count == 0)
                                        {
                                            current_instruction.sreg = (alvm::RegType)(alvm::RegType::DS | 0x80);
                                            current_instruction.displacement = (std::int64_t)it->second.addr;
                                        }
                                        else
                                        {
                                            ASSEMBLE_ERROR(tokens[i], "Instruction doesn't accept a memory operand as its destination.");
                                        }
                                    }
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                }
                                break;
                            case alvm::OpCode::Store:
                                if (it->second.type != DataType::Undefined)
                                {
                                    if (it->second.constant)
                                    {
                                        if (operand_count >= 0)
                                        {
                                            ASSEMBLE_ERROR(tokens[i], "Instruction doesn't accept an immediate value as its destination operand.");
                                        }
                                        current_instruction.imm64 = it->second.value;
                                    }
                                    else
                                    {
                                        if (operand_count >= 0)
                                        {
                                            current_instruction.dreg = (alvm::RegType)(alvm::RegType::DS | 0x80);
                                            current_instruction.displacement = (std::int64_t)it->second.addr;
                                        }
                                        else
                                        {
                                            ASSEMBLE_ERROR(tokens[i], "Instruction doesn't accept a memory operand as its source.");
                                        }
                                    }
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
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
                                            ASSEMBLE_ERROR(tokens[i], "Instruction expects an immediate value.");
                                        }
                                    }
                                    else
                                    {
                                        ASSEMBLE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                    }
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i], "Instruction " << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode] << " is unary.");
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
                                            current_instruction.sreg = (alvm::RegType)(alvm::RegType::DS | 0x80);
                                            current_instruction.displacement = (std::int64_t)it->second.addr;
                                        }
                                    }
                                    else
                                    {
                                        ASSEMBLE_ERROR(tokens[i], "Instruction expects one of the following types: BYTE, WORD, DWORD, QWORD");
                                    }
                                }
                                else
                                {
                                    ASSEMBLE_ERROR(tokens[i], "Instruction " << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode] << " is unary.");
                                }
                                break;
                        }
                    }
                    else
                    {
                        ASSEMBLE_ERROR(tokens[i], "Undefined Identifier '" << tokens[i].text << "'.");
                    }
                    break;
                }
            }
        }

        m_AssembledCode.push_back(current_instruction);
        if (!m_InstEpilogue.empty())
        {
            m_AssembledCode.insert(m_AssembledCode.end(), m_InstEpilogue.begin(), m_InstEpilogue.end());
            m_InstEpilogue.clear();
        }

        m_AssembledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::End });
        return AssemblerStatus::Ok;
    }

    alvm::OpCode Assembler::GetInst(std::string inst)
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

    alvm::RegType Assembler::GetReg(std::string reg)
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
            : alvm::RegType::NUL;
    }
}

DISABLE_ENUM_WARNING_END
