#ifndef ALVM_BYTE_BUFFER_H
#define ALVM_BYTE_BUFFER_H

#include <sdafx.h>

namespace rlang::alvm {
	template<std::size_t Size>
	class ByteBuffer
	{
	private:
		std::vector<std::uint8_t> m_Buffer;
		std::uintptr_t& m_StackPointer;

	public:
		ByteBuffer(std::uintptr_t& stackPointer) :
			m_StackPointer(stackPointer)
		{
			m_Buffer.resize(Size);
			m_StackPointer = (std::uintptr_t)m_Buffer.data() + Size - 1;
		}

	public:
		inline std::uint8_t& operator[](std::size_t index) { return m_Buffer[index]; }
		inline std::size_t GetSize() const { return m_Buffer.size(); }

	public:
		template<typename Container> // FIXME: Add compile-time checking so that none container types are not passed to this guy.
		inline bool InsertAt(const std::size_t where, const Container& container, const std::size_t begin, const std::size_t end)
		{
			if (where > m_Buffer.size() || where < 0 ||
				begin > container.size() || begin > 0 ||
				end > container.size() || end < 0) return false;

			auto it2 = container.begin() + begin;
			for (auto it = m_Buffer.begin() + where;
				 it != m_Buffer.end() && it2 != container.end() - end; ++it, ++it2)
			{
				*it = *it2;
			}
			return true;
		}
		template<typename Iterator> // FIXME: Add compile-time checking to prevent non-iterator types from being passed.
		inline bool InsertBack(const Iterator begin, const Iterator end)
		{
			std::size_t size = std::distance(begin, end);

			if (m_StackPointer >= m_StackPointer + m_Buffer.size() || m_StackPointer < (std::uintptr_t)m_Buffer.data())
				return false;

			for (auto i = 0; i < size; ++i)
			{
				*(std::uint8_t*)(m_StackPointer - i) = *(begin + i);
				//m_Buffer[m_Buffer.size() - 1 - i] = *(begin + i);
			}

			return true;
		}

	public:
		void Push(std::uint8_t data)
		{
			*(std::uint8_t*)(m_StackPointer--) = data;
			//m_Buffer[m_StackPointer--] = data;
		}
		void Push16(std::uint16_t data)
		{
			Push(data & 0xff);
			Push(data >> 8);
		}
		void Push32(std::uint32_t data)
		{
			Push16(data & 0xffff);
			Push16(data >> 16);
		}

		std::uint8_t Pop()
		{
			return *(std::uint8_t*)++m_StackPointer;
			//return m_Buffer[++m_StackPointer];
		}
		std::uint16_t Pop16()
		{
			return ((std::uint16_t)Pop() << 8) | Pop();
		}
		std::uint32_t Pop32()
		{
			return (Pop16() << 16) | (std::uint16_t)Pop16();
 		}
	};
}

#endif // ALVM_BYTE_BUFFER_H
