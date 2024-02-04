#ifndef BLEND_REGISTER_H
#define BLEND_REGISTER_H

#include <sdafx.h>

namespace relang::blend {
    struct RegType
    {
        enum : u8 {
            // General purpose
            R0,
            R1,
            R2,
            R3,
            R4,
            R5,
            R6,
            R7,
            R8,
            R9,
            R10,
            R11,
            R12,
            R13,
            R14,
            R15,
            R16,
            R17,
            R18,
            R19,
            R20,
            R21,
            R22,
            R23,
            R24,
            R25,
            R26,
            R27,
            R28,
            R29,
            R30,
            R31,

            // Reserved
            BP,
            SP,

            // Status Flags Register
            SFR,

            // Segement registers
            CS,
            SS,
            DS,
            ES,
            FS,
            GS,

            // Special, Not real regisers
            // No register
            NUL,
            // Indicates if a registers is a pointer
            PTR = 0x80,
            // Inverted PTR for dereferencing and recovering the original register using bitwise AND
            DPTR = 0x7F
        };

    private:
        u8 m_Enum = NUL;

    public:
        constexpr RegType(const u8 type) noexcept
            : m_Enum(type){}
    
    public:
        constexpr RegType& operator=(const u8 type) noexcept
        {
            m_Enum = type;
            return *this;
        }
        constexpr operator u8() const noexcept
        {
            return m_Enum;
        }
    };

    inline RegType MemReg(const RegType reg) noexcept
    {
        return reg | RegType::PTR;
    }

    struct Register
    {
        RegType type = RegType::NUL;
        bool ptr = false;
        i64 disp = 0;

    public:
        inline static const std::vector<std::string> RegisterStr =
            {
                "r0",
                "r1",
                "r2",
                "r3",
                "r4",
                "r5",
                "r6",
                "r7",
                "r8",
                "r9",
                "r10",
                "r11",
                "r12",
                "r13",
                "r14",
                "r15",
                "r16",
                "r17",
                "r18",
                "r19",
                "r20",
                "r21",
                "r22",
                "r23",
                "r24",
                "r25",
                "r26",
                "r27",
                "r28",
                "r29",
                "r30",
                "r31",

                "bp",
                "sp",

                "sfr",

                "cs",
                "ss",
                "ds",
                "es",
                "fs",
                "gs",

                "nul"};

    public:
        constexpr operator RegType() const noexcept
        {
            return type;
        }
    };

    struct Registers
    {
    private:
        std::array<uintptr, (usize)RegType::NUL + 1> m_Buffer = {0};

    public:
        inline uintptr& operator[](const usize index)
        {
            return m_Buffer[index];
        }
    };
} // namespace relang::blend

#endif // BLEND_REGISTER_H
