#ifndef BLEND_RUNTIME_H
#define BLEND_RUNTIME_H

#include <sdafx.h>

#include "Instruction.h"
#include "Register.h"
#include "Utils.h"

namespace relang::blend {
    constexpr int STACK_SIZE = 1024 * 1024 * 2;
    constexpr u8 DATA_SECTION_INDIC = 0xFD;
    constexpr u8 CODE_SECTION_INDIC = 0xFC;
    constexpr u8 BSS_SECTION_INDIC = 0xFB;

    class Blend
    {
        using InstructionHandler = void (Blend::*)();

    private:
        Instruction* m_Bytecode = nullptr;
        Instruction* m_Pc = nullptr;
        std::vector<u8> m_Stack;
        Registers m_Registers;
        uintptr& m_Sp;
        usize m_BssSize = 0;
        const std::vector<InstructionHandler> m_Instructions =
            {
                &Blend::End,
                &Blend::Push,
                &Blend::Pop,
                &Blend::Add,
                &Blend::Sub,
                &Blend::Mul,
                &Blend::Div,
                &Blend::Neg,
                &Blend::Increment,
                &Blend::Decrement,
                &Blend::Printf,
                &Blend::PrintInt,
                &Blend::PrintStr,
                &Blend::PrintChar,
                &Blend::Compare,
                &Blend::Move,
                &Blend::Lea,
                &Blend::Enter,
                &Blend::Call,
                &Blend::Return,
                &Blend::Leave,
                &Blend::Malloc,
                &Blend::Free,
                &Blend::Memset,
                &Blend::Memcpy,
                &Blend::Lrzf,
                &Blend::Srzf,
                &Blend::Store,
                &Blend::Load,
                &Blend::System,
                &Blend::Syscall,
                &Blend::InvokeC,
                &Blend::GetChar,
                &Blend::Jump,
                &Blend::JmpIfZero,
                &Blend::JmpIfNotZero,
                &Blend::JmpIfSign,
                &Blend::JmpIfNotSign,
                &Blend::JmpIfOverflow,
                &Blend::JmpIfNotOverflow,
                &Blend::JmpIfCarry,
                &Blend::JmpIfNotCarry,

                &Blend::JmpIfNotCarry, //&Blend::JmpIfUnsignedGreaterThan,
                &Blend::JmpIfCarry,    //&Blend::JmpIFUnsignedLessThan,
                &Blend::JmpIfZero,     //&Blend::JmpIfUnsignedEqualTo,
                &Blend::JmpIfNotZero,  // June

                &Blend::JmpIfUnsignedGreaterOrEqualTo,
                &Blend::JmpIfUnsignedLesserOrEqualTo,

                &Blend::JmpIfSignedLessThan,

                &Blend::BitwiseAND,
                &Blend::BitwsieOR,
                &Blend::BitwiseNOT,
                &Blend::BitwiseXOR,
                &Blend::BitwiseTEST,

                &Blend::PushAllRegisters,
                &Blend::PopAllRegisters,

                &Blend::SetConioMode,
                &Blend::Debug_DumpFlags,
                &Blend::Nop};

    public:
        Blend(const std::vector<u8>& data, const usize bssSize);

    public:
        void Run(const std::vector<Instruction>& code, i64& result);

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
        void PrintChar();
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

        void PushAllRegisters();
        void PopAllRegisters();

        void SetConioMode();
        void Debug_DumpFlags();
        void Nop();
    };
} // namespace relang::blend

#endif // BLEND_RUNTIME_H
