#ifndef CMC_COMPILER_H
#define CMC_COMPILER_H

#include <Blend.h>
#include <vector>

#include "../Analyzer/Parser.h"

namespace relang::refront {
    namespace codegen {
        enum class SymbolKind : u8
        {
            None,
            Variable,
            Function
        };

        struct Symbol
        {
            std::string    name{};
            SymbolKind     kind{};
            ast::Statement stmt{};
            usize          size{};
            i32            address{};
        };

        struct SymbolTable
        {
        private:
            std::unordered_map<std::string, Symbol> m_Symbol{};
            i32                                     m_Offset{};
            usize                                   m_UsedRegisters{};

        public:
            inline i32&         GetOffset() noexcept { return m_Offset; }
            inline const i32&   GetOffset() const noexcept { return m_Offset; }
            inline usize&       GetUsedRegisters() noexcept { return m_UsedRegisters; };
            inline const usize& GetUsedRegisters() const noexcept { return m_UsedRegisters; };

        public:
            void          AddSymbol(Symbol symbol) noexcept;
            bool          ContainsSymbol(const std::string& name) const noexcept;
            Symbol&       GetSymbol(const std::string& name) noexcept;
            const Symbol& GetSymbol(const std::string& name) const noexcept;
        };

        struct FunctionDefinition
        {
            std::string name{};
            usize       address{};
        };

        using StringPool = std::unordered_map<std::string, usize>;

        struct CompiledCode
        {
        private:
            std::vector<std::pair<blend::Instruction, ast::Statement>> m_Code{};
            std::vector<u8>                                            m_Data{};
            usize                                                      m_BssSize{};
                StringPool                   m_StringPool{};

        public:
            inline const std::vector<u8>& GetDataSection() const noexcept { return m_Data; }
            inline usize                  GetBssSize() const noexcept { return m_BssSize; }
            inline StringPool&            GetStringPool() noexcept { return m_StringPool; }
            inline const StringPool&      GetStringPool() const noexcept { return m_StringPool; }

        public:
            inline CompiledCode& operator<<(std::pair<blend::Instruction,ast::Statement> inst) noexcept
            {
                m_Code.push_back(std::move(inst));
                return *this;
            }
            inline CompiledCode& operator<<(const u8 byte) noexcept { 
                m_Data.push_back(byte);
                return *this;
            }
            inline operator blend::InstructionList() const noexcept
            { 
                blend::InstructionList list{};
                for (const auto& e : m_Code)
                    list.push_back(e.first);
                return list;
            }
        };
        
        std::pair<blend::Instruction, ast::Statement> MakeInst(const blend::Instruction& inst = blend::Instruction{},
                                                               const ast::Statement&     stmt = ast::Statement{}) noexcept;
    } // namespace codegen

    class Compiler
    {
    private:
        ast::SyntaxTree                          m_Tree{};
        codegen::CompiledCode                    m_CompiledCode{};
        std::vector<codegen::FunctionDefinition> m_CompiledFunctions{};
        std::vector<codegen::SymbolTable>        m_SymbolTableStack{};

    public:
        Compiler(ast::SyntaxTree tree);

    public:
        codegen::CompiledCode Compile();
        void                         CompileFunctionBody(const ast::Statement& fnStmt);
        void                         CompileBlockStatement(const ast::Statement& block);
        void                         CompileVariableDeclaration(const ast::Statement& var);
        void                         CompileInitializer(const ast::Statement& init);
        void                         CompileExpression(const ast::Statement& expr);
        void                         CompileLiteral(const ast::Statement& literal);
        void                         CompileInitializerList(const ast::Statement& initList);
        void                         CompileFunctionCall(const ast::Statement& fnCall);
        void                         CompileIdentifierName(const ast::Statement& ident);
        void                         CompileFunctionArgumentList(const ast::Statement& args);
    };
} // namespace relang::refront

#endif // CMC_COMPILER_H
