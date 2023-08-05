#include <iostream>
#include <fstream>
#include <vector>

#include "../include/ALASM.h"

void DumpIntermediate(const rlang::alvm::InstructionList& code, const std::string& filepath)
{
	std::ofstream fs(filepath);
	if (!fs.is_open())
	{
		std::cerr << "ALASMSM: Error: Failed to create file to write intermediate code to.\n";
		return;
	}

	for (int i = 0; const auto& inst : code)
	{
		fs << "0x" << i++ << ":\t" << std::hex << "0x" << (std::uint64_t)inst.opcode << "\t\t" << rlang::alvm::Instruction::InstructionStr[(std::size_t)inst.opcode];

		if (inst.reg1 == rlang::alvm::RegType::NUL)
		{
			fs << " #0x" << inst.imm64;
		}
		else
		{
			if (inst.reg1 & rlang::alvm::RegType::PTR)
			{
				fs << " ";
				switch (inst.size)
				{
					case 8:
						fs << "byte";
						break;
					case 16:
						fs << "word";
						break;
					case 32:
						fs << "dword";
						break;
					default:
					case 64:
						fs << "qword";
						break;
				}
				fs << " [%" << rlang::alvm::Register::RegisterStr[(std::size_t)(inst.reg1 & rlang::alvm::RegType::DPTR)];
				if (inst.displacement != 0)
				{
					fs << ((inst.displacement < 0) ? "-#0x" : "+#0x") << std::abs(inst.displacement);
				}
				fs << "]";
			}
			else
			{
				fs << " %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.reg1];
			}

			if (inst.reg2 != rlang::alvm::RegType::NUL)
			{
				if (inst.reg2 & rlang::alvm::RegType::PTR)
				{
					fs << ", ";
					switch (inst.size)
					{
						case 8:
							fs << "byte";
							break;
						case 16:
							fs << "word";
							break;
						case 32:
							fs << "dword";
							break;
						default:
						case 64:
							fs << "qword";
							break;
					}
					fs << " [%" << rlang::alvm::Register::RegisterStr[(std::size_t)(inst.reg2 & rlang::alvm::RegType::DPTR)];
					if (inst.displacement != 0)
					{
						fs << ((inst.displacement < 0) ? "-#0x" : "+#0x") << std::abs(inst.displacement);
					}
					fs << "]";
				}
				else
				{
					fs << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.reg2];
				}
			}
			else
			{
				if (inst.opcode != rlang::alvm::OpCode::Push &&
					inst.opcode != rlang::alvm::OpCode::End &&
					inst.opcode != rlang::alvm::OpCode::PStr &&
					inst.opcode != rlang::alvm::OpCode::PInt &&
					inst.opcode != rlang::alvm::OpCode::Inc &&
					inst.opcode != rlang::alvm::OpCode::Dec) fs << ", #0x" << inst.imm64;
			}
		}
		fs << "\n";
	}
}

int main(const int argc, const char* argv[])
{
	using namespace rlang::rmc;

	std::string output_filepath;
	std::string input_filepath;
	bool intermediate = false;
	if (argc > 1)
	{
		for (int i = 1; i < argc; ++i)
		{
			if (std::strcmp(argv[i], "--dump-intermediate") == 0)
			{
				intermediate = true;
			}
			else if (std::strcmp(argv[i], "-o") == 0)
			{
				output_filepath = argv[++i];
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
			std::size_t pos = output_filepath.find_last_of(".");
			if (pos != std::string::npos)
			{
				output_filepath.replace(pos, output_filepath.length() - pos, ".alc");
			}
		}

		std::ifstream fs(input_filepath);

		if (fs.is_open())
		{
			std::string src_code = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
			auto tk_list = rlang::rmc::Lexer::Start(src_code);
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

				std::cout << "Assemble finished.\n";
				return 0;
			}
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
