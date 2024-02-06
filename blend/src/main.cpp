#include <sdafx.h>

#include "../include/Blend.h"

using namespace relang;
using namespace relang::blend;

int main(const int argc, const char* argv[])
{

    i64 result = 0;
    if (argc > 1)
    {
        std::ifstream fs(argv[1]);
        if (fs.is_open())
        {
            InstructionList code_section;
            std::vector<u8> data_section;
            usize bss_size;

            fs.seekg(0, fs.end);
            usize file_size = fs.tellg();
            fs.seekg(0, fs.beg);

            u8 byte;
            usize size = 0;
            while (fs.read((char*)&byte, sizeof(u8)))
            {
                switch (byte)
                {
                    case DATA_SECTION_INDIC:
                    {
                        fs.read((char*)&size, sizeof(usize));
                        data_section.resize(size / sizeof(u8));
                        fs.read((char*)data_section.data(), size);
                        break;
                    }
                    case BSS_SECTION_INDIC:
                    {
                        fs.read((char*)&bss_size, sizeof(usize));
                        break;
                    }
                    case CODE_SECTION_INDIC:
                    {
                        fs.read((char*)&size, sizeof(usize));
                        code_section.resize(size / sizeof(Instruction));
                        fs.read((char*)code_section.data(), size);
                        break;
                    }
                }
            }

            Blend vm(data_section, bss_size);
            vm.Run(code_section, result);
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
