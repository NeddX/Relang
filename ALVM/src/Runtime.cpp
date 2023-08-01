#include "Runtime.h"
#include "Instruction.h"

#define GetRegVal(reg) m_Registers[reg.type]
/*#define TriggerFlags(op1, op2, res, bitsize, optype)					\
    m_Registers[RegType::ZF] = res == 0;								\
    m_Registers[RegType::CF] = (optype) ? res < op1 || res < op2 : res > op1 || res > op2; \
    m_Registers[RegType::SF] = (res >> bitsize - 1) & 1;				\
    m_Registers[RegType::PF] = [](std::uint8_t num) -> std::uint8_t		\
    {                                                               \
        auto n = 0;                                                 \
        while (num)                                                 \
        {                                                           \
            n += num & 1;                                           \
            num >>= 1;                                              \
        }                                                           \
        return n;                                                   \
	} (res & 0xff) % 2 == 0;

*/

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

#define TriggerFlags(op1, op2, res, bitsize, optype)	\
	if (res == 0) SetZF(); else ResetZF();									\
	if ((res >> sizeof(bitsize) - 1) & 1) SetSF(); else ResetSF();	\
	if ((optype) ? res < op1 || res < op2 : res > op1 || res > op2) SetCF(); else ResetCF(); \
	if ((optype) ? (bitsize)res < (bitsize)op1 || (bitsize)res < (bitsize)op2 : (bitsize)res > (bitsize)op1 || (bitsize)res > (bitsize)op2) SetCF(); else ResetCF() \

#define Push8(u8)								\
	m_Sp--;										\
	*(std::uint8_t*)m_Sp = (std::uint8_t)u8
#define Push16(u16)								\
	m_Sp -= 2;									\
	*(std::uint16_t*)m_Sp = (std::uint16_t)u16
#define Push32(u32)								\
 	m_Sp -= 4;									\
	*(std::uint32_t*)m_Sp = (std::uint32_t)u32
#define Push64(u64)	\
	m_Sp -= 8; 	\
	*(std::uint64_t*)m_Sp = (std::uint64_t)u64
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
	*((std::uint8_t*)addr)
#define ReadFrom16(addr)						\
	*((std::uint16_t*)addr)
#define ReadFrom32(addr)						\
	*((std::uint32_t*)addr)
#define ReadFrom64(addr)						\
	*((std::uint64_t*)addr)
#define WriteAt(addr, u8)						\
	*((std::uint8_t*)addr) = (std::uint8_t)u8
#define WriteAt16(addr, u16)						\
	*((std::uint16_t*)addr) = (std::uint16_t)u16
#define WriteAt32(addr, u32)						\
	*((std::uint32_t*)addr) = (std::uint32_t)u32
#define WriteAt64(addr, u64)					\
	*((std::uint64_t*)addr) = (std::uint64_t)u64

namespace rlang::alvm {
	ALVM::ALVM(const std::vector<std::uint8_t>& data)
		: m_Stack(data), m_Sp(m_Registers[RegType::SP])
	{
		m_Stack.resize(m_Stack.size() + STACK_SIZE);

		// Init registers
		m_Registers[RegType::SS] = (std::uintptr_t)m_Stack.data() + data.size();
		m_Registers[RegType::SP] = m_Registers[RegType::SS] + STACK_SIZE;
		m_Registers[RegType::DS] = (std::uintptr_t)m_Stack.data();
	}

	void ALVM::Run(const std::vector<Instruction>& code, std::int32_t& result)
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
				if (m_Pc->reg1.type != RegType::Nul)
				{
					Push8((std::uint8_t)m_Registers[m_Pc->reg1.type]);
				}
				else
				{
					Push8((std::uint8_t)m_Pc->imm64);
				}
				break;
			}
			case 16:
			{
				if (m_Pc->reg1.type != RegType::Nul)
				{
					Push16((std::uint16_t)m_Registers[m_Pc->reg1.type]);
				}
				else
				{
					Push16((std::uint16_t)m_Pc->imm64);
				}
				break;
			}
			case 32:
			{
				if (m_Pc->reg1.type != RegType::Nul)
				{
					Push32(m_Registers[m_Pc->reg1.type]);
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
				if (m_Pc->reg1.type != RegType::Nul)
				{
					Push64(m_Registers[m_Pc->reg1.type]);
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
				if (m_Pc->reg1.type != RegType::Nul)
				{
					Pop8(GetRegVal(m_Pc->reg1));
				}
				else
				{
					Pop8s();
				}
				break;
			}
			case 16:
			{
				if (m_Pc->reg1.type != RegType::Nul)
				{
					Pop16(GetRegVal(m_Pc->reg1));
				}
				else
				{
					Pop16s();
				}
				break;
			}        
			case 32:
			{
				if (m_Pc->reg1.type != RegType::Nul)
				{
					Pop32(GetRegVal(m_Pc->reg1));
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
				if (m_Pc->reg1.type != RegType::Nul)
				{
					Pop64(GetRegVal(m_Pc->reg1));
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
		std::uint32_t op1, op2, res;

		if (m_Pc->reg1.type != RegType::Nul)
		{
			if (m_Pc->reg1.ptr)
			{
				// m, ...
				op1 = ReadFrom64(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement);
				if (m_Pc->reg2.type == RegType::Nul)
				{
					// m, imm32
					op2 = m_Pc->imm64;
				}
				else
				{
					// m, r
					op2 = GetRegVal(m_Pc->reg2);
				}
				res = op1 + op2;
				WriteAt64(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, res);
			}
			else
			{
				// r, ...
				op1 = GetRegVal(m_Pc->reg1);
				if (m_Pc->reg2.type != RegType::Nul)
				{
					if (m_Pc->reg2.ptr)
					{
						// r, m
						op2 = ReadFrom64(GetRegVal(m_Pc->reg2) + m_Pc->reg2.displacement);
					}
					else
					{
						// r, r
						op2 = GetRegVal(m_Pc->reg2);
					}
				}
				else
				{
					// r, imm32
					op2 = m_Pc->imm64;
				}

				res = op1 + op2;
				GetRegVal(m_Pc->reg1) = res;
			}
		}
		else
		{
			// r0, imm32
			op1 = m_Registers[RegType::R0];
			op2 = m_Pc->imm64;
			res = op1 + op2;
			m_Registers[RegType::R0] = res;
		}

		TriggerFlags(op1, op2, res, std::int32_t, 1);
		m_Pc++;
	}

	void ALVM::Sub()
	{
		std::uint32_t op1, op2, res;

		if (m_Pc->reg1.type != RegType::Nul)
		{
			if (m_Pc->reg1.ptr)
			{
				// m, ...
				op1 = ReadFrom64(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement);
				if (m_Pc->reg2.type == RegType::Nul)
				{
					// m, imm32
					op2 = m_Pc->imm64;
				}
				else
				{
					// m, r
					op2 = GetRegVal(m_Pc->reg2);
				}
				res = op1 - op2;
				WriteAt64(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, res);
			}
			else
			{
				// r, ...
				op1 = GetRegVal(m_Pc->reg1);
				if (m_Pc->reg2.type != RegType::Nul)
				{
					if (m_Pc->reg2.ptr)
					{
						// r, m
						op2 = ReadFrom64(GetRegVal(m_Pc->reg2) + m_Pc->reg2.displacement);
					}
					else
					{
						// r, r
						op2 = GetRegVal(m_Pc->reg2);
					}
				}
				else
				{
					// r, imm32
					op2 = m_Pc->imm64;
				}

				res = op1 - op2;
				GetRegVal(m_Pc->reg1) = res;
			}
		}
		else
		{
			// r0, imm32
			op1 = m_Registers[RegType::R0];
			op2 = m_Pc->imm64;
			res = op1 - op2;
			m_Registers[RegType::R0] = res;
		}

		TriggerFlags(op1, op2, res, std::int32_t, 0);
		m_Pc++;
	}

	void ALVM::Mul()
	{
		std::uint32_t op1 = m_Registers[RegType::R0], op2;
		if (m_Pc->reg1.ptr)
		{
			// r0, m
			op2 = ReadFrom64(GetRegVal(m_Pc->reg1));
		}
		else
		{
			// r0, r
			op2 = GetRegVal(m_Pc->reg1);
		}
		m_Registers[RegType::R0] = op1 * op2;
		m_Pc++;
	}

	void ALVM::Div()
	{
		std::uint32_t op1 = m_Registers[RegType::R0], op2;
		if (m_Pc->reg1.ptr)
		{
			// r, m
			op2 = ReadFrom64(GetRegVal(m_Pc->reg1));
		}
		else
		{
			// r, r
			op2 = GetRegVal(m_Pc->reg1);
		}

		m_Registers[RegType::R0] = op1 / op2;
		m_Registers[RegType::R3] = op1 % op2;
		m_Pc++;
	}

	void ALVM::Increment()
	{
		if (m_Pc->reg1.ptr)
		{
			// m
			switch (m_Pc->size)
			{
				case 8:
				{
					std::uint8_t op1 = ReadFrom(GetRegVal(m_Pc->reg1)), res = ++op1;
					TriggerFlags(op1, 1, res, std::int8_t, 1);
					WriteAt(GetRegVal(m_Pc->reg1), res);
					break;
				}
				case 16:
				{
					std::uint16_t op1 = ReadFrom16(GetRegVal(m_Pc->reg1)), res = ++op1;
					TriggerFlags(op1, 1, res, std::int16_t, 1);
					WriteAt16(GetRegVal(m_Pc->reg1), res);
					break;
				}
				case 32:
				{
					std::uint32_t op1 = ReadFrom32(GetRegVal(m_Pc->reg1)), res = ++op1;
					TriggerFlags(op1, 1, res, std::int32_t, 1);
					WriteAt32(GetRegVal(m_Pc->reg1), res);
					break;
				}
				default:
				case 64:
				{
					std::uint64_t op1 = ReadFrom64(GetRegVal(m_Pc->reg1)), res = ++op1;
					TriggerFlags(op1, 1, res, std::int64_t, 1);
					WriteAt64(GetRegVal(m_Pc->reg1), res);
				}
			}
		}
		else
		{
			// r
			std::uint64_t op1 = GetRegVal(m_Pc->reg1), res = ++op1;
			TriggerFlags(op1, 1, res, std::int64_t, 1);
			GetRegVal(m_Pc->reg1) = res;
		}
		m_Pc++;
	}

	void ALVM::Decrement()
	{
		if (m_Pc->reg1.ptr)
		{
			// m
			switch (m_Pc->size)
			{
				case 8:
				{
					std::uint8_t op1 = ReadFrom(GetRegVal(m_Pc->reg1)), res = --op1;
					TriggerFlags(op1, 1, res, std::int8_t, 0);
					break;
				}
				case 16:
				{
					std::uint16_t op1 = ReadFrom16(GetRegVal(m_Pc->reg1)), res = --op1;
					TriggerFlags(op1, 1, res, std::int16_t, 0);
					break;
				}
				case 32:
				{
					std::uint32_t op1 = ReadFrom32(GetRegVal(m_Pc->reg1)), res = --op1;
					TriggerFlags(op1, 1, res, std::int32_t, 0);
					break;
				}
				default:
				case 64:
				{
					std::uint64_t op1 = ReadFrom64(GetRegVal(m_Pc->reg1)), res = --op1;
					TriggerFlags(op1, 1, res, std::int64_t, 0);
					break;
				}
			}
		}
		else
		{
			// r
			std::uint64_t op1 = GetRegVal(m_Pc->reg1), res = --op1;
			TriggerFlags(op1, 1, res, std::int64_t, 0);
		}
		m_Pc++;
	}

	void ALVM::PrintInt()
	{
		if (m_Pc->reg1.type != RegType::Nul)
		{
			if (m_Pc->reg1.ptr)
			{
				switch (m_Pc->size)
				{
					case 8:
					{
						std::printf("%u", ReadFrom((std::size_t)m_Registers[m_Pc->reg1.type] + m_Pc->reg1.displacement));
						break;
					}
					case 16:
					{
						std::printf("%u", ReadFrom16((std::size_t)m_Registers[m_Pc->reg1.type] + m_Pc->reg1.displacement));
						break;
					}
					case 32:
					{
						std::printf("%u", ReadFrom32((std::size_t)m_Registers[m_Pc->reg1.type] + m_Pc->reg1.displacement));
						break;
					}
					default:
					case 64:
					{
						std::printf("%lu", ReadFrom64((std::size_t)m_Registers[m_Pc->reg1.type] + m_Pc->reg1.displacement));
						break;
					}
				}
			}
			else
			{
				std::printf("%lu", m_Registers[m_Pc->reg1.type]);
			}
		}
		else
		{
			std::printf("%lu", m_Pc->imm64);
		}
		m_Pc++;
	}

	void ALVM::PrintStr()
	{
		// FIXME: Possibly unsafe.
		std::printf("%s", (const char*)m_Registers[m_Pc->reg1.type] + m_Pc->reg1.displacement);
		m_Pc++;
	}

    void ALVM::Compare()
    {
		std::uint64_t op1, op2, res;

		if (m_Pc->reg1.type != RegType::Nul)
		{
			if (m_Pc->reg1.ptr)
			{
				// m, ...
				op1 = ReadFrom32(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement);
				if (m_Pc->reg2.type == RegType::Nul)
				{
					// m, imm32
					op2 = m_Pc->imm64;
				}
				else
				{
					// m, r
					op2 = GetRegVal(m_Pc->reg2);
				}
				res = op1 - op2;
			}
			else
			{
				// r, ...
				op1 = GetRegVal(m_Pc->reg1);
				if (m_Pc->reg2.type != RegType::Nul)
				{
					if (m_Pc->reg2.ptr)
					{
						// r, m
						op2 = ReadFrom64(GetRegVal(m_Pc->reg2) + m_Pc->reg2.displacement);
					}
					else
					{
						// r, r
						op2 = GetRegVal(m_Pc->reg2);
					}
				}
				else
				{
					// r, imm32
					op2 = m_Pc->imm64;
				}
				res = op1 - op2;
			}
		}
		else
		{
			// r0, imm32
			op1 = m_Registers[RegType::R0];
			op2 = m_Pc->imm64;
			res = op1 - op2;
		}

		TriggerFlags(op1, op2, res, std::int64_t, 0);
        m_Pc++;
    }

	void ALVM::Move()
	{
		// r, imm64
		// r, r
		if (m_Pc->reg2.type != RegType::Nul)
		{
			GetRegVal(m_Pc->reg1) = GetRegVal(m_Pc->reg2);
		}
		else
		{
			GetRegVal(m_Pc->reg1) = m_Pc->imm64;
		}
		m_Pc++;
	}

	void ALVM::Jump()
	{
		if (m_Pc->reg1.type != RegType::Nul) m_Pc = m_Bytecode + m_Registers[m_Pc->reg1.type];
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
		// r0, imm32
		// r0, r
		if (m_Pc->reg1.type != RegType::Nul)
		{
			m_Registers[RegType::R0] = (std::uintptr_t)std::malloc((std::size_t)GetRegVal(m_Pc->reg1));
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
		std::free((void*)GetRegVal(m_Pc->reg1));
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
		// m, imm32 | reg
		if (m_Pc->reg2.type != RegType::Nul)
		{
			switch (m_Pc->size)
			{
				case 8:
					WriteAt(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, GetRegVal(m_Pc->reg2));
					break;
				case 16:
					WriteAt16(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, GetRegVal(m_Pc->reg2));
					break;
				case 32:
					WriteAt32(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, GetRegVal(m_Pc->reg2));
					break;
				default:
				case 64:
					WriteAt64(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, GetRegVal(m_Pc->reg2));
					break;
			}
		}
		else
		{
			switch (m_Pc->size)
			{
				case 8:
					WriteAt(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, m_Pc->imm64);
					break;
				case 16:
					WriteAt16(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, m_Pc->imm64);
					break;
				case 32:
					WriteAt32(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, m_Pc->imm64);
					break;
				default:
				case 64:
					WriteAt64(GetRegVal(m_Pc->reg1) + m_Pc->reg1.displacement, m_Pc->imm64);
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
				m_Registers[GetRegVal(m_Pc->reg1)] = ReadFrom(GetRegVal(m_Pc->reg2) + m_Pc->reg2.displacement);
				break;
			case 16:
				m_Registers[GetRegVal(m_Pc->reg1)] = ReadFrom16(GetRegVal(m_Pc->reg2) + m_Pc->reg2.displacement);
				break;
			case 32:
				m_Registers[GetRegVal(m_Pc->reg1)] = ReadFrom32(GetRegVal(m_Pc->reg2) + m_Pc->reg2.displacement);
				break;
			default:
			case 64:
				m_Registers[GetRegVal(m_Pc->reg1)] = ReadFrom64(GetRegVal(m_Pc->reg2) + m_Pc->reg2.displacement);
		}
		m_Pc++;
	}

	void ALVM::System()
	{
		auto status = std::system((const char*)m_Registers[m_Pc->reg1.type] + m_Pc->reg1.displacement);
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
		if (m_Pc->reg2.type != RegType::Nul)
		{
			GetRegVal(m_Pc->reg1) &= GetRegVal(m_Pc->reg2);
		}
		else
		{
			GetRegVal(m_Pc->reg1) &= m_Pc->imm64;
		}
		ResetOF();
		ResetCF();
		if (GetRegVal(m_Pc->reg1) == 0) SetZF(); else ResetZF();
		if ((GetRegVal(m_Pc->reg1) >> 64 - 1) & 1) SetSF(); else ResetSF();

		m_Pc++;
	}

	void ALVM::BitwsieOR()
	{
		if (m_Pc->reg2.type != RegType::Nul)
		{
			GetRegVal(m_Pc->reg1) |= GetRegVal(m_Pc->reg2);
		}
		else
		{
			GetRegVal(m_Pc->reg1) |= m_Pc->imm64;
		}
		ResetOF();
		ResetCF();
		if (GetRegVal(m_Pc->reg1) == 0) SetZF(); else ResetZF();
		if ((GetRegVal(m_Pc->reg1) >> 64 - 1) & 1) SetSF(); else ResetSF();

		m_Pc++;
	}

	void ALVM::BitwiseNOT()
	{
		GetRegVal(m_Pc->reg1) = ~GetRegVal(m_Pc->reg1);
		m_Pc++;
	}

	void ALVM::BitwiseXOR()
	{
		if (m_Pc->reg2.type != RegType::Nul)
		{
			GetRegVal(m_Pc->reg1) ^= GetRegVal(m_Pc->reg2);
		}
		else
		{
			GetRegVal(m_Pc->reg1) ^= m_Pc->imm64;
		}
		ResetOF();
		ResetCF();
		if (GetRegVal(m_Pc->reg1) == 0) SetZF(); else ResetZF();
		if ((GetRegVal(m_Pc->reg1) >> 64 - 1) & 1) SetSF(); else ResetSF();
		m_Pc++;
	}

	void ALVM::BitwiseTEST()
	{
		if (m_Pc->reg2.type != RegType::Nul)
		{
			GetRegVal(m_Pc->reg1) &= GetRegVal(m_Pc->reg2);
		}
		else
		{
			GetRegVal(m_Pc->reg1) &= m_Pc->imm64;
		}
		ResetOF();
		ResetCF();
		if (GetRegVal(m_Pc->reg1) == 0) SetZF(); else ResetZF();
		if ((GetRegVal(m_Pc->reg1) >> 64 - 1) & 1) SetSF(); else ResetSF();
		m_Pc++;
	}

	void ALVM::Debug_DumpFlags()
	{
		std::cout << "---------- ART_DBG ----------\n";
		std::cout << "Zero Flag: " << GetZF() << std::endl;
		std::cout << "Carry Flag: " << GetCF() << std::endl;
		std::cout << "Sign Flag: " << GetSF() << std::endl;
		std::cout << "Overflow Flag: " << GetOF() << std::endl;
		m_Pc++;
	}
} // namespace rlang::alvm
