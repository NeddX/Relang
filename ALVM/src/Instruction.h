#ifndef ALVM_ALVM_INSTRUCTION_H
#define ALVM_ALVM_INSTRUCTION_H

#include <sdafx.h>

#include "Register.h"

namespace rlang::alvm {
    enum class OpCode : std::int16_t
    {
        End,
        Push,
        Pop,
        Add,
        Sub,
        Mul,
        Div,
        PrintInt,
        PrintStr,
        CmpIntLT,
        CmpIntET,
        Mov,
        Jump,
        CJump,
        CNJump,
        Db,
        Call,
        Return,

        Nop
    };

    struct Instruction
    {
        OpCode opcode = OpCode::Nop;
        std::int32_t imm32 = 0;
        Register reg1 = { RegType::Nul, false };
        Register reg2 = { RegType::Nul, false };
        std::int8_t size = 32; 
        std::vector<std::int8_t> bytes;

    public:
        inline static const std::vector<std::string> InstructionStr =
        {
            "End",
            "Push",
            "Pop",
            "Add",
            "Sub",
            "Mul",
            "Div",
            "PInt",
            "PStr",
            "CIGT",
            "CIET",
            "Mov",
            "Jump",
            "CJmp",
            "CNJp",
            "Db",
            "Call",
            "Ret",
            "Nop"
        };
    };
}

#endif // ALVM_ALVM_INSTRUCTION_H
