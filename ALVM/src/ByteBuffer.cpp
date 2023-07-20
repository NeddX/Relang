/*#include "ByteBuffer.h"

namespace rlang::alvm {
	template<std::size_t Size>
	void ByteBuffer<Size>::Push(std::uint8_t data)
	{
		m_Buffer[m_StackPointer--] = data;
		//m_Buffer.push_back(data);
	}

	template<std::size_t Size>
	void ByteBuffer<Size>::Push16(std::uint16_t data)
	{
		Push(data & 0xff);
		Push(data >> 8);
	}

	template<std::size_t Size>
	void ByteBuffer<Size>::Push32(std::uint32_t data)
	{
		Push16(data & 0xffff);
		Push16(data >> 16);
	}

	template<std::size_t Size>
	std::uint8_t ByteBuffer<Size>::Pop()
	{
		//std::uint8_t data = m_Buffer.back();
		//m_Buffer.pop_back();
		return m_Buffer[m_StackPointer++];
	}

	template<std::size_t Size>
	std::uint16_t ByteBuffer<Size>::Pop16()
	{
		return ((std::uint16_t)Pop() << 8) | Pop();
	}

	template<std::size_t Size>
	std::uint32_t ByteBuffer<Size>::Pop32()
	{
		return (Pop32() << 16) | (std::uint16_t)Pop32();
	}

	template<std::size_t Size>
	std::uint8_t ByteBuffer<Size>::Read()
	{
		return m_Buffer.back();
	}

	template<std::size_t Size>
	std::uint16_t ByteBuffer<Size>::Read16()
	{
		return ((std::uint16_t)Read() << 8) | Read();
	}

	template<std::size_t Size>
	std::uint32_t ByteBuffer<Size>::Read32()
	{
		return ((std::uint32_t)Read16() << 16) | Read16();
	}

	template<std::size_t Size>
	void ByteBuffer<Size>::WriteAt(std::size_t address, std::uint8_t data)
	{
		m_Buffer[address] = data;
	}

	template<std::size_t Size>
	void ByteBuffer<Size>::WriteAt16(std::size_t address, std::uint16_t data)
	{
		WriteAt(address, data & 0xff);
		WriteAt(address + 1, data >> 8);
	}

	template<std::size_t Size>
	void ByteBuffer<Size>::WriteAt32(std::size_t address, std::uint32_t data)
	{
		WriteAt16(address, data & 0xffff);
		WriteAt16(address + 2, data >> 16);
	}

	template<std::size_t Size>
	std::uint8_t ByteBuffer<Size>::ReadFrom(std::size_t address)
	{
		return m_Buffer[address];
	}

	template<std::size_t Size>
	std::uint16_t ByteBuffer<Size>::ReadFrom16(std::size_t address)
	{
		return ((std::uint16_t)ReadFrom(address + 1) << 8) | ReadFrom(address);
	}

	template<std::size_t Size>
	std::uint32_t ByteBuffer<Size>::ReadFrom32(std::size_t address)
	{
		return ((std::uint32_t)ReadFrom16(address + 2) << 16) | ReadFrom16(address);
	}
}
*/
