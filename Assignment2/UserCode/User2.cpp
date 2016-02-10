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
    bool Block ();
    bool Class ();
    bool Function ();
    bool Var ();

    bool GroupedExpression ();
    bool Literal ();
    bool NameReference ();
    bool Value ();

    bool Expression ();
    bool Expression1 ();
    bool Expression2 ();
    bool Expression3 ();
    bool Expression4 ();
    bool Expression5 ();
    bool Expression6 ();
    bool Expression7 ();

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

    bool Label ();
    bool Goto ();
    bool Return ();

    bool MemberAccess ();
    bool Call ();
    bool Cast ();
    bool Index ();

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
    m_streamCursor = 0;
    Block ();
    // if we didn't consume the whole stream
    if (m_streamCursor != m_tokenStream.size()) {
        throw ParsingException("Stream has remaining tokens");
    }
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
    Expect(TokenType::Asterisk);
    while (Accept(TokenType::Asterisk));
    while (Accept(TokenType::Ampersand));
    Expect(TokenType::OpenParentheses);
    if (!Type()) {
        throw ParsingException("Expected type in function type");
    }
    while (Accept(TokenType::Comma)) {
        if (!Type()) {
            throw ParsingException(
                "Expected Type after comma in function type");
        }
    }
    Expect(TokenType::CloseParentheses);
    SpecifiedType();
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
    while (Statement());

    return rule.Accept(Expect(TokenType::CloseCurley));
}

//=========================================================
bool Parser::Statement () {
    PrintRule rule("Statement");
    return rule.Accept(
        FreeStatement() ||
        (DelimitedStatement() && Expect(TokenType::Semicolon))
    );
}

//=========================================================
bool Parser::FreeStatement () {
    PrintRule rule("FreeStatement");
    return rule.Accept(If() || While() || For());
}

//=========================================================
bool Parser::DelimitedStatement () {
    PrintRule rule("DelimitedStatement");
    return rule.Accept(
        Label()                     ||
        Goto()                      ||
        Return()                    ||
        Accept(TokenType::Break)    ||
        Accept(TokenType::Continue) ||
        Var()                       ||
        Expression()
    );
}

//=========================================================
bool Parser::If () {
    PrintRule rule("If");
    if (Accept(TokenType::If) == false) {
        return false;
    }
    if (!GroupedExpression() || !Scope()) {
        throw ParsingException("expected group expression for if");
    }
    Else ();
    return rule.Accept();
}

//=========================================================
bool Parser::Else () {
    PrintRule rule("Else");
    if (Accept(TokenType::Else) == false) {
        return false;
    }
    if (!If() && !Scope()) {
        throw ParsingException("Failed to parse Else");
    }
    return rule.Accept();
}

//=========================================================
bool Parser::While () {
    PrintRule rule("While");
    if (Accept(TokenType::While) == false) {
        return false;
    }
    if (!GroupedExpression()) {
        throw ParsingException("Expected grouped expression in while");
    }
    if (!Scope()) {
        throw ParsingException("Expected scope in while");
    }
    return rule.Accept();
}

//=========================================================
bool Parser::For () {
    PrintRule rule("For");
    if (Accept(TokenType::For) == false) {
        return false;
    }
    Expect(TokenType::OpenParentheses);
    if (Var() || Expression());
    Expect(TokenType::Semicolon);
    Expression();
    Expect(TokenType::Semicolon);
    Expression();
    Expect(TokenType::CloseParentheses);
    if (!Scope()) {
        throw ParsingException("Expected scope for For block");
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Label () {
    PrintRule rule("Label");
    if (Accept(TokenType::Label) == false) {
        return false;
    }
    return rule.Accept(Expect(TokenType::Identifier));
}

//=========================================================
bool Parser::Goto () {
    PrintRule rule("Goto");
    if (Accept(TokenType::Goto) == false) {
        return false;
    }
    return rule.Accept(Expect(TokenType::Identifier));
}

//=========================================================
bool Parser::Return () {
    PrintRule rule("Return");
    if (Accept(TokenType::Return) == false) {
        return false;
    }
    Expression();
    return rule.Accept();
}

//=========================================================
bool Parser::MemberAccess () {
    PrintRule rule("MemberAccess");
    if (!Accept(TokenType::Dot) && !Accept(TokenType::Arrow)) {
        return false;
    }
    return rule.Accept(Expect(TokenType::Identifier));
}

//=========================================================
bool Parser::Call () {
    PrintRule rule("Call");
    if (Accept(TokenType::OpenParentheses) == false) {
        return false;
    }
    if (Expression()) {
        while (Accept(TokenType::Comma)) {
            if (!Expression()) {
                throw ParsingException(
                    "Expected expression after comma in func call"
                );
            }
        }
    }
    return rule.Accept(Expect(TokenType::CloseParentheses));
}

//=========================================================
bool Parser::Cast () {
    PrintRule rule("Cast");
    if (Accept(TokenType::As) == false) {
        return false;
    }
    if (!Type()) {
        throw ParsingException("Expected Type in Cast");
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Index () {
    PrintRule rule("Index");
    if (Accept(TokenType::OpenBracket) == false) {
        return false;
    }
    if (!Expression()) {
        throw ParsingException("Expected expression in Indexing");
    }
    return rule.Accept(Expect(TokenType::CloseBracket));
}

//=========================================================
bool Parser::Expression () {
    PrintRule rule("Expression");
    if (!Expression1()) {
        return false;
    }

    if (
        Accept(TokenType::Assignment)         ||
        Accept(TokenType::AssignmentPlus)     ||
        Accept(TokenType::AssignmentMinus)    ||
        Accept(TokenType::AssignmentMultiply) ||
        Accept(TokenType::AssignmentDivide)   ||
        Accept(TokenType::AssignmentModulo)
    ) {
        if (!Expression()) {
            throw ParsingException("Failed parsing base expression");
        }
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Expression1 () {
    PrintRule rule("Expression1");
    if (!Expression2()) {
        return false;
    }
    while (Accept(TokenType::LogicalOr)) {
        if (!Expression2()) {
            throw ParsingException("Failed parsing expression 1");
        }
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Expression2 () {
    PrintRule rule("Expression2");
    if (!Expression3()) {
        return false;
    }
    while (Accept(TokenType::LogicalAnd)) {
        if (!Expression3()) {
            throw ParsingException("Failed Parsing expression 2");
        }
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Expression3 () {
    PrintRule rule("Expression3");
    if (!Expression4()) {
        return false;
    }
    while (
        Accept(TokenType::LessThan) ||
        Accept(TokenType::GreaterThan) ||
        Accept(TokenType::LessThanOrEqualTo) ||
        Accept(TokenType::GreaterThanOrEqualTo) ||
        Accept(TokenType::Equality) ||
        Accept(TokenType::Inequality)
    ) {
        if (!Expression4()) {
            throw ParsingException("Failed Parsing Expression 3");
        }
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Expression4 () {
    PrintRule rule("Expression4");
    if (!Expression5()) {
        return false;
    }
    while (Accept(TokenType::Plus) || Accept(TokenType::Minus)) {
        if (!Expression5()) {
            throw ParsingException("Failed Parsing Expression 4");
        }
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Expression5 () {
    PrintRule rule("Expression5");
    if (!Expression6()) {
        return false;
    }
    while (
        Accept(TokenType::Asterisk) ||
        Accept(TokenType::Divide)   ||
        Accept(TokenType::Modulo)
    ) {
        if (!Expression6()) {
            throw ParsingException("Failed Parsing expression 5");
        }
    }
    return rule.Accept();
}

//=========================================================
bool Parser::Expression6 () {
    PrintRule rule("Expression6");
    while (
        Accept(TokenType::Asterisk)   ||
        Accept(TokenType::Ampersand)  ||
        Accept(TokenType::Plus)       ||
        Accept(TokenType::Minus)      ||
        Accept(TokenType::LogicalNot) ||
        Accept(TokenType::Increment)  ||
        Accept(TokenType::Decrement)
    );
    return rule.Accept(Expression7());
}

//=========================================================
bool Parser::Expression7 () {
    PrintRule rule("Expression7");
    if (Value() == false) {
        return false;
    }
    while (
        MemberAccess() ||
        Call() ||
        Cast() ||
        Index()
    );
    return rule.Accept();
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
bool Parser::Literal () {
    PrintRule rule("Literal");
    return rule.Accept(
        Accept(TokenType::True)           ||
        Accept(TokenType::False)          ||
        Accept(TokenType::Null)           ||
        Accept(TokenType::IntegerLiteral) ||
        Accept(TokenType::FloatLiteral)   ||
        Accept(TokenType::StringLiteral)  ||
        Accept(TokenType::CharacterLiteral)
    );
}

//=========================================================
bool Parser::NameReference () {
    PrintRule rule("NameReference");
    return rule.Accept(Accept(TokenType::Identifier));
}

//=========================================================
bool Parser::Value () {
    PrintRule rule("Value");
    return rule.Accept(
        Literal()       ||
        NameReference() ||
        GroupedExpression()
    );
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
