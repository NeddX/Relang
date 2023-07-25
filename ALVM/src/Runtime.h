#ifndef ALVM_RUNTIME_H
#define ALVM_RUNTIME_H

#include <sdafx.h>

#include "Register.h"
#include "ByteBuffer.h"
#include "Instruction.h"

namespace rlang::alvm {
	constexpr auto STACK_SIZE = 30;//1024 * 1024 * 2;

	class ALVM
	{
		using InstructionHandler = void(ALVM::*)();

	private:
		Instruction* m_Bytecode = nullptr;
		Instruction* m_Pc = nullptr;
		Registers m_Registers;
		size_t m_Size = 0;
		ByteBuffer m_Stack;
		ByteBuffer m_Data;
		const std::vector<InstructionHandler> m_Instructions =
		{
			&ALVM::End,
			&ALVM::Push,
			&ALVM::Pop,
			&ALVM::Add,
			&ALVM::Sub,
			&ALVM::Mul,
			&ALVM::Div,
			&ALVM::Increment,
			&ALVM::Decrement,
			&ALVM::PrintInt,
			&ALVM::PrintStr,
			&ALVM::CmpIntLessThan,
			&ALVM::CmpIntEqualTo,
            &ALVM::Cmp,
			&ALVM::Move,
			&ALVM::Jump,
			&ALVM::ConditionalJump,
			&ALVM::ConditionalNotJump,
			&ALVM::Db,
			&ALVM::Call,
			&ALVM::Return,
			&ALVM::Malloc,
			&ALVM::Free,
			&ALVM::Nop
		};
	public:
		ALVM(const std::vector<Instruction>& bytecode, const std::vector<std::uint8_t>& data, std::int32_t result) :
			m_Stack(STACK_SIZE, (size_t&)m_Registers[RegType::Sp]),
			m_Data(data)
		{

		}

	public:
		void Run(const std::vector<Instruction>& bytecode, std::int32_t& result);

	private:
		void Nop();
		void End();
		void Push();
		void Pop();
		void Add();
		void Sub();
		void Mul();
		void Div();
		void Increment();
		void Decrement();
		void PrintInt();
		void PrintStr();
		void CmpIntLessThan();
		void CmpIntEqualTo();
        void Cmp();
		void Move();
		void Jump();
		void ConditionalJump();
		void ConditionalNotJump();
		void Db();
		void Call();
		void Return();
		void Malloc();
		void Free();
	};
}

#endif // ALVM_RUNTIME_H
