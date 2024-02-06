#include <fstream>
#include <iostream>
#include <vector>

#include <CommonDef.h>

#include "Analyzer/Parser.h"
#include "Compiler/Compiler.h"

#include <filesystem>

using namespace relang;
using namespace relang::refront;
using namespace relang::blend;

void DumpIntermediate(const relang::blend::InstructionList& code, const std::optional<std::string> filepath)
{
    // TODO: Code duplication.
    if (filepath != std::nullopt)
    {
        std::ofstream fs(*filepath);
        if (!fs.is_open())
        {
            std::cerr << "BASMSM: Error: Failed to create file to write intermediate code to.\n";
            return;
        }

        for (unsigned int i = 0; const auto& inst : code)
        {
            char fmt[256];
            std::sprintf(fmt, "0x%x:\t%02hhx %lx %02hhx %02hhx %x %02hhx %02hhx\t\t%s", i++, inst.opcode, inst.imm64,
                         inst.sreg, inst.dreg, inst.disp, inst.src_reg, inst.size,
                         relang::blend::Instruction::InstructionStr[(usize)inst.opcode].c_str());
            fs << fmt;

            switch (inst.size)
            {
                case 8: fs << 'b'; break;
                case 16: fs << 'w'; break;
                case 32: fs << 'l'; break;
                case 64: fs << 'q'; break;
            }

            if (inst.dreg != relang::blend::RegType::NUL)
            {
                if (inst.sreg != relang::blend::RegType::NUL)
                {
                    if (inst.sreg & relang::blend::RegType::PTR)
                    {
                        fs << " ";
                        if (inst.disp != 0)
                        {
                            fs << std::hex << ((inst.disp < 0) ? "-$0x" : "+$0x") << std::abs(inst.disp);
                        }
                        fs << "(%"
                           << relang::blend::Register::RegisterStr[(usize)(inst.sreg & relang::blend::RegType::DPTR)];
                        if (inst.src_reg != relang::blend::RegType::NUL)
                        {
                            fs << ", %" << relang::blend::Register::RegisterStr[(usize)inst.src_reg];
                        }
                        fs << ")";
                    }
                    else
                    {
                        fs << " %" << relang::blend::Register::RegisterStr[(usize)inst.sreg];
                    }
                }
                else
                {
                    fs << " $0x" << inst.imm64;
                }

                fs << ',';

                if (inst.dreg & relang::blend::RegType::PTR)
                {
                    fs << " ";
                    if (inst.disp != 0)
                    {
                        fs << std::hex << ((inst.disp < 0) ? "-$0x" : "+$0x") << std::abs(inst.disp);
                    }
                    fs << "(%"
                       << relang::blend::Register::RegisterStr[(usize)(inst.dreg & relang::blend::RegType::DPTR)];
                    if (inst.src_reg != relang::blend::RegType::NUL)
                    {
                        fs << ", %" << relang::blend::Register::RegisterStr[(usize)inst.src_reg];
                    }
                    fs << ")";
                }
                else
                {
                    fs << " %" << relang::blend::Register::RegisterStr[(usize)inst.dreg];
                }
            }
            else
            {
                if (inst.sreg != relang::blend::RegType::NUL)
                {
                    if (inst.sreg & relang::blend::RegType::PTR)
                    {
                        fs << " ";
                        if (inst.disp != 0)
                        {
                            fs << std::hex << ((inst.disp < 0) ? "-$0x" : "+$0x") << std::abs(inst.disp);
                        }
                        fs << "(%"
                           << relang::blend::Register::RegisterStr[(usize)(inst.sreg & relang::blend::RegType::DPTR)];
                        if (inst.src_reg != relang::blend::RegType::NUL)
                        {
                            fs << ", %" << relang::blend::Register::RegisterStr[(usize)inst.src_reg];
                        }
                        fs << ")";
                    }
                    else
                    {
                        fs << " %" << relang::blend::Register::RegisterStr[(usize)inst.sreg];
                    }
                }
                else
                {
                    switch (inst.opcode)
                    {
                        case relang::blend::OpCode::PInt:
                        case relang::blend::OpCode::Enter:
                        case relang::blend::OpCode::SConio:
                        case relang::blend::OpCode::Push:
                        case relang::blend::OpCode::Call:
                        case relang::blend::OpCode::Jz:
                        case relang::blend::OpCode::Jnz:
                        case relang::blend::OpCode::Js:
                        case relang::blend::OpCode::Jns:
                        case relang::blend::OpCode::Jo:
                        case relang::blend::OpCode::Jno:
                        case relang::blend::OpCode::Jc:
                        case relang::blend::OpCode::Jcn:
                        case relang::blend::OpCode::Jug:
                        case relang::blend::OpCode::Jul:
                        case relang::blend::OpCode::Jue:
                        case relang::blend::OpCode::June:
                        case relang::blend::OpCode::Juge:
                        case relang::blend::OpCode::Jule:
                        case relang::blend::OpCode::Jl: fs << " $0x" << inst.imm64; break;
                        default: break;
                    }
                }
            }
            fs << "\n";
        }
    }
    else
    {
        for (int i = 0; const auto& inst : code)
        {
            std::printf("0x%x:\t%02hhx %lx %02hhx %02hhx %x %02hhx %02hhx\t\t%s", i++, inst.opcode, inst.imm64,
                        inst.sreg, inst.dreg, inst.disp, inst.src_reg, inst.size,
                        relang::blend::Instruction::InstructionStr[(usize)inst.opcode].c_str());

            switch (inst.size)
            {
                case 8: std::cout << 'b'; break;
                case 16: std::cout << 'w'; break;
                case 32: std::cout << 'l'; break;
                case 64: std::cout << 'q'; break;
            }

            if (inst.dreg != relang::blend::RegType::NUL)
            {
                if (inst.sreg != relang::blend::RegType::NUL)
                {
                    if (inst.sreg & relang::blend::RegType::PTR)
                    {
                        std::cout << " ";
                        if (inst.disp != 0)
                        {
                            std::cout << std::hex << ((inst.disp < 0) ? "-$0x" : "+$0x") << std::abs(inst.disp);
                        }

                        RegType a = inst.sreg & relang::blend::RegType::DPTR;

                        std::cout << "(%"
                                  << relang::blend::Register::RegisterStr[inst.sreg & relang::blend::RegType::DPTR];
                        if (inst.src_reg != relang::blend::RegType::NUL)
                        {
                            std::cout << ", %" << relang::blend::Register::RegisterStr[inst.src_reg];
                        }
                        std::cout << ")";
                    }
                    else
                    {
                        std::cout << " %" << relang::blend::Register::RegisterStr[inst.sreg];
                    }
                }
                else
                {
                    std::cout << " $0x" << inst.imm64;
                }

                std::cout << ',';

                if (inst.dreg & relang::blend::RegType::PTR)
                {
                    std::cout << " ";
                    if (inst.disp != 0)
                    {
                        std::cout << std::hex << ((inst.disp < 0) ? "-$0x" : "+$0x") << std::abs(inst.disp);
                    }
                    std::cout << "(%" << relang::blend::Register::RegisterStr[inst.dreg & relang::blend::RegType::DPTR];
                    if (inst.src_reg != relang::blend::RegType::NUL)
                    {
                        std::cout << ", %" << relang::blend::Register::RegisterStr[inst.src_reg];
                    }
                    std::cout << ")";
                }
                else
                {
                    std::cout << " %" << relang::blend::Register::RegisterStr[inst.dreg];
                }
            }
            else
            {
                if (inst.sreg != relang::blend::RegType::NUL)
                {
                    if (inst.sreg & relang::blend::RegType::PTR)
                    {
                        std::cout << " ";
                        if (inst.disp != 0)
                        {
                            std::cout << std::hex << ((inst.disp < 0) ? "-$0x" : "+$0x") << std::abs(inst.disp);
                        }
                        std::cout << "(%"
                                  << relang::blend::Register::RegisterStr[inst.sreg & relang::blend::RegType::DPTR];
                        if (inst.src_reg != relang::blend::RegType::NUL)
                        {
                            std::cout << ", %" << relang::blend::Register::RegisterStr[inst.src_reg];
                        }
                        std::cout << ")";
                    }
                    else
                    {
                        std::cout << " %" << relang::blend::Register::RegisterStr[inst.sreg];
                    }
                }
                else
                {
                    switch (inst.opcode)
                    {
                        case relang::blend::OpCode::PInt:
                        case relang::blend::OpCode::PChr:
                        case relang::blend::OpCode::Enter:
                        case relang::blend::OpCode::SConio:
                        case relang::blend::OpCode::Push:
                        case relang::blend::OpCode::Call:
                        case relang::blend::OpCode::Jz:
                        case relang::blend::OpCode::Jnz:
                        case relang::blend::OpCode::Js:
                        case relang::blend::OpCode::Jns:
                        case relang::blend::OpCode::Jo:
                        case relang::blend::OpCode::Jno:
                        case relang::blend::OpCode::Jc:
                        case relang::blend::OpCode::Jcn:
                        case relang::blend::OpCode::Jug:
                        case relang::blend::OpCode::Jul:
                        case relang::blend::OpCode::Jue:
                        case relang::blend::OpCode::June:
                        case relang::blend::OpCode::Juge:
                        case relang::blend::OpCode::Jule:
                        case relang::blend::OpCode::Jl: std::cout << " $0x" << inst.imm64; break;
                        default: break;
                    }
                }
            }
            std::cout << "\n";
        }
    }
}

int main(int argc, const char* argv[])
{
    if (argc > 1)
    {
        std::cout << std::filesystem::current_path().string() << std::endl;

        std::ifstream fs(argv[1]);
        if (fs.is_open())
        {
            auto src    = std::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
            auto parser = Parser(src);
            auto tree   = parser.Parse();
            nlohmann::ordered_json json = tree;
            std::cout << std::setw(4) << json << std::endl;
            auto compiler      = Compiler(tree);
            auto compiled_code = compiler.Compile();

            std::cout << std::endl;
            DumpIntermediate(compiled_code, std::nullopt);
            std::cout << std::endl;

            auto vm = Blend(compiled_code.GetDataSection(), compiled_code.GetDataSection().size());
            i64  result{};
            vm.Run(compiled_code, result);
            return result;
        }
        else
        {
            std::cerr << "cmc: input file non-existent." << std::endl;
            return -1;
        }
    }
    else
        std::cout << "Usage:\n\tcmc [file]" << std::endl;
    return 0;
}
