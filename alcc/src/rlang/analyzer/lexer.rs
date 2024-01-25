#[derive(Debug)]
pub enum TokenType {
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
    start: usize,
    end: usize,
    text: String,
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
    token_type: TokenType,
    span: TextSpan,
}

impl Token {
    pub fn new(token_type: TokenType, span: TextSpan) -> Self {
        Self { token_type, span }
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
                TokenType::EOF,
                TextSpan::new(0, 0, eof.to_string()),
            ));
        }

        let c_opt = self.current_char();
        c_opt.map(|c| {
            let start = self.current_pos;
            let mut token_type = TokenType::None;
            if Self::is_number_start(&c) {
                let num: i64 = self.consume_number();
                token_type = TokenType::NumberLiteral(num);
            } else {
                token_type = self.consume_operator();
            }

            let end = self.current_pos;
            let text = self.source[start..end].to_string();
            let span = TextSpan::new(start, end, text);
            Token::new(token_type, span)
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

    fn consume_operator(&mut self) -> TokenType {
        let c = self.consume().unwrap();
        match c {
            '+' => TokenType::Plus,
            '-' => TokenType::Minus,
            '/' => TokenType::ForwardSlash,
            '*' => TokenType::Asterisk,
            '(' => TokenType::LeftParen,
            ')' => TokenType::RightParen,
            '{' => TokenType::LeftBracket,
            '}' => TokenType::RightBracket,
            '[' => TokenType::LeftSquareBracket,
            ']' => TokenType::RightSquareBracket,
            '<' => TokenType::LeftAngleBracket,
            '>' => TokenType::RightAngleBracket,
            ',' => TokenType::Comma,
            '"' => TokenType::DoubleQuote,
            '\'' => TokenType::SingleQuote,
            ':' => TokenType::Colon,
            ';' => TokenType::SemiColon,
            '=' => TokenType::Equal,
            _ => TokenType::None,
        }
    }
}
