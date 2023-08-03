#ifndef ALVM_REGISTER_H
#define ALVM_REGISTER_H

#include <sdafx.h>

namespace rlang::alvm {
	enum class RegType : std::uint8_t
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
		BP,
		SP,

		// Status Flags Register
		SFR,

		// Flags
		//ZF,
		//CF,
		//OF,
		//SF,
		//PF,
		//AF,

		// Segement pointers
		CS,
		SS,
		DS,
		ES,
		FS,
		GS,

		Nul
	};

	struct Register
	{
		RegType type = RegType::Nul;
		bool ptr = false;
		std::int64_t displacement = 0;

	public:
		inline static const std::vector<std::string> RegisterStr =
		{
			"r0",
			"r1",
			"r2",
			"r3",
			"r4",
			"r5",
			"r6",
			"r7",
			"r8",
			"r9",
			"r10",
			"r11",
			"r12",
			"r13",
			"r14",
			"r15",

			"bp",
			"sp",

			"sfr",

			//"zf",
			//"cf",
			//"of",
			//"sf",
			//"pf",
			//"af",

			"cs",
			"ss",
			"ds",
			"es",
			"fs",
			"gs",

			"nul"
		};
	};

	struct Registers
	{
	private:
		std::array<std::uintptr_t, (std::size_t)RegType::Nul> m_Buffer = { 0 };

	public:
		inline std::uintptr_t& operator[](const std::size_t index) { return m_Buffer[index]; }
		inline std::uintptr_t& operator[](const RegType reg) { return m_Buffer[(std::size_t)reg]; }
		inline std::uintptr_t& operator[](const Register reg) { return m_Buffer[(std::size_t)reg.type]; }
	};
}

#endif // ALVM_REGISTER_H
