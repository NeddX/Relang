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

        struct StringPool
        {
        };
    } // namespace codegen

    class Compiler
    {
    private:
        ast::SyntaxTree                          m_Tree{};
        blend::InstructionList             m_CompiledCode{};
        std::vector<codegen::FunctionDefinition> m_CompiledFunctions{};
        std::vector<codegen::SymbolTable>        m_SymbolTableStack{};

    public:
        Compiler(ast::SyntaxTree tree);

    public:
        blend::InstructionList Compile();
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
