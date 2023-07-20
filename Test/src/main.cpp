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

    std::cout << "\nTesting ALA...\n";
    std::ifstream fs;
    fs.open("./basic.amc");

    std::string src;
    if (fs.is_open())
    {
        src = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
        fs.close();
    }
    else std::cout << "couldnt open file\n";

    rlang::rmc::TokenList tk_list = rlang::rmc::Lexer::Start(src);
    std::vector<rlang::alvm::Instruction> compiled_code = rlang::rmc::Compiler::Compile(tk_list);

    rlang::alvm::ALVM r;
    std::int32_t result = 0;
    r.Run(compiled_code, result); // mr synctactical shugar
    std::cout << "\nExited with code: " << result << std::endl;

    return 0;
}
