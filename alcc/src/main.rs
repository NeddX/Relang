mod rlang;

fn main() {
    let input = "1 + (10 /     100 -              1) ";
    let mut lexer = rlang::analyzer::lexer::Lexer::new(input);
    let mut tokens: Vec<rlang::analyzer::lexer::Token> = Vec::new();
    while let Some(token) = lexer.next_token() {
        tokens.push(token);
    }
    println!("{:?}", tokens);
}
