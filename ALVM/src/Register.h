#ifndef ALVM_REGISTER_H
#define ALVM_REGISTER_H

#include <sdafx.h>

namespace rlang::alvm {
	enum RegType : std::uint8_t
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
		R16,
		R17,
		R18,
		R19,
		R20,
		R21,
		R22,
		R23,
		R24,
		R25,
		R26,
		R27,
		R28,
		R29,
		R30,
		R31,

		// Reserved
		BP,
		SP,

		// Status Flags Register
		SFR,

		// Segement registers
		CS,
		SS,
		DS,
		ES,
		FS,
		GS,

		// Special, Not real regisers
		// No register
		NUL,
		// Indicates if a registers is a pointer
		PTR = 0x80,
		// Inverted PTR for dereferencing and recovering the original register using bitwise AND
		DPTR = 0x7F
	};

	struct Register
	{
		RegType type = RegType::NUL;
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
			"r16",
			"r17",
			"r18",
			"r19",
			"r20",
			"r21",
			"r22",
			"r23",
			"r24",
			"r25",
			"r26",
			"r27",
			"r28",
			"r29",
			"r30",
			"r31",

			"bp",
			"sp",

			"sfr",

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
		std::array<std::uintptr_t, (std::size_t)RegType::NUL + 1> m_Buffer = { 0 };

	public:
		inline std::uintptr_t& operator[](const std::size_t index) { return m_Buffer[index]; }
		inline std::uintptr_t& operator[](const RegType reg) { return m_Buffer[(std::size_t)reg]; }
		inline std::uintptr_t& operator[](const Register reg) { return m_Buffer[(std::size_t)reg.type]; }
	};
}

#endif // ALVM_REGISTER_H
