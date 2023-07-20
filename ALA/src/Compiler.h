#ifndef ALVM_ALA_COMPILER_H
#define ALVM_ALA_COMPILER_H

#include <unordered_map>
#include <cstdint>
#include <ALVM.h>

#include "Lexer.h"
#include "Utils.h"

namespace rlang::rmc {
	struct MemInfo 
	{
		std::int32_t addr = 0;
		std::size_t size = 0;
	};

	struct Compiler
	{
	private:
		static std::unordered_map<std::string, MemInfo> m_DbNameTable;
		static std::unordered_map<std::string, std::size_t> m_LabelAddressMap;
		static std::vector<alvm::Instruction> m_CompiledCode;
		static std::vector<alvm::Instruction> m_InstEpilogue;

	private:
		static void Cleanup();

	public:
		static std::vector<alvm::Instruction> Compile(const TokenList& tokens);
		
	private:
		static alvm::OpCode GetInst(std::string inst);
		static alvm::RegType GetReg(std::string reg);
	};
}

#endif // ALVM_ALA_COMPILER_H
