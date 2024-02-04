#ifndef BLEND_INSTRUCTION_H
#define BLEND_INSTRUCTION_H

#include <sdafx.h>

#include "Register.h"

namespace relang::blend {
    struct OpCode
    {
        enum : u8
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
            PChr,
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

            Pushar,
            Popar,

            SConio,
            DumpFlags,
            Nop
        };

    private:
        u8 m_Enum = Nop;

    public:
        constexpr OpCode(const u8 type) noexcept
            : m_Enum(type){};

    public:
        constexpr OpCode& operator=(const u8 type) noexcept
        {
            m_Enum = type;
            return *this;
        }
        constexpr operator u8() const noexcept
        {
            return m_Enum;
        }
    };
    

    struct Instruction
    {
        // 1 + 8 + 1 + 1 + 4 + 1 + 1 = 17
        OpCode opcode = OpCode::Nop;
        u64 imm64 = 0;
        RegType sreg = RegType::NUL;
        RegType dreg = RegType::NUL;
        i32 disp = 0;
        RegType src_reg = RegType::NUL;
        i8 size = 64;

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
                "pchr",
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
                "pushar",
                "popar",

                // Temporary instructions
                "sconio",
                "_dbg_dumpflags",
                "nop"};
    };

    using InstructionList = std::vector<Instruction>;
} // namespace relang::blend

#endif // BLEND_INSTRUCTION_H
