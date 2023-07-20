#include "Runtime.h"

#define GetRegVal(reg) m_Registers[reg.type]

namespace rlang::alvm {
	void ALVM::Run(const std::vector<Instruction>& bytecode, std::int32_t& result)
	{
		m_Bytecode = const_cast<std::vector<Instruction>&>(bytecode).data();
		m_Pc = m_Bytecode;
		//m_Stack.Reserve(1024);
		//m_Stack.Push32(m_BaseIndex);
		//m_Stack.Push32(0);
		//m_BaseIndex = m_Stack.GetSize();
		//m_Registers[RegType::Sp] = m_Stack.GetSize();

		while (m_Pc != nullptr)
			(this->*m_Instructions[(std::size_t)m_Pc->opcode])();

		result = m_Registers[RegType::Fr];
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
				if (m_Pc->reg1.type != RegType::Nul) m_Stack.Push((std::int8_t)m_Registers[m_Pc->reg1.type]);
				else m_Stack.Push((std::int8_t)m_Pc->imm32);
				break;
			}
			case 16:
			{
				if (m_Pc->reg1.type != RegType::Nul) m_Stack.Push16((std::int16_t)m_Registers[m_Pc->reg1.type]);
				else m_Stack.Push16((std::int16_t)m_Pc->imm32);
				break;
			}
			case 32:
			default:
			{
				if (m_Pc->reg1.type != RegType::Nul) m_Stack.Push32(m_Registers[m_Pc->reg1.type]);
				else m_Stack.Push32(m_Pc->imm32);
				break;
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
				if (m_Pc->reg1.type != RegType::Nul) m_Registers[m_Pc->reg1.type] = (std::int32_t)m_Stack.Read();
				else m_Stack.Pop();
				break;
			}
			case 16:
			{
				if (m_Pc->reg1.type != RegType::Nul) m_Registers[m_Pc->reg1.type] = (std::int32_t)m_Stack.Read16();
				else m_Stack.Pop16();
				break;
			}
			case 32:
			default:
			{
				if (m_Pc->reg1.type != RegType::Nul) m_Registers[m_Pc->reg1.type] = m_Stack.Read32();
				else m_Stack.Pop32();
				break;
			}
		}
		m_Pc++;
	}

	void ALVM::Add()
	{
		std::int32_t rhv, lhv;
		rhv = (std::int32_t)m_Registers[m_Pc->reg1.type];
		if (m_Pc->reg2.type != RegType::Nul) lhv = m_Registers[m_Pc->reg2.type];
		else lhv = m_Pc->imm32;
		m_Registers[m_Pc->reg1.type] = rhv + lhv;
		m_Pc++;
	}

	void ALVM::Sub()
	{
		std::int32_t op1, op2;

		if (m_Pc->reg1.ptr)
		{
			// m, ...
			op1 = m_Stack.ReadFrom32(GetRegVal(m_Pc->reg1));
			if (m_Pc->reg2.type != RegType::Nul)
			{
				if (m_Pc->reg2.ptr)
				{
					// m, m
					op2 = m_Stack.ReadFrom32(GetRegVal(m_Pc->reg2));
				}
				else
				{
					// m, r
					op2 = GetRegVal(m_Pc->reg2);
				}
			}
			else
			{
				// m, imm32
				op2 = m_Pc->imm32;
			}

			m_Stack.WriteAt32(GetRegVal(m_Pc->reg1), op1 - op2);
		}
		else
		{
			// r, ...
			op1 = m_Registers[m_Pc->reg1.type];
			if (m_Pc->reg2.type != RegType::Nul)
			{
				if (m_Pc->reg2.ptr)
				{
					// r, m
					op2 = m_Stack.ReadFrom32(GetRegVal(m_Pc->reg2));
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
				op2 = m_Pc->imm32;
			}

			GetRegVal(m_Pc->reg1) = op1 - op2;
		}
		m_Pc++;
	}

	void ALVM::Mul()
	{
		std::int32_t op1 = m_Registers[RegType::R0], op2;
		if (m_Pc->reg1.ptr)
		{
			// r0, m
			op2 = m_Stack.ReadFrom32(GetRegVal(m_Pc->reg1));
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
		std::int32_t op1 = m_Registers[RegType::R0], op2;
		if (m_Pc->reg1.ptr)
		{
			// r, m
			op2 = m_Stack.ReadFrom32(GetRegVal(m_Pc->reg1));
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

	void ALVM::PrintInt()
	{
		if (m_Pc->reg1.ptr)
		{
			switch (m_Pc->size)
			{
				case 8:
				{
					//std::printf("%d", m_Stack.ReadFrom((std::size_t)m_Registers[m_Pc->reg1.type]));
					std::cout << m_Stack.ReadFrom((std::size_t)m_Registers[m_Pc->reg1.type]);
					break;
				}
				case 16:
				{
					//std::printf("%d", m_Stack.ReadFrom16((std::size_t)m_Registers[m_Pc->reg1.type]));
					std::cout << m_Stack.ReadFrom16((std::size_t)m_Registers[m_Pc->reg1.type]);
					break;
				}
				case 32:
				{
					//std::printf("%d", m_Stack.ReadFrom32((std::size_t)m_Registers[m_Pc->reg1.type]));
					std::cout << m_Stack.ReadFrom32((std::size_t)m_Registers[m_Pc->reg1.type]);
					break;
				}
			}
		}
		else
		{
			//std::printf("%d", m_Registers[m_Pc->reg1.type]);
			std::cout << m_Registers[m_Pc->reg1.type];
		}
		m_Pc++;
	}

	void ALVM::PrintStr()
	{
		// FIXME: Buffer the string instead of making a function for every single character.
		std::int8_t c = 1;
		std::size_t i = 0;
		while ((c = m_Stack.ReadFrom(m_Registers[m_Pc->reg1.type] - i++)) != 0) std::cout << c;

		m_Pc++;
	}

	void ALVM::CmpIntLessThan()
	{
		// Remember, when working with high performance applications, bad looking code always runs faster than clean looking code.
		if (m_Pc->reg2.type != RegType::Nul)
		{
			if (m_Pc->reg1.ptr)
			{
				switch (m_Pc->reg1.size)
				{
					case 8:
						m_Registers[RegType::CF] = m_Stack.ReadFrom(m_Registers[m_Pc->reg1.type]) < m_Registers[m_Pc->reg2.type];
						break;
					case 16:
						m_Registers[RegType::CF] = m_Stack.ReadFrom16(m_Registers[m_Pc->reg1.type]) < m_Registers[m_Pc->reg2.type];
						break;
					case 32:
					default:
						m_Registers[RegType::CF] = m_Stack.ReadFrom32(m_Registers[m_Pc->reg1.type]) < m_Registers[m_Pc->reg2.type];
						break;
				}
			}
			else
			{
				m_Registers[RegType::CF] = m_Registers[m_Pc->reg1.type] < m_Registers[m_Pc->reg2.type];
			}
		}
		else
		{
			if (m_Pc->reg1.ptr)
			{
				switch (m_Pc->reg1.size)
				{
					case 8:
						m_Registers[RegType::CF] = m_Stack.ReadFrom(m_Registers[m_Pc->reg1.type]) < m_Pc->imm32;
						break;
					case 16:
						m_Registers[RegType::CF] = m_Stack.ReadFrom16(m_Registers[m_Pc->reg1.type]) < m_Pc->imm32;
						break;
					case 32:
					default:
						m_Registers[RegType::CF] = m_Stack.ReadFrom32(m_Registers[m_Pc->reg1.type]) < m_Pc->imm32;
						break;
				}
			}
			else
			{
				m_Registers[RegType::CF] = m_Registers[m_Pc->reg1.type] < m_Pc->imm32;
			}
		}
		m_Pc++;
	}

	void ALVM::CmpIntEqualTo()
	{
		// Remember, when working with high performance applications, bad looking code always runs faster than clean looking code.
		if (m_Pc->reg2.type != RegType::Nul)
		{
			if (m_Pc->reg1.ptr)
			{
				switch (m_Pc->reg1.size)
				{
				case 8:
					m_Registers[RegType::CF] = m_Stack.ReadFrom(m_Registers[m_Pc->reg1.type]) == m_Registers[m_Pc->reg2.type];
					break;
				case 16:
					m_Registers[RegType::CF] = m_Stack.ReadFrom16(m_Registers[m_Pc->reg1.type]) == m_Registers[m_Pc->reg2.type];
					break;
				case 32:
				default:
					m_Registers[RegType::CF] = m_Stack.ReadFrom32(m_Registers[m_Pc->reg1.type]) == m_Registers[m_Pc->reg2.type];
					break;
				}
			}
			else
			{
				m_Registers[RegType::CF] = m_Registers[m_Pc->reg1.type] == m_Registers[m_Pc->reg2.type];
			}
		}
		else
		{
			if (m_Pc->reg1.ptr)
			{
				switch (m_Pc->reg1.size)
				{
				case 8:
					m_Registers[RegType::CF] = m_Stack.ReadFrom(m_Registers[m_Pc->reg1.type]) == m_Pc->imm32;
					break;
				case 16:
					m_Registers[RegType::CF] = m_Stack.ReadFrom16(m_Registers[m_Pc->reg1.type]) == m_Pc->imm32;
					break;
				case 32:
				default:
					m_Registers[RegType::CF] = m_Stack.ReadFrom32(m_Registers[m_Pc->reg1.type]) == m_Pc->imm32;
					break;
				}
			}
			else
			{
				m_Registers[RegType::CF] = m_Registers[m_Pc->reg1.type] == m_Pc->imm32;
			}
		}
		m_Pc++;
	}

	void ALVM::Move()
	{
		// this can probably be refactored to be better but meh
		if (m_Pc->reg2.type != RegType::Nul)
		{
			if (m_Pc->reg2.ptr)
			{
				if (m_Pc->reg1.ptr)
				{
					switch (m_Pc->reg1.size)
					{
						case 8:
							m_Stack.WriteAt(m_Registers[m_Pc->reg1.type], m_Stack.ReadFrom(m_Registers[m_Pc->reg2.type]));
							break;
						case 16:
							switch (m_Pc->reg2.size)
							{
								case 8:
									m_Stack.WriteAt16(m_Registers[m_Pc->reg1.type], m_Stack.ReadFrom(m_Registers[m_Pc->reg2.type]));
									break;
								case 16:
									m_Stack.WriteAt16(m_Registers[m_Pc->reg1.type], m_Stack.ReadFrom16(m_Registers[m_Pc->reg2.type]));
									break;
							}
							break;
						case 32:
						default:
							switch (m_Pc->reg2.size)
							{
								case 8:
									m_Stack.WriteAt32(m_Registers[m_Pc->reg1.type], m_Stack.ReadFrom(m_Registers[m_Pc->reg2.type]));
									break;
								case 16:
									m_Stack.WriteAt32(m_Registers[m_Pc->reg1.type], m_Stack.ReadFrom16(m_Registers[m_Pc->reg2.type]));
									break;
								case 32:
								default:
									m_Stack.WriteAt32(m_Registers[m_Pc->reg1.type], m_Stack.ReadFrom32(m_Registers[m_Pc->reg2.type]));
									break;
							}
							break;
					}
				}
				else
				{
					switch (m_Pc->reg1.size)
					{
						case 8:
							m_Registers[m_Pc->reg1.type] = m_Stack.ReadFrom(m_Registers[m_Pc->reg2.type]);
							break;
						case 16:
							m_Registers[m_Pc->reg1.type] = m_Stack.ReadFrom16(m_Registers[m_Pc->reg2.type]);
							break;
						case 32:
						default:
							m_Registers[m_Pc->reg1.type] = m_Stack.ReadFrom32(m_Registers[m_Pc->reg2.type]);
							break;
					}
				}
			}
			else
			{
				if (m_Pc->reg1.ptr)
				{
					switch (m_Pc->reg1.size)
					{
						case 8:
							m_Stack.WriteAt(m_Registers[m_Pc->reg1.type], (std::int8_t)m_Registers[m_Pc->reg2.type]);
							break;
						case 16:
							m_Stack.WriteAt16(m_Registers[m_Pc->reg1.type], (std::int16_t)m_Registers[m_Pc->reg2.type]);
							break;
						case 32:
						default:
							m_Stack.WriteAt32(m_Registers[m_Pc->reg1.type], m_Registers[m_Pc->reg2.type]);
							break;
					}
				}
				else
				{
					m_Registers[m_Pc->reg1.type] = m_Registers[m_Pc->reg2.type];
				}
			}
		}
		else
		{
			if (m_Pc->reg1.ptr)
			{
				switch (m_Pc->reg1.size)
				{
					case 8:
						m_Stack.WriteAt(m_Registers[m_Pc->reg1.type], (std::int8_t)m_Pc->imm32);
						break;
					case 16:
						m_Stack.WriteAt16(m_Registers[m_Pc->reg1.type], (std::int16_t)m_Pc->imm32);
						break;
					case 32:
					default:
						m_Stack.WriteAt32(m_Registers[m_Pc->reg1.type], m_Pc->imm32);
						break;
				}
			}
			else
			{
				m_Registers[m_Pc->reg1.type] = m_Pc->imm32;
			}
		}
		m_Pc++;
	}

	void ALVM::Jump()
	{
		if (m_Pc->reg1.type != RegType::Nul) m_Pc = m_Bytecode + m_Registers[m_Pc->reg1.type];
		else m_Pc = m_Bytecode + m_Pc->imm32;
	}

	void ALVM::ConditionalJump()
	{
		if (m_Registers[RegType::CF]) Jump();
		else m_Pc++;
	}

	void ALVM::ConditionalNotJump()
	{
		if (!m_Registers[RegType::CF]) Jump();
		else m_Pc++;
	}

	void ALVM::Call()
	{
		m_Stack.Push32((uintptr_t)(m_Pc + 1));
		Jump();
	}

	void ALVM::Return()
	{
		Instruction* addr = (Instruction*)(uintptr_t)m_Stack.Read32();
		m_Pc = addr;
	}

	void ALVM::Db()
	{
		std::size_t size = m_Pc->bytes.size();
		m_Stack.InsertBack(m_Pc->bytes.begin(), m_Pc->bytes.end());
		m_Registers[RegType::Sp] -= size;
		m_Pc++;
	}

}
