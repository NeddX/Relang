#ifndef ALVM_ALVM_RUNTIME_H
#define ALVM_ALVM_RUNTIME_H

#include <sdafx.h>

#include "Register.h"
#include "ByteBuffer.h"
#include "Instruction.h"

namespace rlang::alvm {
	constexpr auto ALVM_STACK_SIZE = 50;//1024 * 1024 * 2;

	class ALVM
	{
		using InstructionHandler = void(ALVM::*)();

	private:
		Instruction* m_Bytecode = nullptr;
		Instruction* m_Pc = nullptr;
		Registers m_Registers;
		size_t m_Size = 0;
		ByteBuffer<ALVM_STACK_SIZE> m_Stack;
		//std::int32_t m_BaseIndex = 0;
		const std::vector<InstructionHandler> m_Instructions =
		{
			&ALVM::End,
			&ALVM::Push,
			&ALVM::Pop,
			&ALVM::Add,
			&ALVM::Sub,
			&ALVM::Mul,
			&ALVM::Div,
			&ALVM::PrintInt,
			&ALVM::PrintStr,
			&ALVM::CmpIntLessThan,
			&ALVM::CmpIntEqualTo,
			&ALVM::Move,
			&ALVM::Jump,
			&ALVM::ConditionalJump,
			&ALVM::ConditionalNotJump,
			&ALVM::Call,
			&ALVM::Return,
			&ALVM::Db,
			&ALVM::Call,
			&ALVM::Return,
			&ALVM::Nop
		};
	public:
		ALVM() :
			m_Stack((size_t&)m_Registers[RegType::Sp])
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
		void PrintInt();
		void PrintStr();
		void CmpIntLessThan();
		void CmpIntEqualTo();
		void Move();
		void Jump();
		void ConditionalJump();
		void ConditionalNotJump();
		void Db();
		void Call();
		void Return();
	};
}

#endif // ALVM_ALVM_RUNTIME_H
