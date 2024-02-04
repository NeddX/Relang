#include <fstream>
#include <iostream>
#include <vector>

#include "../include/BASM.h"

using namespace relang;

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
            std::sprintf(fmt, "0x%x:\t%02hhx %lx %02hhx %02hhx %x %02hhx %02hhx\t\t%s",
                         i++,
                         inst.opcode,
                         inst.imm64,
                         inst.sreg,
                         inst.dreg,
                         inst.displacement,
                         inst.src_reg,
                         inst.size,
                         relang::blend::Instruction::InstructionStr[(usize)inst.opcode].c_str());
            fs << fmt;

            switch (inst.size)
            {
                case 8:
                    fs << 'b';
                    break;
                case 16:
                    fs << 'w';
                    break;
                case 32:
                    fs << 'l';
                    break;
                case 64:
                    fs << 'q';
                    break;
            }

            if (inst.dreg != relang::blend::RegType::NUL)
            {
                if (inst.sreg != relang::blend::RegType::NUL)
                {
                    if (inst.sreg & relang::blend::RegType::PTR)
                    {
                        fs << " ";
                        if (inst.displacement != 0)
                        {
                            fs << std::hex << ((inst.displacement < 0) ? "-0x" : "+0x") << std::abs(inst.displacement);
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
                    if (inst.displacement != 0)
                    {
                        fs << std::hex << ((inst.displacement < 0) ? "-0x" : "+0x") << std::abs(inst.displacement);
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
                        if (inst.displacement != 0)
                        {
                            fs << std::hex << ((inst.displacement < 0) ? "-0x" : "+0x") << std::abs(inst.displacement);
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
                        case relang::blend::OpCode::Jl:
                            fs << " $0x" << inst.imm64;
                            break;
                        default:
                            break;
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
            std::printf("0x%x:\t%02hhx %lx %02hhx %02hhx %x %02hhx %02hhx\t\t%s",
                        i++,
                        inst.opcode,
                        inst.imm64,
                        inst.sreg,
                        inst.dreg,
                        inst.displacement,
                        inst.src_reg,
                        inst.size,
                        relang::blend::Instruction::InstructionStr[(usize)inst.opcode].c_str());

            switch (inst.size)
            {
                case 8:
                    std::cout << 'b';
                    break;
                case 16:
                    std::cout << 'w';
                    break;
                case 32:
                    std::cout << 'l';
                    break;
                case 64:
                    std::cout << 'q';
                    break;
            }

            if (inst.dreg != relang::blend::RegType::NUL)
            {
                if (inst.sreg != relang::blend::RegType::NUL)
                {
                    if (inst.sreg & relang::blend::RegType::PTR)
                    {
                        std::cout << " ";
                        if (inst.displacement != 0)
                        {
                            std::cout << std::hex << ((inst.displacement < 0) ? "-0x" : "+0x") << std::abs(inst.displacement);
                        }
                        std::cout << "(%"
                           << relang::blend::Register::RegisterStr[(usize)(inst.sreg & relang::blend::RegType::DPTR)];
                        if (inst.src_reg != relang::blend::RegType::NUL)
                        {
                            std::cout << ", %" << relang::blend::Register::RegisterStr[(usize)inst.src_reg];
                        }
                        std::cout << ")";
                    }
                    else
                    {
                        std::cout << " %" << relang::blend::Register::RegisterStr[(usize)inst.sreg];
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
                    if (inst.displacement != 0)
                    {
                        std::cout << std::hex << ((inst.displacement < 0) ? "-0x" : "+0x") << std::abs(inst.displacement);
                    }
                    std::cout << "(%"
                       << relang::blend::Register::RegisterStr[(usize)(inst.dreg & relang::blend::RegType::DPTR)];
                    if (inst.src_reg != relang::blend::RegType::NUL)
                    {
                        std::cout << ", %" << relang::blend::Register::RegisterStr[(usize)inst.src_reg];
                    }
                    std::cout << ")";
                }
                else
                {
                    std::cout << " %" << relang::blend::Register::RegisterStr[(usize)inst.dreg];
                }
            }
            else
            {
                if (inst.sreg != relang::blend::RegType::NUL)
                {
                    if (inst.sreg & relang::blend::RegType::PTR)
                    {
                        std::cout << " ";
                        if (inst.displacement != 0)
                        {
                            std::cout << std::hex << ((inst.displacement < 0) ? "-0x" : "+0x") << std::abs(inst.displacement);
                        }
                        std::cout << "(%"
                           << relang::blend::Register::RegisterStr[(usize)(inst.sreg & relang::blend::RegType::DPTR)];
                        if (inst.src_reg != relang::blend::RegType::NUL)
                        {
                            std::cout << ", %" << relang::blend::Register::RegisterStr[(usize)inst.src_reg];
                        }
                        std::cout << ")";
                    }
                    else
                    {
                        std::cout << " %" << relang::blend::Register::RegisterStr[(usize)inst.sreg];
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
                        case relang::blend::OpCode::Jl:
                            std::cout << " $0x" << inst.imm64;
                            break;
                        default:
                            break;
                    }
                }
            }
            std::cout << "\n";
        }
    }
}

int main(const int argc, const char* argv[])
{
    using namespace relang;
    using namespace relang::rmc;

    std::string output_filepath;
    std::string input_filepath;
    bool intermediate = false;
    bool disassemble = false;
    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "-D") == 0)
            {
                intermediate = true;
            }
            else if (std::strcmp(argv[i], "-o") == 0)
            {
                output_filepath = argv[++i];
            }
            else if (std::strcmp(argv[i], "-d") == 0)
            {
                disassemble = true;
            }
            else
            {
                // Must be a file name, hopefully.
                input_filepath = argv[i];
            }
        }

        if (output_filepath.empty())
        {
            output_filepath = input_filepath;
            usize pos = output_filepath.find_last_of(".");
            if (pos != std::string::npos)
            {
                output_filepath.replace(pos, output_filepath.length() - pos, ".alc");
            }
        }

        std::ifstream fs(input_filepath);

        if (fs.is_open())
        {
            if (disassemble)
            {
                blend::InstructionList code_section;
                std::vector<u8> data_section;
                fs.seekg(0, fs.end);
                usize file_size = fs.tellg();
                fs.seekg(0, fs.beg);

                u8 byte;
                usize size = 0;
                while (fs.read((char*)&byte, sizeof(u8)))
                {
                    switch (byte)
                    {
                        case blend::DATA_SECTION_INDIC:
                        {
                            fs.read((char*)&size, sizeof(usize));
                            data_section.resize(size / sizeof(u8));
                            fs.read((char*)data_section.data(), size);
                            break;
                        }
                        case blend::CODE_SECTION_INDIC:
                        {
                            fs.read((char*)&size, sizeof(usize));
                            code_section.resize(size / sizeof(blend::Instruction));
                            fs.read((char*)code_section.data(), size);
                            break;
                        }
                    }
                }
                DumpIntermediate(code_section, std::nullopt);
                fs.close();
                return EXIT_SUCCESS;
            }
            std::string src_code = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
            auto tk_list = relang::rmc::Lexer::Start(src_code);
            AssemblerOptions opt =
            {
                    .type = OutputType::XBin,
                    .tokens = tk_list,
                    .path = output_filepath
            };

            auto asmblr_result = Assembler::Assemble(opt);
            if (asmblr_result.status == AssemblerStatus::Ok)
            {
                if (intermediate)
                {
                    DumpIntermediate(asmblr_result.assembledCode, opt.path + ".int");
                }
                return 0;
            }
            fs.close();
        }
        else
        {
            std::cerr << "Error: Couldn't open file " << input_filepath << " for reading.\n";
            return -2;
        }
    }
    else
    {
        std::cerr << "Error: No input files.\n";
        return -1;
    }
    return 0;
}
