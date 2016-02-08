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
    bool Block      ();
    bool Class      ();
    bool Function   ();
    bool Var        ();
    bool GroupedExpression ();
    bool Expression ();
    bool SpecifiedType ();
    bool Type ();
    bool NamedType ();
    bool FunctionType ();
    bool Parameter ();
    bool Scope ();
    bool Statement ();
    bool FreeStatement ();
    bool DelimitedStatement ();
    bool If ();
    bool Else ();
    bool While ();
    bool For ();

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
    if (m_streamCursor >= (int)m_tokenStream.size()) {
        return false;
    }
    Token token = m_tokenStream[m_streamCursor];
    bool match = token.mEnumTokenType == tokenType;
    if (match) {
        PrintRule::AcceptedToken(token);
        ++m_streamCursor;
    }
    return match;
}

//=========================================================
bool Parser::Expect (TokenType::Enum tokenType) {
    if (!Accept(tokenType)) {
        throw ParsingException("failed parsing");
    }
    return true;
}

//=========================================================
bool Parser::Block () {
    PrintRule rule("Block");

    while (
        Class() ||
        Function() ||
        (Var() && Expect(TokenType::Semicolon))
    );

    return rule.Accept();
}

//=========================================================
bool Parser::Class () {
    PrintRule rule("Class");
    if (Accept(TokenType::Class) == false) {
        return false;
    }

    Expect(TokenType::Identifier);
    Expect(TokenType::OpenCurley);

    while (
        (Var() && Expect(TokenType::Semicolon)) ||
        Function()
    );

    return rule.Accept(Expect(TokenType::CloseCurley));
}

//=========================================================
bool Parser::Function () {
    PrintRule rule("Function");
    if (Accept(TokenType::Function) == false) {
        return false;
    }
    Expect(TokenType::Identifier);
    Expect(TokenType::OpenParentheses);

    if (Parameter()) {
        while (Accept(TokenType::Comma)) {
            if (!Parameter()) {
                throw ParsingException("Expected parameter after comma in function sig");
            }
        }
    }

    Expect(TokenType::CloseParentheses);

    SpecifiedType();

    if (!Scope()) {
        throw ParsingException("Expected scope after function");
    }

    return rule.Accept();
}

//=========================================================
bool Parser::Var () {
    PrintRule rule("Var");

    if (Accept(TokenType::Var) == false) {
        return false;
    }

    Expect(TokenType::Identifier);
    if (!SpecifiedType()) {
        throw ParsingException("No specified type provided on variable");
    }

    if (Accept(TokenType::Assignment)) {
        if (!Expression()) {
            throw ParsingException(
                "expected expression on right side of var assignment");
        }
    }

    return rule.Accept();
}

//=========================================================
bool Parser::SpecifiedType () {
    PrintRule rule("SpecifiedType");
    if (Accept(TokenType::Colon) == false) {
        return false;
    }
    if (!Type()) {
        throw ParsingException("Expected type in SpecifiedType");
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Type () {
    PrintRule rule("Type");
    return rule.Accept(NamedType()) || rule.Accept(FunctionType());
}

//=========================================================
bool Parser::NamedType () {
    PrintRule rule("NamedType");
    if (Accept(TokenType::Identifier) == false) {
        return false;
    }

    while (Accept(TokenType::Asterisk));

    Accept(TokenType::Ampersand);

    return rule.Accept();
}

//=========================================================
bool Parser::FunctionType () {
    PrintRule rule("FunctionType");
    if (Accept(TokenType::Function) == false) {
        return false;
    }

    Expect(TokenType::Identifier);
    Expect(TokenType::OpenParentheses);

    if (Parameter()) {
        while (Accept(TokenType::Comma)) {
            if (!Parameter()) {
                throw ParsingException(
                    "Expected Parameter after comma in function type");
            }
        }
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Parameter () {
    PrintRule rule ("Parameter");
    return rule.Accept(
        Accept(TokenType::Identifier) &&
        SpecifiedType()
    );
}

//=========================================================
bool Parser::Scope () {
    PrintRule rule("Scope");
    if (Accept(TokenType::OpenCurley) == false) {
        return false;
    }
    Statement();

    return rule.Accept(Expect(TokenType::CloseCurley));
}

//=========================================================
bool Parser::Statement () {
    PrintRule rule("Statement");

    return false;
}

//=========================================================
bool Parser::FreeStatement () {
    return false;
}

//=========================================================
bool Parser::DelimitedStatement () {
    return false;
}

//=========================================================
bool Parser::If () {
    PrintRule rule("If");
    if (Accept(TokenType::If) == false) {
        return false;
    }

    if (!GroupedExpression()) {
        throw ParsingException("expected group expression for if");
    }
    if (!Scope()) {
        throw ParsingException("expected scope for if block");
    }
    Else ();

    return rule.Accept();
}

//=========================================================
bool Parser::Else () {
    PrintRule rule("Else");
    return false;
}

//=========================================================
bool Parser::While () {
    PrintRule rule("While");
    return false;
}

//=========================================================
bool Parser::For () {
    PrintRule rule("For");
    return false;
}

//=========================================================
bool Parser::Expression () {
    PrintRule rule("Expression");
    return false;
}

//=========================================================
bool Parser::GroupedExpression () {
    PrintRule rule("GroupedExpression");
    if (Accept(TokenType::OpenParentheses) == false) {
        return false;
    }

    Expression();

    return rule.Accept(Expect(TokenType::CloseParentheses));
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
                return (t.mEnumTokenType == TokenType::Whitespace)
                    || (t.mEnumTokenType == TokenType::SingleLineComment)
                    || (t.mEnumTokenType == TokenType::MultiLineComment);
            }
        ),
        tokens.end()
    );
}

//=========================================================
void Recognize (std::vector<Token>& tokens) {
    Parser parser(tokens);
    parser.Parse();
}
