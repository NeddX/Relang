use super::lexer::{Lexer, Token, TokenKind};
use super::{Expression, Statement};

pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
}

impl Parser {
    pub fn new() -> Self {
        Self {
            tokens: Vec::new(),
            current: 0,
        }
    }

    pub fn new_from(source: &str) -> Self {
        let mut lexer = Lexer::new(source);
        let mut token_vec = Vec::new();
        while let Some(token) = lexer.next_token() {
            token_vec.push(token);
        }
        Self {
            tokens: token_vec,
            current: 0,
        }
    }

    pub fn parse_expression(&mut self) -> Option<Expression> {
        let token = self.current()?;
        match token.kind {
            TokenKind::NumberLiteral(num) => Some(Expression::number(num)),
            _ => None,
        }
    }

    pub fn parse_statement(&mut self) -> Option<Statement> {
        let token = self.current()?;
        let expr = self.parse_expression()?;
        Some(Statement::expression(expr))
    }

    pub fn next_statement(&mut self) -> Option<Statement> {
        self.parse_statement()
    }

    pub fn peek<'a>(&'a self, offset: usize) -> Option<&'a Token> {
        None
    }

    pub fn current<'a>(&'a self) -> Option<&'a Token> {
        self.peek(0)
    }
}
