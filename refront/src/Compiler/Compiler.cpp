#include "Compiler.h"

namespace relang::refront {
    using ast::FundamentalType;
    using ast::Statement;
    using ast::StatementKind;
    using ast::SyntaxTree;
    using ast::Type;
    using namespace relang;
    using namespace relang::blend;

    namespace codegen {
        void SymbolTable::AddSymbol(Symbol symbol) noexcept
        {
            m_Symbol[symbol.name] = std::move(symbol);
            m_Offset += symbol.size;
        }

        bool SymbolTable::ContainsSymbol(const std::string& name) const noexcept
        {
            return m_Symbol.contains(name);
        }

        Symbol& SymbolTable::GetSymbol(const std::string& name) noexcept
        {
            return m_Symbol[name];
        }

        const Symbol& SymbolTable::GetSymbol(const std::string& name) const noexcept
        {
            return ((SymbolTable*)this)->GetSymbol(name);
        }
    } // namespace codegen

    using namespace codegen;

    RegType GetReg(const usize idx) noexcept
    {
        constexpr RegType r = RegType::R0;
        return (RegType)((usize)r + idx);
    }

    Compiler::Compiler(SyntaxTree tree) : m_Tree(std::move(tree))
    {
    }

    InstructionList Compiler::Compile()
    {
        for (const auto& s : m_Tree)
        {
            switch (s.kind)
            {
                using enum StatementKind;

                case FunctionDeclaration: CompileFunctionBody(s); break;

                default: break;
            }
        }
        m_CompiledCode.push_back(Instruction{ .opcode = OpCode::End });
        return m_CompiledCode;
    }

    void Compiler::CompileFunctionBody(const Statement& fnStmt)
    {
        for (const auto& s : fnStmt.children)
        {
            if (s.kind == StatementKind::BlockStatement)
            {
                CompileBlockStatement(s);
            }
        }
    }

    void Compiler::CompileBlockStatement(const Statement& block)
    {
        m_SymbolTableStack.push_back(SymbolTable{});

        m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Push, .sreg = RegType::BP });
        m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Mov, .sreg = RegType::SP, .dreg = RegType::BP });
        // m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Pushar });

        for (const auto& s : block.children)
        {
            switch (s.kind)
            {
                using enum StatementKind;

                case VariableDeclaration: CompileVariableDeclaration(s); break;
                case FunctionCallExpression: CompileFunctionCall(s); break;
                default: break;
            }
        }

        m_SymbolTableStack.pop_back();
        // m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Popar });
        m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Leave });
    }

    void Compiler::CompileVariableDeclaration(const Statement& var)
    {
        // Grab our current block's symbol table.
        auto& current_table = m_SymbolTableStack.back();

        Symbol sym{};
        sym.stmt    = var;
        sym.name    = var.name;
        sym.kind    = SymbolKind::Variable;
        sym.size    = (var.type.size / 8) * ((var.type.length == 0) ? 1 : var.type.length);
        sym.address = current_table.GetOffset();

        // Initialized.
        if (!var.children.empty())
        {
            auto current_offset = current_table.GetOffset();

            // We know that a variable declaration statement will always have an Initializer statement if initialized
            // (but of course).
            m_CompiledCode.push_back(Instruction{
                .opcode = OpCode::Mov, .sreg = RegType::SP, .dreg = GetReg(current_table.GetUsedRegisters()++) });
            CompileInitializer(var.children[0]);
            switch (sym.stmt.type.ftype)
            {
                using enum FundamentalType;

                case String: {
                        m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Pop,
                                                              .sreg   = GetReg(current_table.GetUsedRegisters()++),
                                                              .size   = (i8)Type::Character.size });
                        m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Store,
                                                              .sreg   = GetReg(--current_table.GetUsedRegisters()),
                                                              .dreg   = MemReg(RegType::BP),
                                                              .disp   = current_table.GetOffset(),
                                                              .size   = (i8)var.type.size });
                    break;
                }

                case Character:
                case Boolean:
                case Integer32:
                case Integer64: {
                    m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Store,
                                                          .sreg   = GetReg(--current_table.GetUsedRegisters()),
                                                          .dreg   = MemReg(RegType::BP),
                                                          .disp   = current_table.GetOffset(),
                                                          .size   = (i8)var.type.size });
                    break;
                }
                default: break;
            }
            m_CompiledCode.push_back(Instruction{
                .opcode = OpCode::Mov, .sreg = GetReg(--current_table.GetUsedRegisters()), .dreg = RegType::SP });
            current_table.AddSymbol(sym);
        }
        else
        {
            // Just allocate space on the stack.
            switch (var.type.ftype)
            {
                using enum FundamentalType;

                case Boolean:
                case Character:
                case Integer32:
                case Integer64:
                    m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Store,
                                                          .dreg   = MemReg(RegType::BP),
                                                          .disp   = current_table.GetOffset(),
                                                          .size   = (i8)var.type.size });
                    break;
                // FIXME: Uninitilized strings do not allocate space.
                case String: break;
                default: break;
            }
        }
    }

    void Compiler::CompileInitializer(const Statement& init)
    {
        // Check if the initializer's value is a value, expression or a initializer list.
        CompileExpression(init.children[0]);
    }

    void Compiler::CompileExpression(const ast::Statement& expr)
    {
        auto& current_table = m_SymbolTableStack.back();
        switch (expr.kind)
        {
            using enum StatementKind;

            case FunctionCallExpression: {
                break;
            }
            case FunctionArgumentList: {
                break;
            }
            case LiteralExpression: {
                CompileLiteral(expr);
                break;
            }
            case AdditionExpression:
            case SubtractionExpression: {
                CompileExpression(expr.children[0]);
                CompileExpression(expr.children[1]);

                auto&      used_regs = current_table.GetUsedRegisters();
                const auto op_code   = (expr.kind == AdditionExpression) ? OpCode::Add : OpCode::Sub;
                m_CompiledCode.push_back(
                    Instruction{ .opcode = op_code, .sreg = GetReg(--used_regs), .dreg = GetReg(used_regs - 1) });
                break;
            }
            case DivisionExpression:
            case MultiplicationExpression: {
                CompileExpression(expr.children[0]);
                CompileExpression(expr.children[1]);

                auto&      used_regs = current_table.GetUsedRegisters();
                const auto op_code   = (expr.kind == DivisionExpression) ? OpCode::Div : OpCode::Mul;
                m_CompiledCode.push_back(
                    Instruction{ .opcode = op_code, .sreg = GetReg(--used_regs), .dreg = GetReg(used_regs - 1) });
                break;
            }

            case IdentifierName: {
                auto& sym = m_SymbolTableStack.back().GetSymbol(expr.name);
                m_CompiledCode.push_back(Instruction{ .opcode  = OpCode::Lea,
                                                      .sreg    = GetReg(current_table.GetUsedRegisters()++),
                                                      .disp    = sym.address,
                                                      .src_reg = RegType::BP });
                break;
            }
            default: break;
        }
    }

    void Compiler::CompileLiteral(const Statement& literal)
    {
        // Grab our current block's symbol table.
        auto& current_table = m_SymbolTableStack.back();

        // The literal token.
        auto& literal_token = literal.tokens[0];

        // We support fundamental types only for now.
        switch (literal.type.ftype)
        {
            using enum FundamentalType;

            // For numeric types.
            case Boolean:
            case Character:
            case Integer32:
            case Integer64: {
                m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Mov,
                                                      .imm64  = (u64)literal_token.num,
                                                      .dreg   = GetReg(current_table.GetUsedRegisters()++) });
                break;
            }
            case String: {
                auto offset = current_table.GetOffset();
                for (char c : literal_token.span.text)
                {
                    m_CompiledCode.push_back(
                        Instruction{ .opcode = OpCode::Push, .imm64 = (u64)c, .size = (i8)(Type::Character.size / 8) });
                    offset += Type::Character.size / 8;
                }
                m_CompiledCode.push_back(
                    Instruction{ .opcode = OpCode::Push, .imm64 = 0, .size = (i8)(Type::Character.size / 8) });
                break;
            }

            default: break;
        }
    }

    void Compiler::CompileInitializerList(const ast::Statement& initList)
    {
        // Grab our current block's symbol table.
        auto& current_table = m_SymbolTableStack.back();

        auto prev_offset = current_table.GetOffset();
        for (const auto& expr : initList.children)
        {
            CompileExpression(expr);
            current_table.GetOffset() += expr.type.size / 8;
        }

        current_table.GetOffset() = prev_offset;
    }

    void Compiler::CompileFunctionCall(const ast::Statement& fnCall)
    {
        auto& current_table = m_SymbolTableStack.back();
        if (fnCall.name == "printi64")
        {
            CompileFunctionArgumentList(fnCall.children[0]);
            m_CompiledCode.push_back(
                Instruction{ .opcode = OpCode::PInt, .sreg = GetReg(--current_table.GetUsedRegisters()) });
            return;
        }
        else if (fnCall.name == "printstr")
        {
            CompileFunctionArgumentList(fnCall.children[0]);
            m_CompiledCode.push_back(
                Instruction{ .opcode = OpCode::PStr, .sreg = GetReg(--current_table.GetUsedRegisters()) });
            return;
        }

        for (const auto& fn : m_CompiledFunctions)
        {
            if (fn.name == fnCall.name)
            {
                m_CompiledCode.push_back(Instruction{ .opcode = OpCode::Call, .imm64 = fn.address });
                for (const auto& arg : fnCall.children[0].children)
                {
                    CompileExpression(arg);
                }
            }
        }
    }

    void Compiler::CompileFunctionArgumentList(const ast::Statement& args)
    {
        for (const auto& arg : args.children)
        {
            CompileExpression(arg);
        }
    }
} // namespace relang::refront
