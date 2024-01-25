#[derive(Debug)]
pub enum TokenKind {
    None,
    NumberLiteral(i64),

    // Operators
    Plus,
    Minus,
    ForwardSlash,
    Asterisk,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    LeftSquareBracket,
    RightSquareBracket,
    LeftAngleBracket,
    RightAngleBracket,
    Comma,
    DoubleQuote,
    SingleQuote,
    Colon,
    SemiColon,
    Equal,

    EOF,
}

#[derive(Debug)]
pub struct TextSpan {
    pub(crate) start: usize,
    pub(crate) end: usize,
    pub(crate) text: String,
}

impl TextSpan {
    pub fn new(start: usize, end: usize, text: String) -> Self {
        Self { start, end, text }
    }

    pub fn len(&self) -> usize {
        return self.end - self.start;
    }
}

#[derive(Debug)]
pub struct Token {
    pub(crate) kind: TokenKind,
    pub(crate) span: TextSpan,
}

impl Token {
    pub fn new(token_kind: TokenKind, span: TextSpan) -> Self {
        Self {
            kind: token_kind,
            span,
        }
    }
}

pub struct Lexer<'a> {
    source: &'a str,
    current_pos: usize,
}

impl<'a> Lexer<'a> {
    pub fn new(input: &'a str) -> Self {
        Self {
            source: input,
            current_pos: 0,
        }
    }

    pub fn next_token(&mut self) -> Option<Token> {
        // Skip whitespaces
        while let Some(c) = self.current_char() {
            if !c.is_whitespace() {
                break;
            }
            self.current_pos += 1;
        }

        if self.current_pos > self.source.len() {
            return None;
        }

        if self.current_pos == self.source.len() {
            let eof: char = '\0';
            self.current_pos += 1;
            return Some(Token::new(
                TokenKind::EOF,
                TextSpan::new(0, 0, eof.to_string()),
            ));
        }

        let c_opt = self.current_char();
        c_opt.map(|c| {
            let start = self.current_pos;
            let mut token_kind = TokenKind::None;
            if Self::is_number_start(&c) {
                let num: i64 = self.consume_number();
                token_kind = TokenKind::NumberLiteral(num);
            } else {
                token_kind = self.consume_operator();
            }

            let end = self.current_pos;
            let text = self.source[start..end].to_string();
            let span = TextSpan::new(start, end, text);
            Token::new(token_kind, span)
        })
    }

    fn is_number_start(c: &char) -> bool {
        c.is_digit(10)
    }

    fn current_char(&self) -> Option<char> {
        self.source.chars().nth(self.current_pos)
    }

    fn consume(&mut self) -> Option<char> {
        if self.current_pos >= self.source.len() {
            return None;
        }

        let c = self.current_char();
        self.current_pos += 1;

        c
    }

    fn consume_number(&mut self) -> i64 {
        let mut num: i64 = 0;
        while let Some(c) = self.current_char() {
            if c.is_digit(10) {
                self.consume().unwrap();
                num = num * 10 + c.to_digit(10).unwrap() as i64;
            } else {
                break;
            }
        }
        num
    }

    fn consume_operator(&mut self) -> TokenKind {
        let c = self.consume().unwrap();
        match c {
            '+' => TokenKind::Plus,
            '-' => TokenKind::Minus,
            '/' => TokenKind::ForwardSlash,
            '*' => TokenKind::Asterisk,
            '(' => TokenKind::LeftParen,
            ')' => TokenKind::RightParen,
            '{' => TokenKind::LeftBracket,
            '}' => TokenKind::RightBracket,
            '[' => TokenKind::LeftSquareBracket,
            ']' => TokenKind::RightSquareBracket,
            '<' => TokenKind::LeftAngleBracket,
            '>' => TokenKind::RightAngleBracket,
            ',' => TokenKind::Comma,
            '"' => TokenKind::DoubleQuote,
            '\'' => TokenKind::SingleQuote,
            ':' => TokenKind::Colon,
            ';' => TokenKind::SemiColon,
            '=' => TokenKind::Equal,
            _ => TokenKind::None,
        }
    }
}
