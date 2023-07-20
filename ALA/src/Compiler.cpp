#include "Compiler.h"

namespace rlang::rmc {
    std::unordered_map<std::string, MemInfo> Compiler::m_DbNameTable;
    std::unordered_map<std::string, std::size_t> Compiler::m_LabelAddressMap;
    std::vector<alvm::Instruction> Compiler::m_CompiledCode;
    std::vector<alvm::Instruction> Compiler::m_InstEpilogue;

    void Compiler::Cleanup()
    {
        m_CompiledCode.clear();
        m_InstEpilogue.clear();
        m_DbNameTable.clear();
        m_LabelAddressMap.clear();
    }

    std::vector<alvm::Instruction> Compiler::Compile(const TokenList& tokens)
    {
        Cleanup();

        std::int32_t stack_size = 0;
        std::int8_t operand_count = 0;
        std::int8_t bit_size = 32;
        bool ptr = false;
        alvm::Instruction current_instruction;
        for (std::size_t i = 0; i < tokens.size(); ++i)
        {
            switch (tokens[i].type)
            {
                case TokenType::Instruction:
                {
                    if (i > 0)
                    {
                        m_CompiledCode.push_back(current_instruction);
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
                            std::cout << "Syntax Error: '" << tokens[i + 1].text << "' undefined identifier.\n";
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
                                m_CompiledCode.push_back({ .opcode = alvm::OpCode::Push, .reg1 = current_instruction.reg1.type });
                                m_CompiledCode.push_back(
                                {
                                    .opcode = alvm::OpCode::Add,
                                    .imm32 = (std::int32_t)std::stoul(tokens[i + 1].text),
                                    .reg1 = current_instruction.reg1.type,
                                });
                                m_InstEpilogue.push_back({ .opcode = alvm::OpCode::Pop, .reg1 = current_instruction.reg1.type });
                            }
                            else
                            {

                                m_CompiledCode.push_back({ .opcode = alvm::OpCode::Push, .reg1 = current_instruction.reg2.type });
                                m_CompiledCode.push_back(
                                {
                                    .opcode = alvm::OpCode::Add,
                                    .imm32 = (std::int32_t)std::stoul(tokens[i + 1].text),
                                    .reg1 = current_instruction.reg2.type,
                                });
                                m_InstEpilogue.push_back({ .opcode = alvm::OpCode::Pop, .reg1 = current_instruction.reg2.type });
                            }
                            i++;
                        }
                        else
                        {
                            std::cerr << "Bad operand @ line (" << tokens[i].line << ", " << tokens[i].cur << ")." << std::endl;
                        }
                    }
                    else if (tokens[i].text == "-")
                    {
                        if (ptr)
                        {
                            if (operand_count <= 0)
                            {
                                m_CompiledCode.push_back({ .opcode = alvm::OpCode::Push, .reg1 = current_instruction.reg1.type });
                                m_CompiledCode.push_back(
                                {
                                    .opcode = alvm::OpCode::Sub,
                                    .imm32 = (std::int32_t)std::stoul(tokens[i + 1].text),
                                    .reg1 = current_instruction.reg1.type,
                                });
                                m_InstEpilogue.push_back({ .opcode = alvm::OpCode::Pop, .reg1 = current_instruction.reg1.type });
                            }
                            else
                            {

                                m_CompiledCode.push_back({ .opcode = alvm::OpCode::Push, .reg1 = current_instruction.reg2.type });
                                m_CompiledCode.push_back(
                                {
                                    .opcode = alvm::OpCode::Sub,
                                    .imm32 = (std::int32_t)std::stoul(tokens[i + 1].text),
                                    .reg1 = current_instruction.reg2.type,
                                });
                                m_InstEpilogue.push_back({ .opcode = alvm::OpCode::Pop, .reg1 = current_instruction.reg2.type });
                            }
                            i++;
                        }
                        else
                        {
                            std::cerr << "Bad operand @ line (" << tokens[i].line << ", " << tokens[i].cur << ")." << std::endl;
                        }
                    }
                    else if (tokens[i].text == "*")
                    {
                        std::cerr << "Multiplication is not supported.\n";
                        {
                        if (ptr)
                            if (operand_count <= 0)
                            {
                                current_instruction.reg1.ptr_offset *= (std::int32_t)std::stoul(tokens[i + 1].text);
                          }
                            else
                            {
                                current_instruction.reg2.ptr_offset *= (std::int32_t)std::stoul(tokens[i + 1].text);
                            }
                            i++;
                        }
                    }
                    else if (tokens[i].text == "/")
                    {
                        std::cerr << "Divison is not supported.\n";
                        if (ptr)
                        {
                            if (operand_count <= 0)
                            {
                                current_instruction.reg1.ptr_offset /= (std::int32_t)std::stoul(tokens[i + 1].text);
                            }
                            else
                            {
                                current_instruction.reg2.ptr_offset /= (std::int32_t)std::stoul(tokens[i + 1].text);
                            }
                            i++;
                        }
                    }
                    else if (tokens[i].text == "@")
                    {
                        // Possible label definition.
                        if (tokens[i + 1].type == TokenType::Identifier)
                        {
                            if (tokens[i + 2].type == TokenType::Operator && tokens[i + 2].text == ":")
                            {
                                m_LabelAddressMap[tokens[i + 1].text] = m_CompiledCode.size();
                            }
                            else
                            {
                                // Possible label call?


                                /*std::cerr << "Syntax Error @ line: (" << tokens[i + 2].line << "," << tokens[i + 2].cur << "): "
                                          << "Expected an ':' after '" << tokens[i + 1].text << "'"
                                          << std::endl;*/
                            }
                        }
                        else
                        {
                            std::cerr << "Syntax Error @ line: (" << tokens[i + 1].line << "," << tokens[i + 1].cur << "): "
                                      << "Expected an identifier after '@'."
                                      << std::endl;
                        }
                    }
                    break;
                }
                case TokenType::Number:
                {
                    current_instruction.imm32 = (std::int32_t)std::stoul(tokens[i].text);
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
                                    m_DbNameTable[tokens[i].text] = { stack_size, str_bytes.size() };
                                    stack_size += str_bytes.size();
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
        m_CompiledCode.push_back(alvm::Instruction{ .opcode = alvm::OpCode::End });
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
        return 
            (it != alvm::Instruction::InstructionStr.end()) 
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
