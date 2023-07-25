#include "Compiler.h"
#include "Lexer.h"

#ifdef __clang__
#define DISABLE_ENUM_WARNING_BEGIN _Pragma("clang diagnostic push") \
                                   _Pragma("clang diagnostic ignored \"-Wswitch\"")
#define DISABLE_ENUM_WARNING_END   _Pragma("clang diagnostic pop")
#else
#define DISABLE_ENUM_WARNING_BEGIN
#define DISABLE_ENUM_WARNING_END
#endif

DISABLE_ENUM_WARNING_BEGIN

namespace rlang::rmc {
    std::unordered_map<std::string, ConstInfo> Compiler::m_ConstantNameTable;
    std::unordered_map<std::string, std::pair<std::size_t, std::unordered_map<std::string, std::size_t>>> Compiler::m_LabelAddressMap;
    std::vector<alvm::Instruction> Compiler::m_CompiledCode;
    std::vector<alvm::Instruction> Compiler::m_InstEpilogue;
    std::size_t Compiler::m_StackSize = 0;
    std::size_t Compiler::m_ConstantCount = 0;

    void Compiler::Cleanup()
    {
        m_CompiledCode.clear();
        m_InstEpilogue.clear();
        m_ConstantNameTable.clear();
        m_LabelAddressMap.clear();
    }

    void Compiler::Preproccess(const TokenList& tokens)
    {
        // Pre proccess the source code.
        std::size_t inst_count = 0;
        std::string last_label_definition;
        for (auto i = 0; i < tokens.size(); ++i)
        {
            if (i > 0 && tokens[i].line > tokens[i - 1].line)
                inst_count++;

            switch (tokens[i].type)
            {
                case TokenType::Operator:
                    if (tokens[i].text == "@" && tokens[i + 1].type == TokenType::Identifier)
                    {
                        // Label definition.
                        if (tokens[i + 2].type == TokenType::Operator && tokens[i + 2].text == ":")
                        {
                            m_LabelAddressMap[tokens[i + 1].text] = { inst_count, { } };
                            last_label_definition = tokens[i + 1].text;
                        }
                    }
                    else if (tokens[i].text == "." && tokens[i + 1].type == TokenType::Identifier)
                    {
                        // Local label definition.
                        if (tokens[i + 2].type == TokenType::Operator && tokens[i + 2].text == ":")
                        {
                            m_LabelAddressMap[last_label_definition].second[tokens[i + 1].text] = inst_count;
                        }
                    }
                    break;
            }
        }
    }

    std::vector<alvm::Instruction> Compiler::Compile(const TokenList& tokens)
    {
        Cleanup();
        Preproccess(tokens);

        std::string current_label;
        std::int8_t operand_count = 0;
        std::int8_t bit_size = 32;
        bool ptr = false;
        alvm::Instruction current_instruction;
        for (auto i = 0; i < tokens.size(); ++i)
        {
            switch (tokens[i].type)
            {
                case TokenType::Instruction:
                {
                    if (i > 0)
                    {
                        // Check for constant instruction (if that makes sense).
                        switch (current_instruction.opcode)
                        {
                            // We only have one of those and that's db.
                            // It is a real instruction but it's also a syntactical shugar intended to make things easier.
                            case alvm::OpCode::Db:
                                if (!m_CompiledCode.empty())
                                    m_CompiledCode.insert(m_CompiledCode.begin() + m_ConstantCount - 1, current_instruction);
                                else
                                    m_CompiledCode.push_back(current_instruction);
                                goto instruction_epilogue;
                                break;
                        }

                        // Quick and lazy fix.
                        if (current_instruction.opcode != alvm::OpCode::Nop) m_CompiledCode.push_back(current_instruction);
instruction_epilogue:
                        if (!m_InstEpilogue.empty())
                        {
                            m_CompiledCode.insert(m_CompiledCode.end(), m_InstEpilogue.begin(), m_InstEpilogue.end());
                            m_InstEpilogue.clear();
                        }
                    }
                    current_instruction = alvm::Instruction();
                    operand_count = 0;
                    bit_size = 32;

                    current_instruction.opcode = GetInst(tokens[i].text);

                    // Might be a syntactical insutrction.
                    if (current_instruction.opcode == alvm::OpCode::Nop)
                    {
                        if (tokens[i].text == "equ")
                        {
                            if (tokens[i + 1].type == TokenType::Identifier)
                            {
                                if (tokens[i + 2].type == TokenType::Number)
                                {
                                    m_ConstantNameTable[tokens[i + 1].text] =
                                    {
                                        .size = 4,
                                        .value = (std::uint32_t)std::stoul(tokens[i + 2].text),
                                        .type = TokenType::Number
                                    };
                                    i += 3;
                                }
                                else
                                {
                                    std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "):"
                                              << "Expected a number literal after  '" << tokens[i + 1].text << "'.\n";
                                    std::exit(-1);
                                }
                            }
                            else
                            {
                                std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "):"
                                          << "Expected an identifier after instruction.\n";
                                std::exit(-1);
                            }
                        }
                        else
                        {
                            std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): Unknown Instruction '"
                                      << tokens[i].text << "'.\n";
                            std::exit(-1);
                        }
                    }
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
                                current_instruction.reg1 = alvm::Register{ .type = reg, .ptr = ptr, .size = bit_size };
                            else
                                current_instruction.reg2 = alvm::Register{ .type = reg, .ptr = ptr, .size = bit_size };
                            i++;
                        }
                        else
                        {
                            std::cerr << "Compile Error: '" << tokens[i + 1].text << "' undefined identifier.\n";
                            std::exit(-1);
                        }
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
                        bit_size = 32;
                        ptr = false;
                    }
                    else if (tokens[i].text == "+")
                    {
                        if (ptr)
                        {
                            if (operand_count <= 0)
                            {
                                m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = current_instruction.reg1.type });
                                m_CompiledCode.push_back(alvm::Instruction
                                {
                                    .opcode = alvm::OpCode::Add,
                                    .imm32 = (std::uint32_t)std::stoul(tokens[i + 1].text),
                                    .reg1 = current_instruction.reg1.type,
                                });
                                m_InstEpilogue.push_back(alvm::Instruction { .opcode = alvm::OpCode::Pop, .reg1 = current_instruction.reg1.type });
                            }
                            else
                            {

                                m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = current_instruction.reg2.type });
                                m_CompiledCode.push_back(alvm::Instruction
                                {
                                    .opcode = alvm::OpCode::Add,
                                    .imm32 = (std::uint32_t)std::stoul(tokens[i + 1].text),
                                    .reg1 = current_instruction.reg2.type,
                                });
                                m_InstEpilogue.push_back(alvm::Instruction { .opcode = alvm::OpCode::Pop, .reg1 = current_instruction.reg2.type });
                            }
                            i++;
                        }
                        else
                        {
                            std::cerr << "Bad operand @ line (" << tokens[i].line << ", " << tokens[i].cur << ")." << std::endl;
                            std::exit(-1);
                        }
                    }
                    else if (tokens[i].text == "-")
                    {
                        if (ptr)
                        {
                            if (operand_count <= 0)
                            {
                                m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = current_instruction.reg1.type });
                                m_CompiledCode.push_back(alvm::Instruction
                                {
                                    .opcode = alvm::OpCode::Sub,
                                    .imm32 = (std::uint32_t)std::stoul(tokens[i + 1].text),
                                    .reg1 = current_instruction.reg1.type,
                                });
                                m_InstEpilogue.push_back(alvm::Instruction { .opcode = alvm::OpCode::Pop, .reg1 = current_instruction.reg1.type });
                            }
                            else
                            {

                                m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = current_instruction.reg2.type });
                                m_CompiledCode.push_back(alvm::Instruction
                                {
                                    .opcode = alvm::OpCode::Sub,
                                    .imm32 = (std::uint32_t)std::stoul(tokens[i + 1].text),
                                    .reg1 = current_instruction.reg2.type,
                                });
                                m_InstEpilogue.push_back(alvm::Instruction { .opcode = alvm::OpCode::Pop, .reg1 = current_instruction.reg2.type });
                            }
                            i++;
                        }
                        else
                        {
                            std::cerr << "Bad operand @ line (" << tokens[i].line << ", " << tokens[i].cur << ")." << std::endl;
                            std::exit(-1);
                        }
                    }
                    else if (tokens[i].text == "*")
                    {
                        std::cerr << "Multiplication is not supported.\n";
                        std::exit(-1);
                    }
                    else if (tokens[i].text == "/")
                    {
                        std::cerr << "Divison is not supported.\n";
                        std::exit(-1);
                    }
                    else if (tokens[i].text == "@")
                    {
                        // Check for a label call
                        if (tokens[i + 1].type == TokenType::Identifier &&
                            tokens[i + 2].text != ":")
                        {
                            current_instruction.imm32 = (std::uint32_t)m_LabelAddressMap[tokens[i + 1].text].first;
                        }
                        else
                        {
                            current_label = tokens[i + 1].text;
                        }

                    }
                    else if (tokens[i].text == ".")
                    {
                        if (tokens[i + 1].type == TokenType::Identifier &&
                            tokens[i + 2].type != TokenType::Operator && tokens[i + 2].text != ":")
                        {
                            current_instruction.imm32 = (std::uint32_t)m_LabelAddressMap[current_label].second[tokens[i + 1].text];
                        }
                        else if (m_LabelAddressMap[current_label].second.find(tokens[i + 1].text) != m_LabelAddressMap[current_label].second.end())
                        {
                            std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): "
                                      << "Unconsistent local label redefinition of '" << tokens[i + 1].text << "'."
                                      << std::endl;
                            std::exit(-1);
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
                    current_instruction.imm32 = (std::uint32_t)std::stoul(tokens[i].text);
                    break;
                }
                case TokenType::Identifier:
                {
                    // Check for keywords
                    if (tokens[i].text == "byte")
                    {
                        bit_size = 8;
                    }
                    else if (tokens[i].text == "word")
                    {
                        bit_size = 16;
                    }
                    else if (tokens[i].text == "dword")
                    {
                        bit_size = 32;
                    }
                    else if (auto it = m_ConstantNameTable.find(tokens[i].text); it != m_ConstantNameTable.end())
                    {
                        // I don't know what I wrote, this is so bug prone and I can feel it.
                        // I don't want to debug.
                        switch (current_instruction.opcode)
                        {
                            case alvm::OpCode::PrintStr:
                                if (operand_count == 0)
                                {
                                    if (it->second.type == TokenType::StringLiteral)
                                    {
                                        if (ptr)
                                        {
                                            m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = {alvm::RegType::R0}});
                                            m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Mov, .imm32 = alvm::STACK_SIZE - 1, .reg1 = {alvm::RegType::R0}});
                                            m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Sub, .imm32 = it->second.addr, .reg1 = { alvm::RegType::R0 } });
                                            current_instruction.reg1 = {alvm::RegType::R0, true};
                                            m_InstEpilogue.push_back(alvm::Instruction {.opcode = alvm::OpCode::Pop, .reg1 = {alvm::RegType::R0}});
                                        }
                                        else
                                        {
                                            m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = {alvm::RegType::R0}});
                                            m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Mov, .imm32 = alvm::STACK_SIZE - 1, .reg1 = {alvm::RegType::R0}});
                                            m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Sub, .imm32 =it->second.addr, .reg1 = { alvm::RegType::R0 }});
                                            current_instruction.reg1 = {alvm::RegType::R0};
                                            m_InstEpilogue.push_back(alvm::Instruction {.opcode = alvm::OpCode::Pop, .reg1 = {alvm::RegType::R0}});
                                        }
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
                            case alvm::OpCode::Jump:
                            case alvm::OpCode::PrintInt:
                            case alvm::OpCode::Push:
                                if (operand_count == 0)
                                {
                                    if (it->second.type == TokenType::Number)
                                    {
                                        if (ptr)
                                        {
                                            m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = {alvm::RegType::R0}});
                                            m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Mov, .imm32 = it->second.value, .reg1 = {alvm::RegType::R0}});
                                            current_instruction.reg1 = { alvm::RegType::R0, true };
                                            m_InstEpilogue.push_back(alvm::Instruction { .opcode = alvm::OpCode::Pop, .reg1 = {alvm::RegType::R0}});
                                        }
                                        else
                                        {
                                            current_instruction.imm32 = it->second.value;
                                        }
                                    }
                                    else
                                    {
                                        std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): Constant '"
                                                  << tokens[i].text << "' is not a number\n'"
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
                                if (operand_count == 0)
                                {
                                    if (ptr)
                                    {
                                        m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = { alvm::RegType::R0 } });
                                        m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Mov, .imm32 = it->second.value, .reg1 = { alvm::RegType::R0 } });
                                        current_instruction.reg1 = { alvm::RegType::R0, true };
                                        m_InstEpilogue.push_back(alvm::Instruction { .opcode = alvm::OpCode::Pop, .reg1 = { alvm::RegType::R0 } });
                                    }
                                    else
                                    {
                                        std::cerr << "Compile Error @ line (" << tokens[i].line << ", " << tokens[i].cur << "): First operand of '"
                                                  << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode] << "' doesn't take an immediate value.\n'"
                                                  << alvm::Instruction::InstructionStr[(std::size_t)current_instruction.opcode]
                                                  << "' Instruction Operand Encoding:\n"
                                                  << "\top1: [r/m] op2: [reg]\n"
                                                  << "\top1: [reg] op2: [r/m]\n"
                                                  << "\top1: [r/m] op2: [imm8/16/32]\n";
                                        std::exit(-1);
                                    }
                                }
                                else
                                {
                                    if (ptr)
                                    {
                                        m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Push, .reg1 = { alvm::RegType::R0 } });
                                        m_CompiledCode.push_back(alvm::Instruction { .opcode = alvm::OpCode::Mov, .imm32 = it->second.value, .reg1 = { alvm::RegType::R0 } });
                                        current_instruction.reg2 = { alvm::RegType::R0, true };
                                        m_InstEpilogue.push_back(alvm::Instruction { .opcode = alvm::OpCode::Pop, .reg1 = { alvm::RegType::R0 } });
                                    }
                                    else
                                    {
                                        current_instruction.imm32 = it->second.value;
                                    }
                                }
                                break;
                        }
                    }

                    switch (current_instruction.opcode)
                    {
                        case alvm::OpCode::Db:
                        {
                            switch (tokens[i + 1].type)
                            {
                                case TokenType::StringLiteral:
                                {
                                    std::vector<std::int8_t> str_bytes = utils::string::ToBytes(tokens[i + 1].text);
                                    str_bytes.push_back(0); // Null term
                                    current_instruction.bytes.reserve(current_instruction.bytes.size() + str_bytes.size());
                                    current_instruction.bytes.insert(
                                        current_instruction.bytes.end(), 
                                        str_bytes.begin(), 
                                        str_bytes.end()
                                    );
                                    m_ConstantNameTable[tokens[i].text] = { m_StackSize, str_bytes.size(), 0, TokenType::StringLiteral };
                                    m_StackSize += str_bytes.size();
                                    m_ConstantCount++;
                                    i++;
                                    break;
                                }
                                case TokenType::Whitespace:
                                case TokenType::Comment:
                                case TokenType::Identifier:
                                default:
                                    break;
                            }
                            break;
                        }
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
        return m_CompiledCode;
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

/*
namespace amcc
{
    ByteCode Compiler::Compile(const TokenList& tokens)
    {
        Instruction* inst = new Instruction();
        ByteCode m_CompiledCode;
        map<string, std::size_t> labelAddresses;
        int i = 0;
        for (auto& l : tokens)
        {
            if (l.type == INST) i++;
            else if (l.type == LABEL_DEFINITION) labelAddresses[l.text] = i;
        }
        for (auto& t : tokens)
        {
            switch (t.type)
            {
                case INST:
                {
                    if (inst->opcode == NOP)
                    {            
                        inst->opcode = static_cast<OpCode>(t.data);
                        break;
                    }   
                    m_CompiledCode.push_back(*inst); 
					delete inst;
					inst = new Instruction();            
                    inst->opcode = static_cast<OpCode>(t.data);
                    break;
                }
                case NUM:
                {
                    inst->p3 = t.data;
                    break;
                }
                case REG:
                {
                    if (inst->reg1 == NUL) inst->reg1 = static_cast<Regs>(t.data);
                    else inst->reg2 = static_cast<Regs>(t.data);
                    break;
                }
                case LABEL_CALL:
                {
                    inst->p3 = labelAddresses[t.text];
                    break;
                }
            }
        }
        m_CompiledCode.push_back(*inst); 
		delete inst;

        // for (auto& e : m_CompiledCode)
        // {
            // cout << "INST: " << opcodeType[e.opcode] << " reg1: " << regType[e.reg1] << " reg2: " << regType[e.reg2] << " p3: " << e.p3 << NL;
        // }
        return m_CompiledCode;
    }
}
*/

DISABLE_ENUM_WARNING_END
