#include "Runtime.h"
#include "Instruction.h"

using namespace relang;

#define ResetFlags() \
    m_Registers[RegType::SFR] = 0
#define SetZF() \
    m_Registers[RegType::SFR] |= 0x01
#define SetSF() \
    m_Registers[RegType::SFR] |= 0x02
#define SetOF() \
    m_Registers[RegType::SFR] |= 0x04
#define SetCF() \
    m_Registers[RegType::SFR] |= 0x08
#define ResetZF() \
    m_Registers[RegType::SFR] &= ~0x01
#define ResetSF() \
    m_Registers[RegType::SFR] &= ~0x02
#define ResetOF() \
    m_Registers[RegType::SFR] &= ~0x04
#define ResetCF() \
    m_Registers[RegType::SFR] &= ~0x8
#define GetZF() \
    (m_Registers[RegType::SFR] & 0x01)
#define GetSF() \
    (m_Registers[RegType::SFR] & 0x02)
#define GetOF() \
    (m_Registers[RegType::SFR] & 0x04)
#define GetCF() \
    (m_Registers[RegType::SFR] & 0x08)
#define ResetSFR() \
    m_Registers[RegType::SFR] = 0x0

/*m_Registers[RegType::PF] = [](u8 num) -> u8	\
{                                                                   \
    auto n = 0;                                                     \
    while (num)                                                     \
    {                                                               \
        n += num & 1;                                               \
        num >>= 1;                                                  \
    }                                                               \
    return n;                                                       \
} (res & 0xff) % 2 == 0;*/
#define TriggerFlags(op1, op2, res, type, optype)                                             \
    do                                                                                        \
    {                                                                                         \
        const type msb = 1ull << (sizeof(type) * 8 - 1);                                      \
        if (res == 0)                                                                         \
            SetZF();                                                                          \
        else                                                                                  \
            ResetZF();                                                                        \
        if (res & msb)                                                                        \
            SetSF();                                                                          \
        else                                                                                  \
            ResetSF();                                                                        \
        if ((optype) ? res < op1 || res < op2 : res > op1 || res > op2)                       \
            SetCF();                                                                          \
        else                                                                                  \
            ResetCF();                                                                        \
        if ((optype) ? ((op1 ^ res) & (op2 ^ res) & msb) : ((op1 ^ res) & (op2 ^ res) & msb)) \
            SetOF();                                                                          \
        else                                                                                  \
            ResetOF();                                                                        \
    } while (0)

#define Push8(uval8) \
    m_Sp--;       \
    *(u8*)m_Sp = (u8)(uval8)
#define Push16(uval16) \
    m_Sp -= 2;      \
    *(u16*)m_Sp = (u16)(uval16)
#define Push32(uval32) \
    m_Sp -= 4;      \
    *(u32*)m_Sp = (u32)(uval32)
#define Push64(uval64) \
    m_Sp -= 8;      \
    *(u64*)m_Sp = (u64)(uval64)
#define Pop8(uval8)               \
    uval8 = *(u8*)m_Sp; \
    m_Sp++
#define Pop16(uval16)               \
    uval16 = *(u16*)m_Sp; \
    m_Sp += 2
#define Pop32(uval32)               \
    uval32 = *(u32*)m_Sp; \
    m_Sp += 4
#define Pop64(uval64)               \
    uval64 = *(u64*)m_Sp; \
    m_Sp += 8
#define Pop8s() \
    m_Sp++
#define Pop16s() \
    m_Sp += 2
#define Pop32s() \
    m_Sp += 4
#define Pop64s() \
    m_Sp += 8
#define ReadFrom(addr) \
    *((u8*)(addr))
#define ReadFrom16(addr) \
    *((u16*)(addr))
#define ReadFrom32(addr) \
    *((u32*)(addr))
#define ReadFrom64(addr) \
    *((u64*)(addr))
#define WriteAt(addr, uval8) \
    *((u8*)(addr)) = (u8)(uval8)
#define WriteAt16(addr, uval16) \
    *((u16*)(addr)) = (u16)(uval16)
#define WriteAt32(addr, uval32) \
    *((u32*)(addr)) = (u32)(uval32)
#define WriteAt64(addr, uval64) \
    *((u64*)(addr)) = (u64)(uval64)

#define RegDeref(reg) \
    reg& RegType::DPTR

namespace relang::blend {
    Blend::Blend(const std::vector<u8>& data, const usize bssSize)
        : m_Stack(data), m_Sp(m_Registers[RegType::SP]), m_BssSize(bssSize)
    {
        m_Stack.resize(m_Stack.size() + STACK_SIZE + m_BssSize);

        // Init registers
        m_Registers[RegType::SS] = (uintptr)m_Stack.data() + data.size();
        m_Registers[RegType::SP] = m_Registers[RegType::SS] + STACK_SIZE;
        m_Registers[RegType::DS] = (uintptr)m_Stack.data();
    }

    void Blend::Run(const std::vector<Instruction>& code, i64& result)
    {
        m_Bytecode = ((std::vector<Instruction>&)code).data();
        m_Pc = m_Bytecode;

        m_Registers[RegType::CS] = (uintptr)m_Bytecode;

        while (m_Pc)
            (this->*m_Instructions[(usize)m_Pc->opcode])();

        result = m_Registers[RegType::R0];
    }

    void Blend::Nop()
    {
        m_Pc++;
    }

    void Blend::End()
    {
        m_Pc = nullptr;
    }

    void Blend::Push()
    {
        switch (m_Pc->size)
        {
            case 8:
            {
                if (m_Pc->sreg != RegType::NUL)
                {
                    Push8((u8)m_Registers[m_Pc->sreg]);
                }
                else
                {
                    Push8((u8)m_Pc->imm64);
                }
                break;
            }
            case 16:
            {
                if (m_Pc->sreg != RegType::NUL)
                {
                    Push16((u16)m_Registers[m_Pc->sreg]);
                }
                else
                {
                    Push16((u16)m_Pc->imm64);
                }
                break;
            }
            case 32:
            {
                if (m_Pc->sreg != RegType::NUL)
                {
                    Push32(m_Registers[m_Pc->sreg]);
                }
                else
                {
                    Push32(m_Pc->imm64);
                }
                break;
            }
            default:
            case 64:
            {
                if (m_Pc->sreg != RegType::NUL)
                {
                    Push64(m_Registers[m_Pc->sreg]);
                }
                else
                {
                    Push64(m_Pc->imm64);
                }
            }
        }
        m_Pc++;
    }

    void Blend::Pop()
    {
        switch (m_Pc->size)
        {
            case 8:
            {
                if (m_Pc->sreg != RegType::NUL)
                {
                    Pop8(m_Registers[m_Pc->sreg]);
                }
                else
                {
                    Pop8s();
                }
                break;
            }
            case 16:
            {
                if (m_Pc->sreg != RegType::NUL)
                {
                    Pop16(m_Registers[m_Pc->sreg]);
                }
                else
                {
                    Pop16s();
                }
                break;
            }
            case 32:
            {
                if (m_Pc->sreg != RegType::NUL)
                {
                    Pop32(m_Registers[m_Pc->sreg]);
                }
                else
                {
                    Pop32s();
                }
                break;
            }
            default:
            case 64:
            {
                if (m_Pc->sreg != RegType::NUL)
                {
                    Pop64(m_Registers[m_Pc->sreg]);
                }
                else
                {
                    Pop64s();
                }
            }
        }
        m_Pc++;
    }

    void Blend::Add()
    {
        u64 op1, op2, res;
        // ..., r
        if (m_Pc->dreg != RegType::NUL)
        {
            // r, r
            op1 = m_Registers[m_Pc->dreg];
            op2 = m_Registers[m_Pc->sreg];
            res = op1 + op2;
            m_Registers[m_Pc->dreg] = res;
        }
        else
        {
            // imm32, r
            op1 = m_Registers[m_Pc->dreg];
            op2 = m_Pc->imm64;
            res = op1 + op2;
            m_Registers[m_Pc->dreg] = res;
        }
        TriggerFlags(op1, op2, res, i32, 1);
        m_Pc++;
    }

    void Blend::Sub()
    {
        u64 op1, op2, res;
        // ..., r
        if (m_Pc->dreg != RegType::NUL)
        {
            // r, r
            op1 = m_Registers[m_Pc->dreg];
            op2 = m_Registers[m_Pc->sreg];
            res = op1 - op2;
            m_Registers[m_Pc->dreg] = res;
        }
        else
        {
            // imm32, r
            op1 = m_Registers[m_Pc->dreg];
            op2 = m_Pc->imm64;
            res = op1 - op2;
            m_Registers[m_Pc->dreg] = res;
        }
        TriggerFlags(op1, op2, res, i32, 1);
        m_Pc++;
    }

    void Blend::Mul()
    {
        // r0, r
        u64 op1 = m_Registers[m_Pc->dreg], op2 = m_Registers[m_Pc->sreg], res = op1 * op2;
        m_Registers[m_Pc->dreg] = res;
        TriggerFlags(op1, op2, res, i32, 1);
        m_Pc++;
    }

    void Blend::Div()
    {
        // r0, r
        u64 op1 = m_Registers[RegType::R0], op2 = m_Registers[m_Pc->sreg], res = op1 / op2;
        m_Registers[RegType::R0] = res;
        m_Registers[RegType::R3] = op1 % op2;
        TriggerFlags(op1, op2, res, i32, 0);
        m_Pc++;
    }

    void Blend::Neg()
    {
        m_Registers[m_Pc->sreg] *= -1;
        m_Pc++;
    }

    void Blend::Increment()
    {
        // r
        u64 op1 = m_Registers[m_Pc->sreg], res = ++op1;
        TriggerFlags(op1, 1, res, i64, 1);
        m_Registers[m_Pc->sreg] = res;
        m_Pc++;
    }

    void Blend::Decrement()
    {
        // r
        u64 op1 = m_Registers[m_Pc->sreg], res = --op1;
        TriggerFlags(op1, 1, res, i64, 0);
        m_Registers[m_Pc->sreg] = res;
        m_Pc++;
    }

    void Blend::Printf()
    {
        // m, m
        // sfmt_ptr, args_ptr
        usize total_size = 0;
        usize last_occurence = 0;
        const usize size = std::strlen((const char*)m_Registers[m_Pc->sreg] + m_Pc->disp + m_Registers[m_Pc->src_reg]) + 1;
        const unsigned char* format_str = (const unsigned char*)m_Registers[m_Pc->sreg] + m_Pc->disp + m_Registers[m_Pc->src_reg];

        for (auto i = 0; i < size; ++i)
        {
            switch (format_str[i])
            {
                case '%':
                {
                    auto citer = ++i;
                    auto riter = 0;

                    // Find end of the format specifier
                    for (auto x = i; x < size; ++x)
                    {
                        /*if (!((format_str[x] >= '0' && format_str[x] <= '9') ||
                            (format_str[x] >= 'A' && format_str[x] <= 'Z') ||
                            (format_str[x] >= 'a' && format_str[x] <= 'z')))
                        */
                        if (!std::isalnum(format_str[x]))
                        {
                            riter = x - citer;
                            break;
                        }
                    }

                    // Print all the characters before %
                    // char* before_f = (char*)std::malloc(i - last_occurence + 1);
                    // std::memset(before_f, 0, i - last_occurence + 1);
                    // std::memcpy(before_f, format_str + last_occurence, i - last_occurence);
                    // last_occurence = i + riter;
                    // std::printf("%s", before_f);

                    // Substring that shite
                    char f[12];
                    std::memset(f, 0, citer + riter + 1);
                    std::memcpy(f, format_str + citer, riter);

                    // Now compare and print relative to the fomrat specifier
                    // TODO: Optimize!
                    if (std::strcmp(f, "d") == 0)
                    {
                        std::printf("%d",
                                    *((i32*)(m_Registers[m_Pc->dreg] + total_size)));
                        total_size += 4;
                    }
                    else if (std::strcmp(f, "u") == 0)
                    {
                        std::printf("%u",
                                    *((u32*)(m_Registers[m_Pc->dreg] + total_size)));
                        total_size += 4;
                    }
                    else if (std::strcmp(f, "lu") == 0)
                    {
                        std::printf("%lu",
                                    *((u64*)(m_Registers[m_Pc->dreg] + total_size)));
                        total_size += 8;
                    }
                    else if (std::strcmp(f, "ld") == 0)
                    {
                        std::printf("%ld",
                                    *((i64*)(m_Registers[m_Pc->dreg] + total_size)));
                        total_size += 8;
                    }
                    else if (std::strcmp(f, "s") == 0)
                    {
                        std::printf("%s", (const char*)m_Registers[m_Pc->dreg] + total_size);
                    }
                    else if (std::strcmp(f, "c") == 0)
                    {
                        std::printf("%c", *((const char*)m_Registers[m_Pc->dreg] + total_size));
                    }
                    else if (std::strcmp(f, "b") == 0)
                    {
                        std::printf("%u", *((i8*)(m_Registers[m_Pc->dreg] + total_size)));
                    }
                    else
                    {
                        // Not a real format specifier so treat it just like a regular string
                        std::puts(f);
                    }
                    i += riter - 1;
                    break;
                }
                default:
                    std::putchar(format_str[i]);
                    break;
            }
        }
        m_Pc++;
    }

    void Blend::PrintInt()
    {
        if (m_Pc->sreg != RegType::NUL)
        {
            std::printf("%lu", m_Registers[m_Pc->sreg]);
        }
        else
        {
            std::printf("%lu", m_Pc->imm64);
        }
        m_Pc++;
    }

    void Blend::PrintStr()
    {
        std::printf("%s", (const char*)m_Registers[m_Pc->sreg]);
        m_Pc++;
    }

    void Blend::PrintChar()
    {
        if (m_Pc->sreg != RegType::NUL)
        {
            std::putchar(*(const char*)m_Registers[m_Pc->sreg]);
        }
        else
        {
            std::putchar(m_Pc->imm64);
        }
        m_Pc++;
    }

    void Blend::Compare()
    {
        u64 op1, op2, res;
        // r, ...
        op2 = m_Registers[m_Pc->sreg];
        if (m_Pc->dreg != RegType::NUL)
        {
            // r, r
            op1 = m_Registers[m_Pc->dreg];
        }
        else
        {
            // r, imm32
            op1 = m_Pc->imm64;
        }
        res = op1 - op2;
        TriggerFlags(op1, op2, res, i64, 0);
        m_Pc++;
    }

    void Blend::Move()
    {
        // r, ...
        if (m_Pc->dreg != RegType::NUL)
        {
            // r, r
            m_Registers[m_Pc->dreg] = m_Registers[m_Pc->sreg];
        }
        else
        {
            m_Registers[m_Pc->dreg] = m_Pc->imm64;
        }
        m_Pc++;
    }

    void Blend::Lea()
    {
        // r, m
        m_Registers[m_Pc->dreg] = m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg];
        m_Pc++;
    }

    void Blend::Jump()
    {
        if (m_Pc->sreg != RegType::NUL)
            m_Pc = m_Bytecode + m_Registers[m_Pc->sreg];
        else
            m_Pc = m_Bytecode + m_Pc->imm64;
    }

    void Blend::Enter()
    {
        Push64(m_Registers[RegType::BP]);
        m_Registers[RegType::BP] = m_Registers[RegType::SP];
        m_Registers[RegType::SP] -= m_Pc->imm64;
        m_Pc++;
    }

    void Blend::Call()
    {
        Push64((uintptr)(1 + m_Pc));
        Jump();
    }

    void Blend::Return()
    {
        Pop64(uintptr addr);
        m_Pc = (Instruction*)addr;
    }

    void Blend::Leave()
    {
        m_Registers[RegType::SP] = m_Registers[RegType::BP];
        Pop64(m_Registers[RegType::BP]);
        m_Pc++;
    }

    void Blend::Malloc()
    {
        // imm64, reg
        if (m_Pc->sreg != RegType::NUL)
        {
            m_Registers[RegType::R0] = (uintptr)std::malloc((usize)m_Registers[m_Pc->dreg]);
        }
        else
        {
            m_Registers[RegType::R0] = (uintptr)std::malloc((usize)m_Pc->imm64);
        }
        m_Pc++;
    }

    void Blend::Free()
    {
        // So dodgy...
        std::free((void*)m_Registers[m_Pc->sreg]);
        m_Pc++;
    }

    void Blend::Memset()
    {
        if (m_Pc->sreg != RegType::NUL)
        {
            std::memset((void*)m_Registers[m_Pc->dreg], m_Registers[RegType::R0], m_Registers[m_Pc->sreg]);
        }
        else
        {
            std::memset((void*)m_Registers[m_Pc->dreg], m_Registers[RegType::R0], m_Pc->imm64);
        }
        m_Pc++;
    }

    void Blend::Memcpy()
    {
        if (m_Pc->sreg != RegType::NUL)
        {
            std::memcpy((void*)m_Registers[m_Pc->dreg], (const void*)m_Registers[RegType::R0], m_Registers[m_Pc->sreg]);
        }
        else
        {
            std::memcpy((void*)m_Registers[m_Pc->dreg], (const void*)m_Registers[RegType::R0], m_Pc->imm64);
        }
        m_Pc++;
    }

    void Blend::Lrzf()
    {
        m_Registers[RegType::R0] = m_Registers[RegType::SFR];
        m_Pc++;
    }

    void Blend::Srzf()
    {
        m_Registers[RegType::SFR] = m_Registers[RegType::R0];
        m_Pc++;
    }

    void Blend::Store()
    {
        // imm32 | reg, m
        if (m_Pc->sreg != RegType::NUL)
        {
            switch (m_Pc->size)
            {
                case 8:
                    WriteAt(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg], m_Registers[m_Pc->sreg]);
                    break;
                case 16:
                    WriteAt16(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg], m_Registers[m_Pc->sreg]);
                    break;
                case 32:
                    WriteAt32(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg], m_Registers[m_Pc->sreg]);
                    break;
                default:
                case 64:
                    WriteAt64(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg], m_Registers[m_Pc->sreg]);
                    break;
            }
        }
        else
        {
            switch (m_Pc->size)
            {
                case 8:
                    WriteAt(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg], m_Pc->imm64);
                    break;
                case 16:
                    WriteAt16(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg], m_Pc->imm64);
                    break;
                case 32:
                    WriteAt32(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg], m_Pc->imm64);
                    break;
                default:
                case 64:
                    WriteAt64(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg], m_Pc->imm64);
                    break;
            }
        }
        m_Pc++;
    }

    void Blend::Load()
    {
        // r, m
        switch (m_Pc->size)
        {
            case 8:
                m_Registers[m_Pc->dreg] = ReadFrom(m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg]);
                break;
            case 16:
                m_Registers[m_Pc->dreg] = ReadFrom16(m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg]);
                break;
            case 32:
                m_Registers[m_Pc->dreg] = ReadFrom32(m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg]);
                break;
            default:
            case 64:
                m_Registers[m_Pc->dreg] = ReadFrom64(m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->disp + m_Registers[m_Pc->src_reg]);
                break;
        }
        m_Pc++;
    }

    void Blend::System()
    {
        auto status = std::system((const char*)m_Registers[m_Pc->sreg]);
        m_Registers[RegType::R4] = status;
        m_Pc++;
    }

    void Blend::Syscall()
    {
#if defined(__linux__)
        m_Registers[RegType::R0] = syscall(
            m_Registers[RegType::R0],
            m_Registers[RegType::R1],
            m_Registers[RegType::R2],
            m_Registers[RegType::R3],
            m_Registers[RegType::R4],
            m_Registers[RegType::R5],
            m_Registers[RegType::R6]);
#else
        std::printf("Runtime Error: System calls are not yet supported on your platform.");
        std::exit(-1);
#endif
        m_Pc++;
    }

    void Blend::InvokeC()
    {
        m_Pc++;
    }

    void Blend::GetChar()
    {
        m_Registers[m_Pc->sreg] = (uintptr)std::getchar();
        m_Pc++;
    }

    void Blend::JmpIfZero()
    {
        if (GetZF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfNotZero()
    {
        if (!GetZF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfSign()
    {
        if (GetSF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfNotSign()
    {
        if (!GetSF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfOverflow()
    {
        if (GetOF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfNotOverflow()
    {
        if (!GetOF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfCarry()
    {
        if (GetCF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfNotCarry()
    {
        if (!GetCF())
            Jump();
        else
            m_Pc++;
    }

    // TODO: Shove all flags into a single registers, please.

    void Blend::JmpIfUnsignedGreaterOrEqualTo()
    {
        if (!GetCF() || GetZF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfUnsignedLesserOrEqualTo()
    {
        if (GetCF() || GetZF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::JmpIfSignedLessThan()
    {
        if (GetSF() != GetOF())
            Jump();
        else
            m_Pc++;
    }

    void Blend::BitwiseAND()
    {
        if (m_Pc->dreg != RegType::NUL)
        {
            m_Registers[m_Pc->dreg] &= m_Registers[m_Pc->sreg];
            if (m_Registers[m_Pc->dreg] == 0)
                SetZF();
            else
                ResetZF();
            if ((m_Registers[m_Pc->dreg] >> (64 - 1)) & 1)
                SetSF();
            else
                ResetSF();
        }
        else
        {
            m_Registers[m_Pc->dreg] &= m_Pc->imm64;
            if (m_Registers[m_Pc->dreg] == 0)
                SetZF();
            else
                ResetZF();
            if ((m_Registers[m_Pc->dreg] >> (64 - 1)) & 1)
                SetSF();
            else
                ResetSF();
        }
        ResetOF();
        ResetCF();
        m_Pc++;
    }

    void Blend::BitwsieOR()
    {
        if (m_Pc->dreg != RegType::NUL)
        {
            m_Registers[m_Pc->dreg] |= m_Registers[m_Pc->sreg];
            if (m_Registers[m_Pc->dreg] == 0)
                SetZF();
            else
                ResetZF();
            if ((m_Registers[m_Pc->dreg] >> (64 - 1)) & 1)
                SetSF();
            else
                ResetSF();
        }
        else
        {
            m_Registers[m_Pc->dreg] |= m_Pc->imm64;
            if (m_Registers[m_Pc->dreg] == 0)
                SetZF();
            else
                ResetZF();
            if ((m_Registers[m_Pc->dreg] >> (64 - 1)) & 1)
                SetSF();
            else
                ResetSF();
        }
        ResetOF();
        ResetCF();
        m_Pc++;
    }

    void Blend::BitwiseNOT()
    {
        m_Registers[m_Pc->sreg] = ~m_Registers[m_Pc->sreg];
        m_Pc++;
    }

    void Blend::BitwiseXOR()
    {
        if (m_Pc->dreg != RegType::NUL)
        {
            m_Registers[m_Pc->dreg] ^= m_Registers[m_Pc->sreg];
            if (m_Registers[m_Pc->dreg] == 0)
                SetZF();
            else
                ResetZF();
            if ((m_Registers[m_Pc->dreg] >> (64 - 1)) & 1)
                SetSF();
            else
                ResetSF();
        }
        else
        {
            m_Registers[m_Pc->dreg] ^= m_Pc->imm64;
            if (m_Registers[m_Pc->dreg] == 0)
                SetZF();
            else
                ResetZF();
            if ((m_Registers[m_Pc->dreg] >> (64 - 1)) & 1)
                SetSF();
            else
                ResetSF();
        }
        ResetOF();
        ResetCF();
        m_Pc++;
    }

    void Blend::BitwiseTEST()
    {
        uintptr res;
        if (m_Pc->dreg != RegType::NUL)
        {
            res = m_Registers[m_Pc->dreg] & m_Registers[m_Pc->sreg];
            if (res == 0)
                SetZF();
            else
                ResetZF();
            if ((res >> (64 - 1)) & 1)
                SetSF();
            else
                ResetSF();
        }
        else
        {
            res = m_Registers[m_Pc->dreg] & m_Pc->imm64;
            if (res == 0)
                SetZF();
            else
                ResetZF();
            if ((res >> (64 - 1)) & 1)
                SetSF();
            else
                ResetSF();
        }
        ResetOF();
        ResetCF();
        m_Pc++;
    }

    void Blend::PushAllRegisters()
    {
        for (u8 i = (u8)RegType::R0; i <= (u8)RegType::R31; ++i)
        {
            switch (m_Pc->size)
            {
                case 8:
                    Push8(m_Registers[i]);
                    break;
                case 16:
                    Push16(m_Registers[i]);
                    break;
                case 32:
                    Push32(m_Registers[i]);
                    break;
                case 64:
                    Push64(m_Registers[i]);
                    break;
            }
        }
        m_Pc++;
    }

    void Blend::PopAllRegisters()
    {
        for (u8 i = (u8)RegType::R31; i != 255; --i)
        {
            switch (m_Pc->size)
            {
                case 8:
                    Pop8(m_Registers[i]);
                    break;
                case 16:
                    Pop16(m_Registers[i]);
                    break;
                case 32:
                    Pop32(m_Registers[i]);
                    break;
                case 64:
                    Pop64(m_Registers[i]);
                    break;
            }
        }
        m_Pc++;
    }

    void Blend::SetConioMode()
    {
        if (m_Pc->imm64)
        {
            //utils::gterm::set_conio_terminal_mode();
        }
        else
        {
            //utils::gterm::reset_terminal_mode();
        }
        m_Pc++;
    }

    void Blend::Debug_DumpFlags()
    {
        std::cout << "---------- ART_DBG ----------\n";
        std::cout << "Zero Flag: " << GetZF() << std::endl;
        std::cout << "Carry Flag: " << GetCF() << std::endl;
        std::cout << "Sign Flag: " << GetSF() << std::endl;
        std::cout << "Overflow Flag: " << GetOF() << std::endl;
        std::cout << "-----------------------------\n";
        m_Pc++;
    }
} // namespace relang::blend
