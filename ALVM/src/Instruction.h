#ifndef ALVM_INSTRUCTION_H
#define ALVM_INSTRUCTION_H

#include <sdafx.h>

#include "Register.h"

namespace rlang::alvm {
    enum OpCode : std::uint8_t
    {
        End,
        Push,
        Pop,
        Add,
        Sub,
        Mul,
        Div,
        Neg,
        Inc,
        Dec,
        Printf,
        PInt,
        PStr,
        Cmp,
        Mov,
        Lea,
        Enter,
        Call,
        Return,
        Leave,
        Malloc,
        Free,
        Memset,
        Memcpy,
        Lrzf,
        Srzf,
        Store,
        Load,
        System,
        Syscall,
        InvokeC,
        GetChar,
        Jump,
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
        Jue,
        June,
        Juge,
        Jule,

        Jl,

        AND,
        OR,
        NOT,
        XOR,
        TEST,

        SConio,
        DumpFlags,
        Nop
    };

    struct Instruction
    {
        // 1 + 8 + 1 + 1 + 4 + 1 + 1 = 17
        OpCode opcode = OpCode::Nop;
        std::uint64_t imm64 = 0;
        RegType sreg = RegType::NUL;
        RegType dreg = RegType::NUL;
        std::int32_t displacement = 0;
        RegType src_reg = RegType::NUL;
        std::int8_t size = 64;

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
            "neg",
            "inc",
            "dec",
            "printf",
            "pint",
            "pstr",
            "cmp",
            "mov",
            "lea",
            "enter",
            "call",
            "ret",
            "leave",
            "malloc",
            "free",
            "memset",
            "memcpy",
            "lrzf",
            "srzf",
            "st",
            "ld",
            "system",
            "syscall",
            "invokec",
            "getchar",
            "jmp",
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
            "june",
            "juge",
            "jule",
            "jl",
            "and",
            "or",
            "not",
            "xor",
            "test",

            // Temporary instructions
            "sconio",
            "_dbg_dumpflags",
            "nop"
        };
    };

    using InstructionList = std::vector<Instruction>;
}

#endif // ALVM_INSTRUCTION_H
