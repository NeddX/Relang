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
        System,
        Syscall,
        InvokeC,
        Jz,
        Jnz,
        Js,
        Jns,
        Jo,
        Jno,
        Jc,
        Jcn,

        Jug,
        Jul,
        Je,
        Juge,
        Jule,

        Jl,

        AND,
        OR,
        NOT,
        XOR,
        TEST,

        Nop
    };

    struct Instruction
    {
        OpCode opcode = OpCode::Nop;
        std::uint64_t imm64 = 0;
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
            "system",
            "syscall",
            "invokec",
            "jz",
            "jnz",
            "js",
            "jns",
            "jo",
            "jno",
            "jc",
            "jcn",
            "jug",
            "jul",
            "jue",
            "juge",
            "jule",
            "jl",
            "and",
            "or",
            "not",
            "xor",
            "test",
            "nop"
        };
    };
}

#endif // ALVM_INSTRUCTION_H
