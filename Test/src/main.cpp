#include <iostream>
#include <string>
#include <sstream>

#include "CommonUtils.h"
#include "ALVM_Testman.h"

#include <ALA.h>

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
    //std::cout << "Testing ALVM...\n";
    //TestAndExpect(&testman::alvm::SimplePrint, "it be workin!");

    //std::cout << "\nTesting ALA...\n";
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

    rlang::rmc::TokenList tk_list = rlang::rmc::Lexer::Start(src);
    auto compiler_result = rlang::rmc::Compiler::Compile(tk_list);
    std::vector<rlang::alvm::Instruction> compiled_code = compiler_result.first;

    std::cout << "Compilation finished.\n";
    if (argc > 1 && std::strcmp(argv[1], "--dump-intermediate") == 0)
    {
        std::cout << "Dumping intermediate code...\n\n";
        for (int i = 0; const auto& inst : compiled_code)
        {
            std::cout << "0x" << i++ << ":\t" << std::hex << "0x" << (std::uint64_t)inst.opcode << "\t\t" << rlang::alvm::Instruction::InstructionStr[(std::size_t)inst.opcode];

            if (inst.reg1.type == rlang::alvm::RegType::Nul)
            {
                std::cout << " #0x" << inst.imm64;
            }
            else
            {
                if (inst.reg1.ptr)
                {
                    std::cout << " ";
                    switch (inst.reg1.size)
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
                    std::cout << " [%" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.reg1.type];
                    if (inst.reg1.displacement != 0)
                    {
                        std::cout << ((inst.reg1.displacement < 0) ? "-#0x" : "+#0x") << std::abs(inst.reg1.displacement);
                    }
                    std::cout << "]";
                }
                else
                {
                    std::cout << " %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.reg1.type];
                }

                if (inst.reg2.type != rlang::alvm::RegType::Nul)
                {
                    if (inst.reg2.ptr)
                    {
                            std::cout << ", ";
                            switch (inst.reg1.size)
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
                            std::cout << " [%" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.reg2.type];
                            if (inst.reg2.displacement != 0)
                            {
                                std::cout << ((inst.reg2.displacement < 0) ? "-#0x" : "+#0x") << std::abs(inst.reg2.displacement);
                            }
                            std::cout << "]";
                    }
                    else
                    {
                        std::cout << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.reg2.type];
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
    rlang::alvm::ALVM r(compiler_result.second);
    std::int32_t result = 0;
    r.Run(compiled_code, result); // mr synctactical shugar
    std::cout << "Exited with code: " << result << std::endl;

    return 0;
}
