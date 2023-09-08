#include "Runtime.h"
#include "Instruction.h"

#define ResetFlags()							\
	m_Registers[RegType::SFR] = 0

#define SetZF()									\
	m_Registers[RegType::SFR] |= 0x01
#define SetSF()									\
	m_Registers[RegType::SFR] |= 0x02
#define SetOF()									\
	m_Registers[RegType::SFR] |= 0x04
#define SetCF()									\
	m_Registers[RegType::SFR] |= 0x08
#define ResetZF()								\
	m_Registers[RegType::SFR] &= ~0x01
#define ResetSF()								\
	m_Registers[RegType::SFR] &= ~0x02
#define ResetOF()								\
	m_Registers[RegType::SFR] &= ~0x04
#define ResetCF()								\
	m_Registers[RegType::SFR] &= ~0x8
#define GetZF()									\
	(m_Registers[RegType::SFR] & 0x01)
#define GetSF()									\
	(m_Registers[RegType::SFR] & 0x02)
#define GetOF()									\
	(m_Registers[RegType::SFR] & 0x04)
#define GetCF()									\
	(m_Registers[RegType::SFR] & 0x08)
#define ResetSFR()								\
	m_Registers[RegType::SFR] = 0x0

		/*m_Registers[RegType::PF] = [](std::uint8_t num) -> std::uint8_t	\
		{                                                                   \
			auto n = 0;                                                     \
			while (num)                                                     \
			{                                                               \
				n += num & 1;                                               \
				num >>= 1;                                                  \
			}                                                               \
			return n;                                                       \
		} (res & 0xff) % 2 == 0;*/
#define TriggerFlags(op1, op2, res, type, optype)	                                                                  \
    do                                                                                                                \
    { 														                                                          \
        const type msb = 1ull << (sizeof(type) * 8 - 1);                                                              \
        if (res == 0) SetZF(); else ResetZF();                                                                         \
        if (res & msb) SetSF(); else ResetSF();                                                                       \
        if ((optype) ? res < op1 || res < op2 : res > op1 || res > op2) SetCF(); else ResetCF();                      \
        if ((optype) ? ((op1 ^ res) & (op2 ^ res) & msb) : ((op1 ^ res) & (op2 ^ res) & msb)) SetOF(); else ResetOF(); \
    } while (0)


#define Push8(u8)								\
	m_Sp--;										\
	*(std::uint8_t*)m_Sp = (std::uint8_t)(u8)
#define Push16(u16)								\
	m_Sp -= 2;									\
	*(std::uint16_t*)m_Sp = (std::uint16_t)(u16)
#define Push32(u32)								\
 	m_Sp -= 4;									\
	*(std::uint32_t*)m_Sp = (std::uint32_t)(u32)
#define Push64(u64)	\
	m_Sp -= 8; 	\
	*(std::uint64_t*)m_Sp = (std::uint64_t)(u64)
#define Pop8(u8)								\
	u8 = *(std::uint8_t*)m_Sp;					\
	m_Sp++
#define Pop16(u16)								\
	u16 = *(std::uint16_t*)m_Sp;				\
	m_Sp += 2
#define Pop32(u32)								\
	u32 = *(std::uint32_t*)m_Sp;				\
	m_Sp += 4
#define Pop64(u64)	\
	u64 = *(std::uint64_t*)m_Sp; \
	m_Sp += 8
#define Pop8s()									\
	m_Sp++
#define Pop16s()								\
	m_Sp += 2
#define Pop32s()								\
	m_Sp += 4
#define Pop64s()								\
	m_Sp += 8
#define ReadFrom(addr)							\
	*((std::uint8_t*)(addr))
#define ReadFrom16(addr)						\
	*((std::uint16_t*)(addr))
#define ReadFrom32(addr)						\
	*((std::uint32_t*)(addr))
#define ReadFrom64(addr)						\
	*((std::uint64_t*)(addr))
#define WriteAt(addr, u8)						\
	*((std::uint8_t*)(addr)) = (std::uint8_t)(u8)
#define WriteAt16(addr, u16)						\
	*((std::uint16_t*)(addr)) = (std::uint16_t)(u16)
#define WriteAt32(addr, u32)						\
	*((std::uint32_t*)(addr)) = (std::uint32_t)(u32)
#define WriteAt64(addr, u64)					\
	*((std::uint64_t*)(addr)) = (std::uint64_t)(u64)

#define RegDeref(reg)							\
	reg & RegType::DPTR

namespace rlang::alvm {
	ALVM::ALVM(const std::vector<std::uint8_t>& data, const std::size_t bssSize)
		: m_Stack(data), m_Sp(m_Registers[RegType::SP]), m_BssSize(bssSize)
	{
		m_Stack.resize(m_Stack.size() + STACK_SIZE + m_BssSize);

		// Init registers
		m_Registers[RegType::SS] = (std::uintptr_t)m_Stack.data() + data.size();
		m_Registers[RegType::SP] = m_Registers[RegType::SS] + STACK_SIZE;
		m_Registers[RegType::DS] = (std::uintptr_t)m_Stack.data();
	}

	void ALVM::Run(const std::vector<Instruction>& code, std::int64_t& result)
	{
		m_Bytecode = ((std::vector<Instruction>&)code).data();
		m_Pc = m_Bytecode;

		m_Registers[RegType::CS] = (std::uintptr_t)m_Bytecode;

		while (m_Pc)
			(this->*m_Instructions[(std::size_t)m_Pc->opcode])();

		result = m_Registers[RegType::R0];
	}

	void ALVM::Nop()
	{
		m_Pc++;
	}

	void ALVM::End()
	{
		m_Pc = nullptr;
	}

	void ALVM::Push()
	{
		switch (m_Pc->size)
		{
			case 8:
			{
				if (m_Pc->sreg != RegType::NUL)
				{
					Push8((std::uint8_t)m_Registers[m_Pc->sreg]);
				}
				else
				{
					Push8((std::uint8_t)m_Pc->imm64);
				}
				break;
			}
			case 16:
			{
				if (m_Pc->sreg != RegType::NUL)
				{
					Push16((std::uint16_t)m_Registers[m_Pc->sreg]);
				}
				else
				{
					Push16((std::uint16_t)m_Pc->imm64);
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

	void ALVM::Pop()
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

	void ALVM::Add()
    {
        std::uint64_t op1, op2, res;
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
        TriggerFlags(op1, op2, res, std::int32_t, 1);
        m_Pc++;
    }

    void ALVM::Sub()
    {
        std::uint64_t op1, op2, res;
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
        TriggerFlags(op1, op2, res, std::int32_t, 1);
        m_Pc++;
    }

    void ALVM::Mul()
	{
		// r0, r
		std::uint64_t op1 = m_Registers[m_Pc->dreg], op2 = m_Registers[m_Pc->sreg], res = op1 * op2;
		m_Registers[m_Pc->dreg] = res;
		TriggerFlags(op1, op2, res, std::int32_t, 1);
		m_Pc++;
	}

	void ALVM::Div()
	{
		// r0, r
		std::uint64_t op1 = m_Registers[RegType::R0], op2 = m_Registers[m_Pc->sreg], res = op1 / op2;
		m_Registers[RegType::R0] = res;
		m_Registers[RegType::R3] = op1 % op2;
		TriggerFlags(op1, op2, res, std::int32_t, 0);
		m_Pc++;
	}

	void ALVM::Neg()
	{
		m_Registers[m_Pc->sreg] *= -1;
		m_Pc++;
	}

	void ALVM::Increment()
	{
		// r
		std::uint64_t op1 = m_Registers[m_Pc->sreg], res = ++op1;
		TriggerFlags(op1, 1, res, std::int64_t, 1);
		m_Registers[m_Pc->sreg] = res;
		m_Pc++;
	}

	void ALVM::Decrement()
	{
		// r
		std::uint64_t op1 = m_Registers[m_Pc->sreg], res = --op1;
		TriggerFlags(op1, 1, res, std::int64_t, 0);
		m_Registers[m_Pc->sreg] = res;
		m_Pc++;
	}

	void ALVM::Printf()
	{
		// m, m
		// sfmt_ptr, args_ptr
		std::size_t total_size = 0;
		std::size_t last_occurence = 0;
		const std::size_t size = std::strlen((const char*)m_Registers[m_Pc->sreg] + m_Pc->displacement + m_Registers[m_Pc->src_reg]) + 1;
		const unsigned char* format_str = (const unsigned char*)m_Registers[m_Pc->sreg] + m_Pc->displacement + m_Registers[m_Pc->src_reg];

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
					//char* before_f = (char*)std::malloc(i - last_occurence + 1);
					//std::memset(before_f, 0, i - last_occurence + 1);
					//std::memcpy(before_f, format_str + last_occurence, i - last_occurence);
					//last_occurence = i + riter;
					//std::printf("%s", before_f);

					// Substring that shite
					char f[12];
					std::memset(f, 0, citer + riter + 1);
					std::memcpy(f, format_str + citer, riter);

					// Now compare and print relative to the fomrat specifier
					// TODO: Optimize!
					if (std::strcmp(f, "d") == 0)
					{
						std::printf("%d",
									*((std::int32_t*)(m_Registers[m_Pc->dreg] + total_size)));
						total_size += 4;
					}
					else if (std::strcmp(f, "u") == 0)
					{
						std::printf("%u",
									*((std::uint32_t*)(m_Registers[m_Pc->dreg] + total_size)));
						total_size += 4;
					}
					else if (std::strcmp(f, "lu") == 0)
					{
						std::printf("%lu",
									*((std::uint64_t*)(m_Registers[m_Pc->dreg] + total_size)));
						total_size += 8;
					}
					else if (std::strcmp(f, "ld") == 0)
					{
						std::printf("%ld",
									*((std::int64_t*)(m_Registers[m_Pc->dreg] + total_size)));
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
						std::printf("%u", *((std::int8_t*)(m_Registers[m_Pc->dreg] + total_size)));
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

	void ALVM::PrintInt()
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

	void ALVM::PrintStr()
	{
		std::printf("%s", (const char*)m_Registers[m_Pc->sreg]);
		m_Pc++;
	}

	void ALVM::PrintChar()
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

    void ALVM::Compare()
    {
		std::uint64_t op1, op2, res;
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
        TriggerFlags(op1, op2, res, std::int64_t, 0);
        m_Pc++;
    }

	void ALVM::Move()
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

	void ALVM::Lea()
	{
		// r, m
		m_Registers[m_Pc->dreg] = m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg];
		m_Pc++;
	}

	void ALVM::Jump()
	{
		if (m_Pc->sreg != RegType::NUL) m_Pc = m_Bytecode + m_Registers[m_Pc->sreg];
		else m_Pc = m_Bytecode + m_Pc->imm64;
	}

	void ALVM::Enter()
	{
		Push64(m_Registers[RegType::BP]);
		m_Registers[RegType::BP] = m_Registers[RegType::SP];
		m_Registers[RegType::SP] -= m_Pc->imm64;
		m_Pc++;
	}

	void ALVM::Call()
	{
		Push64((std::uintptr_t)(m_Pc + 1));
		Jump();
	}

	void ALVM::Return()
	{
		Pop64(std::uintptr_t addr);
		m_Pc = (Instruction*)addr;
	}

	void ALVM::Leave()
	{
		m_Registers[RegType::SP] = m_Registers[RegType::BP];
		Pop64(m_Registers[RegType::BP]);
		m_Pc++;
	}

	void ALVM::Malloc()
	{
		// imm64, reg
		if (m_Pc->sreg != RegType::NUL)
		{
			m_Registers[RegType::R0] = (std::uintptr_t)std::malloc((std::size_t)m_Registers[m_Pc->dreg]);
		}
		else
		{
			m_Registers[RegType::R0] = (std::uintptr_t)std::malloc((std::size_t)m_Pc->imm64);
		}
		m_Pc++;
	}

	void ALVM::Free()
	{
		// So dodgy...
		std::free((void*)m_Registers[m_Pc->sreg]);
		m_Pc++;
	}

	void ALVM::Memset()
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

	void ALVM::Memcpy()
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

	void ALVM::Lrzf()
	{
		m_Registers[RegType::R0] = m_Registers[RegType::SFR];
		m_Pc++;
	}

	void ALVM::Srzf()
	{
		m_Registers[RegType::SFR] = m_Registers[RegType::R0];
		m_Pc++;
	}

	void ALVM::Store()
	{
		// imm32 | reg, m
		if (m_Pc->sreg != RegType::NUL)
		{
			switch (m_Pc->size)
			{
				case 8:
					WriteAt(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg], m_Registers[m_Pc->sreg]);
					break;
				case 16:
					WriteAt16(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg], m_Registers[m_Pc->sreg]);
					break;
				case 32:
					WriteAt32(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg], m_Registers[m_Pc->sreg]);
					break;
				default:
				case 64:
					WriteAt64(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg], m_Registers[m_Pc->sreg]);
					break;
			}
		}
		else
		{
			switch (m_Pc->size)
			{
				case 8:
					WriteAt(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg], m_Pc->imm64);
					break;
				case 16:
					WriteAt16(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg], m_Pc->imm64);
					break;
				case 32:
					WriteAt32(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg], m_Pc->imm64);
					break;
				default:
				case 64:
					WriteAt64(m_Registers[RegDeref(m_Pc->dreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg], m_Pc->imm64);
					break;
			}
		}
		m_Pc++;
	}

	void ALVM::Load()
	{
		// r, m
		switch (m_Pc->size)
		{
			case 8:
				m_Registers[m_Pc->dreg] = ReadFrom(m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg]);
				break;
			case 16:
				m_Registers[m_Pc->dreg] = ReadFrom16(m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg]);
				break;
			case 32:
				m_Registers[m_Pc->dreg] = ReadFrom32(m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg]);
				break;
			default:
			case 64:
                m_Registers[m_Pc->dreg] = ReadFrom64(m_Registers[RegDeref(m_Pc->sreg)] + m_Pc->displacement + m_Registers[m_Pc->src_reg]);
                break;
        }
		m_Pc++;
	}

	void ALVM::System()
	{
		auto status = std::system((const char*)m_Registers[m_Pc->sreg]);
		m_Registers[RegType::R4] = status;
		m_Pc++;
	}

	void ALVM::Syscall()
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

	void ALVM::InvokeC()
	{
		m_Pc++;
	}

	void ALVM::GetChar()
	{
		m_Registers[m_Pc->sreg] = (std::uintptr_t)std::getchar();
		m_Pc++;
	}

	void ALVM::JmpIfZero()
	{
		if (GetZF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfNotZero()
	{
		if (!GetZF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfSign()
	{
		if (GetSF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfNotSign()
	{
		if (!GetSF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfOverflow()
	{
		if (GetOF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfNotOverflow()
	{
		if (!GetOF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfCarry()
	{
		if (GetCF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfNotCarry()
	{
		if (!GetCF())
			Jump();
		else m_Pc++;
	}

	// TODO: Shove all flags into a single registers, please.

	void ALVM::JmpIfUnsignedGreaterOrEqualTo()
	{
		if (!GetCF() || GetZF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfUnsignedLesserOrEqualTo()
	{
		if (GetCF() || GetZF())
			Jump();
		else m_Pc++;
	}

	void ALVM::JmpIfSignedLessThan()
	{
		if (GetSF() != GetOF())
			Jump();
		else m_Pc++;
	}

	void ALVM::BitwiseAND()
	{
		if (m_Pc->dreg != RegType::NUL)
		{
			m_Registers[m_Pc->dreg] &= m_Registers[m_Pc->sreg];
			if (m_Registers[m_Pc->dreg] == 0) SetZF(); else ResetZF();
			if ((m_Registers[m_Pc->dreg] >> 64 - 1) & 1) SetSF(); else ResetSF();
		}
		else
		{
			m_Registers[m_Pc->dreg] &= m_Pc->imm64;
			if (m_Registers[m_Pc->dreg] == 0) SetZF(); else ResetZF();
			if ((m_Registers[m_Pc->dreg] >> 64 - 1) & 1) SetSF(); else ResetSF();
		}
		ResetOF();
		ResetCF();
		m_Pc++;
	}

	void ALVM::BitwsieOR()
	{
		if (m_Pc->dreg != RegType::NUL)
		{
			m_Registers[m_Pc->dreg] |= m_Registers[m_Pc->sreg];
			if (m_Registers[m_Pc->dreg] == 0) SetZF(); else ResetZF();
			if ((m_Registers[m_Pc->dreg] >> 64 - 1) & 1) SetSF(); else ResetSF();
		}
		else
		{
			m_Registers[m_Pc->dreg] |= m_Pc->imm64;
			if (m_Registers[m_Pc->dreg] == 0) SetZF(); else ResetZF();
			if ((m_Registers[m_Pc->dreg] >> 64 - 1) & 1) SetSF(); else ResetSF();
		}
		ResetOF();
		ResetCF();
		m_Pc++;
	}

	void ALVM::BitwiseNOT()
	{
		m_Registers[m_Pc->sreg] = ~m_Registers[m_Pc->sreg];
		m_Pc++;
	}

	void ALVM::BitwiseXOR()
	{
		if (m_Pc->dreg != RegType::NUL)
		{
			m_Registers[m_Pc->dreg] ^= m_Registers[m_Pc->sreg];
			if (m_Registers[m_Pc->dreg] == 0) SetZF(); else ResetZF();
			if ((m_Registers[m_Pc->dreg] >> 64 - 1) & 1) SetSF(); else ResetSF();
		}
		else
		{
			m_Registers[m_Pc->dreg] ^= m_Pc->imm64;
			if (m_Registers[m_Pc->dreg] == 0) SetZF(); else ResetZF();
			if ((m_Registers[m_Pc->dreg] >> 64 - 1) & 1) SetSF(); else ResetSF();
		}
		ResetOF();
		ResetCF();
		m_Pc++;
	}

	void ALVM::BitwiseTEST()
	{
		std::uintptr_t res;
		if (m_Pc->dreg != RegType::NUL)
		{
			res = m_Registers[m_Pc->dreg] & m_Registers[m_Pc->sreg];
			if (res == 0) SetZF(); else ResetZF();
			if ((res >> 64 - 1) & 1) SetSF(); else ResetSF();
		}
		else
		{
			res = m_Registers[m_Pc->dreg] & m_Pc->imm64;
			if (res == 0) SetZF(); else ResetZF();
			if ((res >> 64 - 1) & 1) SetSF(); else ResetSF();
		}
		ResetOF();
		ResetCF();
		m_Pc++;
	}

	void ALVM::PushAllRegisters()
	{
		for (std::uint8_t i = (std::uint8_t)RegType::R0; i <= (std::uint8_t)RegType::R31; ++i)
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

	void ALVM::PopAllRegisters()
	{
		for (std::uint8_t i = (std::uint8_t)RegType::R31; i != 255; --i)
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

	void ALVM::SetConioMode()
	{
		if (m_Pc->imm64)
		{
			utils::gterm::set_conio_terminal_mode();
		}
		else
		{
			utils::gterm::reset_terminal_mode();
		}
		m_Pc++;
	}

	void ALVM::Debug_DumpFlags()
	{
		std::cout << "---------- ART_DBG ----------\n";
		std::cout << "Zero Flag: " << GetZF() << std::endl;
		std::cout << "Carry Flag: " << GetCF() << std::endl;
		std::cout << "Sign Flag: " << GetSF() << std::endl;
		std::cout << "Overflow Flag: " << GetOF() << std::endl;
		std::cout << "-----------------------------\n";
		m_Pc++;
	}
} // namespace rlang::alvm
