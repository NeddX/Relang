#ifndef ALVM_MEMORY_MAN_H
#ddefine ALVM_MEMORY_MAN_H

#include <sdafx.h>

namespace rlang::alvm
{
    class MemMan
    {
    public:
        static inline void WriteAt(const std::size_t address, const std::uint8_t data)
        {
            *(std::uint8_t*)address = data;
        }
        static inline void WriteAt16(const std::size_t address, const std::uint16_t data)
        {
            WriteAt(address, data & 0xff);
            WriteAt(address + 1, data >> 8);
        }
        static inline void WriteAt32(const std::size_t address, const std::uint32_t data)
        {
            WriteAt16(address, data & 0xffff);
            WriteAt16(address + 2, data >> 16);
        }

        static inline std::uint8_t ReadFrom(const std::uintptr_t address)
        {
            return *(std::uint8_t*)address;
        }
        static inline std::uint16_t ReadFrom16(const std::uintptr_t address)
        {
            return ((std::uint16_t)ReadFrom(address + 1) << 8) | ReadFrom(address);
        }
        static inline std::uint32_t ReadFrom32(const std::size_t address)
        {
            return ((std::uint32_t)ReadFrom16(address + 2) << 16) | ReadFrom16(address);
        }
    };
} // namespace rlang::alvm

#endif // ALVM_MEMORY_MAN_H
