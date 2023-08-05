#include <iostream>
#include <string>
#include <sstream>

#include "CommonUtils.h"
#include "ALVM_Testman.h"

#include <ALASM.h>

using std::ifstream;

void TestAndExpect(void (*function)(), const std::string& expect)
{
    static std::size_t test_count = 0;
    std::stringstream ss;
    std::streambuf* old = std::cout.rdbuf(ss.rdbuf());
    function();
    std::cout.rdbuf(old);
    std::string output = ss.str();
    if (output == expect)
        std::cout << "Test " << test_count++ << " passed.\n";
    else
        std::cout << "Test " << test_count++ << " failed.\n\tExpected: " << expect
                  << " but got: " << output << "\n";
}

int main(int argc, const char* argv[])
{
    /*
    std::ifstream fs;
    fs.open("./basic.amc");

    std::string src;
    if (fs.is_open())
    {
        src = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
        fs.close();
    }
    else
    {
        std::cout << "Error: Couldn't open file\n";
        std::exit(-1);
    }

    using Inst = rlang::alvm::Instruction;
    using Code = rlang::alvm::OpCode;
    using Reg = rlang::alvm::RegType;

    auto tk_list = rlang::rmc::Lexer::Start(src);

    const rlang::rmc::AssemblerOptions opt =
    {
        .type = rlang::rmc::OutputType::XBin,
        .tokens = tk_list,
        .filename = "o.aex",
        .path = "./"
    };

    auto assembler_result = rlang::rmc::Assembler::Assemble(opt);
    auto& assembled_code = assembler_result.assembledCode;

    if (assembler_result.status != rlang::rmc::AssemblerStatus::Ok)
    {
        std::cerr << "\nCompilation failed.\n";
        std::exit(-1);
    }

    std::cout << "Compilation finished.\n";
    if (argc > 1 && std::strcmp(argv[1], "--dump-intermediate") == 0)
    {
        std::cout << "Dumping intermediate code...\n\n";
        for (int i = 0; const auto& inst : assembled_code)
        {
            std::cout << "0x" << i++ << ":\t" << std::hex << "0x" << (std::uint64_t)inst.opcode << "\t\t" << rlang::alvm::Instruction::InstructionStr[(std::size_t)inst.opcode];

            if (inst.reg1 == rlang::alvm::RegType::NUL)
            {
                std::cout << " #0x" << inst.imm64;
            }
            else
            {
                if (inst.reg1 & rlang::alvm::RegType::PTR)
                {
                    std::cout << " ";
                    switch (inst.size)
                    {
                        case 8:
                            std::cout << "byte";
                            break;
                        case 16:
                            std::cout << "word";
                            break;
                        case 32:
                            std::cout << "dword";
                            break;
                        default:
                        case 64:
                            std::cout << "qword";
                            break;
                    }
                    std::cout << " [%" << rlang::alvm::Register::RegisterStr[(std::size_t)(inst.reg1 & rlang::alvm::RegType::DPTR)];
                    if (inst.displacement != 0)
                    {
                        std::cout << ((inst.displacement < 0) ? "-#0x" : "+#0x") << std::abs(inst.displacement);
                    }
                    std::cout << "]";
                }
                else
                {
                    std::cout << " %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.reg1];
                }

                if (inst.reg2 != rlang::alvm::RegType::NUL)
                {
                    if (inst.reg2 & rlang::alvm::RegType::PTR)
                    {
                            std::cout << ", ";
                            switch (inst.size)
                            {
                                case 8:
                                    std::cout << "byte";
                                    break;
                                case 16:
                                    std::cout << "word";
                                    break;
                                case 32:
                                    std::cout << "dword";
                                    break;
                                default:
                                case 64:
                                    std::cout << "qword";
                                    break;
                            }
                            std::cout << " [%" << rlang::alvm::Register::RegisterStr[(std::size_t)(inst.reg2 & rlang::alvm::RegType::DPTR)];
                            if (inst.displacement != 0)
                            {
                                std::cout << ((inst.displacement < 0) ? "-#0x" : "+#0x") << std::abs(inst.displacement);
                            }
                            std::cout << "]";
                    }
                    else
                    {
                        std::cout << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.reg2];
                    }
                }
                else
                {
                    if (inst.opcode != rlang::alvm::OpCode::Push &&
                        inst.opcode != rlang::alvm::OpCode::End &&
                        inst.opcode != rlang::alvm::OpCode::PStr &&
                        inst.opcode != rlang::alvm::OpCode::PInt &&
                        inst.opcode != rlang::alvm::OpCode::Inc &&
                        inst.opcode != rlang::alvm::OpCode::Dec) std::cout << ", #0x" << inst.imm64;
                }
            }
            std::cout << "\n";
        }
    }

    std::cout << std::endl;
    rlang::alvm::ALVM r(assembler_result.dataSection);
    std::int32_t result = 0;
    r.Run(assembled_code, result); // mr synctactical shugar
    std::cout << "Exited with code: " << result << std::endl;
    */
    return 0;
}
