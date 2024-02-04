#ifndef BLEND_ASSEMBLER_H
#define BLEND_ASSEMBLER_H

#include <Blend.h>
#include <cstdint>
#include <unordered_map>

#include "Lexer.h"
#include "Utils.h"

#include <CommonDef.h>

namespace relang::basm
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
        uintptr addr = 0;
        usize size = 0;
        u64 value = 0;
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
        blend::InstructionList assembledCode;
        std::vector<u8> dataSection;
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
        static std::unordered_map<std::string, std::pair<usize, std::unordered_map<std::string, usize>>> m_LabelAddressMap;
        static std::vector<blend::Instruction> m_AssembledCode;
        static std::vector<blend::Instruction> m_InstEpilogue;
        static std::vector<u8> m_DataSection;
        static usize m_BssSize;
        static std::string m_CurrentSection;

    private:
        static void LabelProcessor(const TokenList& tokens);
        static void Cleanup();
        static AssemblerStatus WriteToBinary(const std::string& path);

    public:
        static AssemblerResult Assemble(const AssemblerOptions& opt);

    private:
        static AssemblerStatus CodeGen(const TokenList& tokens);
        static blend::OpCode GetInst(std::string inst);
        static blend::RegType GetReg(std::string reg);
    };
} // namespace relang::rmc

#endif // BLEND_ASSEMBLER_H
