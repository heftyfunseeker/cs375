/******************************************************************\
 * Author: Nicco Simone
 * Copyright 2015, DigiPen Institute of Technology
\******************************************************************/

#include <algorithm>
#include "../Drivers/Driver2.hpp"

class Parser {
public:
    Parser  (const std::vector<Token> & tokenStream);
    ~Parser ();
    void Parse ();
private:
    bool Block    ();
    bool Class    ();
    bool Function ();
    bool Var      ();

    bool Accept (TokenType::Enum tokenType);
    bool Expect (TokenType::Enum tokenType);

    const std::vector<Token> & m_tokenStream;
    int m_streamCursor;
};

//=========================================================
Parser::Parser (const std::vector<Token> & tokenStream) :
    m_tokenStream(tokenStream),
    m_streamCursor(0)
{}


//=========================================================
Parser::~Parser ()
{}

//=========================================================
void Parser::Parse () {
    Block ();
}

//=========================================================
bool Parser::Accept (TokenType::Enum tokenType) {
    return false;
}

//=========================================================
bool Parser::Expect (TokenType::Enum tokenType) {
    return false;
}

//=========================================================
bool Parser::Block () {
    PrintRule rule("Block");

    while (Class() || Function() || Var() && Expect(TokenType::Semicolon));

    return rule.Accept();
}

//=========================================================
bool Parser::Class () {
    return false;
}

//=========================================================
bool Parser::Function () {
    return false;
}

//=========================================================
bool Parser::Var () {
    return false;
}

//=========================================================
void RemoveWhitespaceAndComments(std::vector<Token>& tokens) {
    if (tokens.size() == 0) {
        return;
    }

    tokens.erase(
        std::remove_if(
            tokens.begin(),
            tokens.end(),
            [](Token t) {
                return t.mEnumTokenType == TokenType::Whitespace
                    || t.mEnumTokenType == TokenType::SingleLineComment
                    || t.mEnumTokenType == TokenType::MultiLineComment;
            }
        )
    );
}

//=========================================================
void Recognize (std::vector<Token>& tokens) {
    Parser parser(tokens);
    parser.Parse();
}
