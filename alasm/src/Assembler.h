#ifndef ALVM_ASSEMBLER_H
#define ALVM_ASSEMBLER_H

#include <ALVM.h>
#include <cstdint>
#include <unordered_map>

#include "Lexer.h"
#include "Utils.h"

namespace rlang::rmc
{
    // Heads Up: This file is cursed by the devil himself and formatting these enums are impossible!
    // --at least with Emacs that is.
    enum DataType
    {
    Undefined,
    Byte,
    Word,
    DWord,
    QWord
};

    struct DataInfo
    {
        std::uintptr_t addr = 0;
        std::size_t size = 0;
        std::uint64_t value = 0;
        bool constant = false;
        bool initialized = true;
        DataType type = DataType::Undefined;
    };

    enum AssemblerStatus
    {
    Ok,
    AssembleError,
    WriteError,
    ReadError
};

    struct AssemblerResult
    {
        alvm::InstructionList assembledCode;
        std::vector<std::uint8_t> dataSection;
        AssemblerStatus status;
    };

    enum class OutputType
    {
    Lib,
    DLib,
    XBin
};

    struct AssemblerOptions
    {
        OutputType type;
        const TokenList& tokens;
        const std::string& path;
    };

    struct Assembler
    {
    private:
        static std::unordered_map<std::string, DataInfo> m_SymbolTable;
        static std::unordered_map<std::string, std::pair<std::size_t, std::unordered_map<std::string, std::size_t>>> m_LabelAddressMap;
        static std::vector<alvm::Instruction> m_AssembledCode;
        static std::vector<alvm::Instruction> m_InstEpilogue;
        static std::vector<std::uint8_t> m_DataSection;
        static std::size_t m_BssSize;
        static std::string m_CurrentSection;

    private:
        static void LabelProcessor(const TokenList& tokens);
        static void Cleanup();
        static AssemblerStatus WriteToBinary(const std::string& path);

    public:
        static AssemblerResult Assemble(const AssemblerOptions& opt);

    private:
        static AssemblerStatus CodeGen(const TokenList& tokens);
        static alvm::OpCode GetInst(std::string inst);
        static alvm::RegType GetReg(std::string reg);
    };
} // namespace rlang::rmc

#endif // ALVM_ASSEMBLER_H
