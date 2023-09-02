#ifndef ALVM_RUNTIME_H
#define ALVM_RUNTIME_H

#include <sdafx.h>

#include "Register.h"
#include "Instruction.h"
#include "Utils.h"

namespace rlang::alvm {
	constexpr int STACK_SIZE = 1024 * 1024 * 2;
	constexpr std::uint8_t DATA_SECTION_INDIC = 0xFD;
	constexpr std::uint8_t CODE_SECTION_INDIC = 0xFC;
	constexpr std::uint8_t BSS_SECTION_INDIC = 0xFB;

	class ALVM
	{
		using InstructionHandler = void(ALVM::*)();

	private:
		Instruction* m_Bytecode = nullptr;
		Instruction* m_Pc = nullptr;
		std::vector<std::uint8_t> m_Stack;
		Registers m_Registers;
		std::uintptr_t& m_Sp;
		std::size_t m_BssSize = 0;
		const std::vector<InstructionHandler> m_Instructions =
		{
			&ALVM::End,
			&ALVM::Push,
			&ALVM::Pop,
			&ALVM::Add,
			&ALVM::Sub,
			&ALVM::Mul,
			&ALVM::Div,
			&ALVM::Neg,
			&ALVM::Increment,
			&ALVM::Decrement,
			&ALVM::Printf,
			&ALVM::PrintInt,
			&ALVM::PrintStr,
            &ALVM::Compare,
			&ALVM::Move,
			&ALVM::Lea,
			&ALVM::Enter,
			&ALVM::Call,
			&ALVM::Return,
			&ALVM::Leave,
			&ALVM::Malloc,
			&ALVM::Free,
			&ALVM::Memset,
			&ALVM::Memcpy,
			&ALVM::Lrzf,
			&ALVM::Srzf,
			&ALVM::Store,
			&ALVM::Load,
			&ALVM::System,
			&ALVM::Syscall,
			&ALVM::InvokeC,
			&ALVM::GetChar,
			&ALVM::Jump,
			&ALVM::JmpIfZero,
			&ALVM::JmpIfNotZero,
			&ALVM::JmpIfSign,
			&ALVM::JmpIfNotSign,
			&ALVM::JmpIfOverflow,
			&ALVM::JmpIfNotOverflow,
			&ALVM::JmpIfCarry,
			&ALVM::JmpIfNotCarry,

			&ALVM::JmpIfNotCarry,//&ALVM::JmpIfUnsignedGreaterThan,
			&ALVM::JmpIfCarry, //&ALVM::JmpIFUnsignedLessThan,
			&ALVM::JmpIfZero, //&ALVM::JmpIfUnsignedEqualTo,
			&ALVM::JmpIfNotZero, // June

			&ALVM::JmpIfUnsignedGreaterOrEqualTo,
			&ALVM::JmpIfUnsignedLesserOrEqualTo,

			&ALVM::JmpIfSignedLessThan,

			&ALVM::BitwiseAND,
			&ALVM::BitwsieOR,
			&ALVM::BitwiseNOT,
			&ALVM::BitwiseXOR,
			&ALVM::BitwiseTEST,

			&ALVM::SetConioMode,
			&ALVM::Debug_DumpFlags,
			&ALVM::Nop
	};
	public:
		ALVM(const std::vector<std::uint8_t>& data, const std::size_t bssSize);

	public:
		void Run(const std::vector<Instruction>& code, std::int64_t& result);

	private:
		void End();
		void Push();
		void Pop();
		void Add();
		void Sub();
		void Mul();
		void Div();
		void Neg();
		void Increment();
		void Decrement();
		void Printf();
		void PrintInt();
		void PrintStr();
        void Compare();
		void Move();
		void Lea();
		void Enter();
		void Call();
		void Return();
		void Leave();
		void Malloc();
		void Free();
		void Memset();
		void Memcpy();
		void Lrzf();
		void Srzf();
		void Store();
		void Load();
		void System();
		void Syscall();
		void InvokeC();
		void GetChar();
		void Jump();
		void JmpIfZero();
		void JmpIfNotZero();
		void JmpIfSign();
		void JmpIfNotSign();
		void JmpIfOverflow();
		void JmpIfNotOverflow();
		void JmpIfCarry();
		void JmpIfNotCarry();

		void JmpIfUnsignedGreaterOrEqualTo();
		void JmpIfUnsignedLesserOrEqualTo();

		void JmpIfSignedLessThan();
		void BitwiseAND();
		void BitwsieOR();
		void BitwiseNOT();
		void BitwiseXOR();
		void BitwiseTEST();

		void SetConioMode();
		void Debug_DumpFlags();
		void Nop();
	};
}

#endif // ALVM_RUNTIME_H
