pub mod lexer;
pub mod parser;

pub enum ExpressionKind {
    NumberLiteral(i64),
}

pub struct Expression {
    kind: ExpressionKind,
}

pub enum StatementKind {
    Expression(Expression),
}

pub struct Statement {
    kind: StatementKind,
}

impl Statement {
    pub fn new(kind: StatementKind) -> Self {
        Self { kind }
    }

    pub fn expression(expr: Expression) -> Self {
        Statement::new(StatementKind::Expression(expr))
    }
}

pub struct AST {
    pub statements: Vec<Statement>,
}

impl AST {
    pub fn new() -> Self {
        Self {
            statements: Vec::new(),
        }
    }

    pub fn display(&self) {}
}

pub struct ASTVisitor<'a> {
    pub ast: &'a AST,
}

impl<'a> ASTVisitor<'a> {
    pub fn new(ast: &'a AST) {
        Self { ast }
    }

    pub fn visit_expression() {}

    pub fn visit(&self) {
        for stmt in self.ast.statements.iter() {
            match &stmt.kind {
                StatementKind::Expression(expr) => {
                    self.visit_expression(expr);
                }
            }
        }
    }
}
