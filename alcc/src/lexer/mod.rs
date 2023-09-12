mod lexer {
    pub enum TokenType {
        NumberLiteral(i64),
        Plus,
        Minus,
        Asterisk,
        Slash,
        LeftParen,
        RightParen
    }

    pub struct TextSpan {
        start: usize,
        end: usize,
        text: String
    }

    impl TextSpan {
        pub fn new(start: usize, end: usize, text: String) -> self {
            return TextSpan { start, end, text };
        }

        pub fn len(&self) -> usize {
            return self.end - self.start;
        }
    }

    pub struct Token {
        tokenType: TokenType,
        span: TextSpan,
        line: usize,
        cur: usize
    }

    impl Token {
        pub fn new(tokenType: TokenType, span: TextSpan, line: usize, cur: usize) -> self {
            return Token { tokenType, span, line, cur };
        }
    }

    pub struct Lexer<'a> {
        source: Peekable<Chars<'a>>
    }

    impl Lexer<'a> {

    }
}
