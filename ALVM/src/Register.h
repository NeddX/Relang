#ifndef ALVM_ALVM_REGISTER_H
#define ALVM_ALVM_REGISTER_H

#include <sdafx.h>

namespace rlang::alvm {
	enum class RegType
	{
		// General purpose
		R0,
		R1,
		R2,
		R3,
		R4,
		R5,
		R6,
		R7,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15,

		// Reserved
		Bp,		// Base pointer
		Sp,		// Stack pointer
		Fr,		// Function return pointer

		// Flags
		CF,		// Conditional flag, gets modified by Ciet, Cigt instructions.

		Nul
	};

	struct Register
	{
		RegType type = RegType::Nul;
		bool ptr = false;
		std::int8_t size = 32;
		std::int32_t ptr_offset = 0;

	public:
		inline static const std::vector<std::string> RegisterStr =
		{
			"R0",
			"R1",
			"R2",
			"R3",
			"R4",
			"R5",
			"R6",
			"R7",
			"R8",
			"R9",
			"R10",
			"R11",
			"R12",
			"R13",
			"R14",
			"R15",
			"Bp",
			"Sp",
			"Fr",
			"CF",
			"Nul"
		};
	};

	struct Registers
	{
	private:
		std::array<std::int32_t, (std::size_t)RegType::Nul> m_Buffer = { 0 };

	public:
		inline std::int32_t& operator[](std::size_t index) { return m_Buffer[index]; }
		inline std::int32_t& operator[](RegType reg) { return m_Buffer[(std::size_t)reg]; }
	};
}

#endif // ALVM_ALVM_REGISTER_H