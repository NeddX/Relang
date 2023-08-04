#ifndef ALVM_INSTRUCTION_H
#define ALVM_INSTRUCTION_H

#include <sdafx.h>

#include "Register.h"

namespace rlang::alvm {
    enum class OpCode : std::uint16_t
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

        DumpFlags,
        Nop
    };

    struct Instruction
    {
        OpCode opcode = OpCode::Nop;
        std::uint64_t imm64 = 0;
        Register reg1 = { RegType::Nul, false };
        Register reg2 = { RegType::Nul, false };
        std::int8_t size = 64;
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
            "neg",
            "inc",
            "dec",
            "printf",
            "pint",
            "pstr",
            "cmp",
            "mov",
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
            "store",
            "load",
            "system",
            "syscall",
            "invokec",
            "getch",
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
            "_dbg_dumpflags",
            "nop"
        };
    };
}

#endif // ALVM_INSTRUCTION_H
