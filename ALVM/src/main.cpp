#include <sdafx.h>

#include "../include/ALVM.h"

int main(const int argc, const char* argv[])
{
    using namespace rlang::alvm;

    std::int64_t result = 0;
    if (argc > 1)
    {
        std::ifstream fs(argv[1]);
        if (fs.is_open())
        {
            InstructionList code_section;
            std::vector<std::uint8_t> data_section;

            fs.seekg(0, fs.end);
            std::size_t file_size = fs.tellg();
            fs.seekg(0, fs.beg);

            std::uint8_t byte;
            std::size_t size = 0;
            while (fs.read((char*)&byte, sizeof(std::uint8_t)))
            {
                switch (byte)
                {
                    case DATA_SECTION_INDIC:
                    {
                        fs.read((char*)&size, sizeof(std::size_t));
                        data_section.resize(size / sizeof(std::uint8_t));
                        fs.read((char*)data_section.data(), size);
                        ;
                        break;
                    }
                    case CODE_SECTION_INDIC:
                    {
                        fs.read((char*)&size, sizeof(std::size_t));
                        code_section.resize(size / sizeof(Instruction));
                        fs.read((char*)code_section.data(), size);
                        break;
                    }
                }

                ALVM vm(data_section);
                vm.Run(code_section, result);
            }
        }
        else
        {
            std::cerr << "Error: Couldn't open file " << argv[1] << " for reading.\n";
            result = -2;
        }
    }
    else
    {
        std::cerr << "Error: Shell interpreted state not yet supported.\n";
        result = -1;
    }
    return result;
}
