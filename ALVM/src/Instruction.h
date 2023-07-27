#ifndef ALVM_INSTRUCTION_H
#define ALVM_INSTRUCTION_H

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
        Inc,
        Dec,
        PInt,
        PStr,
        Cmp,
        Mov,
        Jump,
        CJump,
        CNJump,
        Call,
        Return,
        Malloc,
        Free,
        Lrzf,
        Srzf,
        Store,
        Load,
        Jz,
        Jnz,
        Js,
        Jns,
        Jo,
        Jno,
        Jc,
        Jcn,

        Jl,

        Nop
    };

    struct Instruction
    {
        OpCode opcode = OpCode::Nop;
        std::uint32_t imm32 = 0;
        Register reg1 = { RegType::Nul, false };
        Register reg2 = { RegType::Nul, false };
        std::int8_t size = 32; 
        std::vector<std::int8_t> bytes;

    public:
        inline static const std::vector<std::string> InstructionStr =
        {
            "end",
            "push",
            "pop",
            "add",
            "sub",
            "mul",
            "div",
            "inc",
            "dec",
            "pint",
            "pstr",
            "cmp",
            "mov",
            "jmp",
            "cjmp",
            "cjp",
            "call",
            "ret",
            "malloc",
            "free",
            "lrzf",
            "srzf",
            "store",
            "load",
            "jz",
            "jnz",
            "js",
            "jns",
            "jo",
            "jno",
            "jc",
            "jcn",
            "jl",
            "nop"
        };
    };
}

#endif // ALVM_INSTRUCTION_H
