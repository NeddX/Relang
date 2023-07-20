#ifndef ALVM_ALVM_BYTE_BUFFER_H
#define ALVM_ALVM_BYTE_BUFFER_H

#include <sdafx.h>

namespace rlang::alvm {
	template<std::size_t Size>
	class ByteBuffer
	{
	private:
		std::array<std::uint8_t, Size> m_Buffer;
		std::size_t& m_StackPointer;

	public:
		ByteBuffer(std::size_t& stackPointer) :
			m_StackPointer(stackPointer)
		{
			m_StackPointer = m_Buffer.size() - 1;
			std::fill(m_Buffer.begin(), m_Buffer.end(), 0);
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
			if (m_StackPointer == 0 || m_StackPointer - size < 0)
				return false;

			for (auto i = 0; i < size; ++i)
			{
				m_Buffer[m_StackPointer - i] = *(begin + i);
			}

			return true;
		}

	public:
		void Push(std::uint8_t data)
		{
			m_Buffer[m_StackPointer--] = data;
			//m_Buffer.push_back(data);
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
			//std::uint8_t data = m_Buffer.back();
			//m_Buffer.pop_back();
			return m_Buffer[m_StackPointer++];
		}
		std::uint16_t Pop16()
		{
			return ((std::uint16_t)Pop() << 8) | Pop();
		}
		std::uint32_t Pop32()
		{
			return (Pop32() << 16) | (std::uint16_t)Pop32();
		}

		std::uint8_t Read()
		{
			return m_Buffer.back();
		}
		std::uint16_t Read16()
		{
			return ((std::uint16_t)Read() << 8) | Read();
		}
		std::uint32_t Read32()
		{
			return ((std::uint32_t)Read16() << 16) | Read16();
		}

		void WriteAt(std::size_t address, std::uint8_t data)
		{
			m_Buffer[address] = data;
		}
		void WriteAt16(std::size_t address, std::uint16_t data)
		{
			WriteAt(address, data & 0xff);
			WriteAt(address + 1, data >> 8);
		}
        void WriteAt32(std::size_t address, std::uint32_t data)
		{
			WriteAt16(address, data & 0xffff);
			WriteAt16(address + 2, data >> 16);
        }

        std::uint8_t ReadFrom(std::size_t address)
		{
			return m_Buffer[address];
		}
		std::uint16_t ReadFrom16(std::size_t address)
		{
			return ((std::uint16_t)ReadFrom(address + 1) << 8) | ReadFrom(address);
		}
		std::uint32_t ReadFrom32(std::size_t address)
		{
			return ((std::uint32_t)ReadFrom16(address + 2) << 16) | ReadFrom16(address);
		}
	};
}

#endif // ALVM_ALVM_BYTE_BUFFER_H
