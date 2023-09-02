#include <iostream>
#include <fstream>
#include <vector>

#include "../include/ALASM.h"

void DumpIntermediate(const rlang::alvm::InstructionList& code, const std::optional<std::string> filepath)
{
	// TODO: Code duplication.
	if (filepath != std::nullopt)
	{
		std::ofstream fs(*filepath);
		if (!fs.is_open())
		{
			std::cerr << "ALASMSM: Error: Failed to create file to write intermediate code to.\n";
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
						rlang::alvm::Instruction::InstructionStr[(std::size_t)inst.opcode].c_str());

			/*
			fs << "0x" << i++ << ":\t" << std::hex << (std::uint16_t)inst.opcode;
			fs << " " << inst.imm64 << " " << (std::uint16_t)inst.reg1 << " " << (std::uint16_t)inst.reg2;
			fs << " " << inst.displacement << " " << (std::uint16_t)inst.src_reg << " " << (std::uint16_t)inst.size;
			fs << "\t\t\t" << rlang::alvm::Instruction::InstructionStr[(std::size_t)inst.opcode];
			*/
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

			if (inst.sreg == rlang::alvm::RegType::NUL)
			{
				fs << " $0x" << inst.imm64;
			}
			else
			{
				if (inst.sreg & rlang::alvm::RegType::PTR)
				{
					fs << " ";
					if (inst.displacement != 0)
					{
						fs << std::hex << ((inst.displacement < 0) ? "-$0x" : "+$0x") << std::abs(inst.displacement);
					}
					fs << "(%"
					   << rlang::alvm::Register::RegisterStr[(std::size_t)(inst.sreg & rlang::alvm::RegType::DPTR)];
					if (inst.src_reg != rlang::alvm::RegType::NUL)
					{
						fs << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.src_reg];
					}
					fs << ")";
				}
				else
				{
					fs << " %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.sreg];
				}

				if (inst.dreg != rlang::alvm::RegType::NUL)
				{
					if (inst.dreg & rlang::alvm::RegType::PTR)
					{
						fs << ", ";
						if (inst.displacement != 0)
						{
							fs << std::hex << ((inst.displacement < 0) ? "-0x" : "+0x") << std::abs(inst.displacement);
						}
						fs << " (%"
						   << rlang::alvm::Register::RegisterStr[(std::size_t)(inst.dreg & rlang::alvm::RegType::DPTR)];
						if (inst.src_reg != rlang::alvm::RegType::NUL)
						{
							fs << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.src_reg];
						}
						fs << ")";
					}
					else
					{
						fs << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.dreg];
					}
				}
				else
				{
					if (inst.opcode != rlang::alvm::OpCode::Push &&
						inst.opcode != rlang::alvm::OpCode::End &&
						inst.opcode != rlang::alvm::OpCode::PStr &&
						inst.opcode != rlang::alvm::OpCode::PInt &&
						inst.opcode != rlang::alvm::OpCode::Inc &&
						inst.opcode != rlang::alvm::OpCode::Dec)
					{
						fs << ", $0x" << inst.imm64;
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
						rlang::alvm::Instruction::InstructionStr[(std::size_t)inst.opcode].c_str());
			/*
			std::cout << "0x" << i++ << ":\t" << std::hex << (std::uint16_t)inst.opcode;
			std::cout << " " << inst.imm64 << " " << (std::uint16_t)inst.reg1 << " " << (std::uint16_t)inst.reg2;
			std::cout << " " << inst.displacement << " " << (std::uint16_t)inst.src_reg << " " << (std::uint16_t)inst.size;
			std::cout << "\t\t\t" << rlang::alvm::Instruction::InstructionStr[(std::size_t)inst.opcode];
			*/
			
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

			if (inst.sreg == rlang::alvm::RegType::NUL)
			{
				std::cout << std::hex << " $0x" << inst.imm64;
			}
			else
			{
				if (inst.sreg & rlang::alvm::RegType::PTR)
				{
					std::cout << " ";
					if (inst.displacement != 0)
					{
						std::cout << std::hex << ((inst.displacement < 0) ? "-0x" : "+0x") << std::abs(inst.displacement);
					}
					std::cout << "(%"
					   << rlang::alvm::Register::RegisterStr[(std::size_t)(inst.sreg & rlang::alvm::RegType::DPTR)];
					if (inst.src_reg != rlang::alvm::RegType::NUL)
					{
						std::cout << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.src_reg];
					}
					std::cout << ")";
				}
				else
				{
					std::cout << " %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.sreg];
				}

				if (inst.dreg != rlang::alvm::RegType::NUL)
				{
					if (inst.dreg & rlang::alvm::RegType::PTR)
					{
						std::cout << ", ";
						if (inst.displacement != 0)
						{
							std::cout << ((inst.displacement < 0) ? "-0x" : "+0x") << std::hex << std::abs(inst.displacement);
						}
						std::cout << "(%"
						   << rlang::alvm::Register::RegisterStr[(std::size_t)(inst.dreg & rlang::alvm::RegType::DPTR)];
						if (inst.src_reg != rlang::alvm::RegType::NUL)
						{
							std::cout << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.src_reg];
						}
						std::cout << ")";
					}
					else
					{
						std::cout << ", %" << rlang::alvm::Register::RegisterStr[(std::size_t)inst.dreg];
					}
				}
				else
				{
					if (inst.opcode != rlang::alvm::OpCode::Push &&
						inst.opcode != rlang::alvm::OpCode::End &&
						inst.opcode != rlang::alvm::OpCode::PStr &&
						inst.opcode != rlang::alvm::OpCode::PInt &&
						inst.opcode != rlang::alvm::OpCode::Inc &&
						inst.opcode != rlang::alvm::OpCode::Dec) std::cout << ", $0x" << inst.imm64;
				}
			}
			std::cout << "\n";
		}
	}
}

int main(const int argc, const char* argv[])
{
	using namespace rlang;
	using namespace rlang::rmc;

	std::string output_filepath;
	std::string input_filepath;
	bool intermediate = false;
	bool disassemble = false;
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
			std::size_t pos = output_filepath.find_last_of(".");
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
				alvm::InstructionList code_section;
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
						case alvm::DATA_SECTION_INDIC:
						{
							fs.read((char*)&size, sizeof(std::size_t));
							data_section.resize(size / sizeof(std::uint8_t));
							fs.read((char*)data_section.data(), size);
							break;
						}
						case alvm::CODE_SECTION_INDIC:
						{
							fs.read((char*)&size, sizeof(std::size_t));
							code_section.resize(size / sizeof(alvm::Instruction));
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
